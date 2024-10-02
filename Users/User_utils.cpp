#include "Includes/CargasInfo_utils.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <pqxx/pqxx>

#include <nlohmann/json.hpp>

/* CREATE TABLE users (
  id SERIAL PRIMARY KEY,
  username VARCHAR(50) NOT NULL,
  email VARCHAR(100) UNIQUE NOT NULL,
  password VARCHAR(255) NOT NULL,
  first_name VARCHAR(50),
  last_name VARCHAR(50),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
company_id VARCHAR(255) NOT NULL);
 */

using json = nlohmann::json;
extern ConnectionPool connection_pool;
extern RedisConnectionPool redis_pool;

Postdata parse_post(const std::string req_body)
{
	Postdata result;
	json data_body = json::parse(req_body);

	auto get_string = [&](const std::string &key, const std::string &default_value)
	{
		return data_body.contains(key) && !data_body[key].is_null() ? data_body[key].get<std::string>() : default_value;
	};
	result.username = get_string("username", "");
	result.password = get_string("password", "");
	return result;
}

Putdata parse_put(const std::string req_body)
{
	Putdata result;
	json data_body = json::parse(req_body);

	auto get_string = [&](const std::string &key, const std::string &default_value)
	{
		return data_body.contains(key) && !data_body[key].is_null() ? data_body[key].get<std::string>() : default_value;
	};

	result.username = get_string("username", "");
	result.password = get_string("password", "");
	result.email = get_string("email", "");
	result.first_name = get_string("first_name", "");
	result.last_name = get_string("last_name", "");

	return result;
}

void handle_request(http::request<http::string_body> const &req, http::response<http::string_body> &res)
{
	if (req.method() == http::verb::post)
	{
		try
		{

			Postdata data = parse_post(req.body());
			std::cout << GREEN_TEXT << "[POST]" << RESET_COLOR << ": " << req.body() << std::endl;

			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			conn->prepare("verify_user", "SELECT * FROM users WHERE username = $1 AND password = $2");

			pqxx::result result = txn.exec_prepared("verify_user", data.username, data.password);

			if (result.size() == 1)
			{
				pqxx::row row = result[0];
				std::string user_id = row["id"].as<std::string>();
				std::string company_id = row["company_id"].as<std::string>();
				std::string permission_level = row["permission"].as<std::string>();

				std::cout << GREEN_TEXT << "[PERMISSION LEVEL]" << RESET_COLOR << ": " << permission_level << std::endl;

				auto cache_context = redis_pool.acquire();
			    if (!cache_context || cache_context->err)
				{
				std::cerr << "Redis context is invalid or not connected." << std::endl;
				return;
    			}

				json json_result = json::array();
				for (const auto &row : result)
				{
					json json_row;
					for (const auto &field : row)
					{
						json_row[field.name()] = field.c_str();
					}
					json_result.push_back(json_row);
				}

				std::string cache_values = json_result.dump();
				std::string cache_key = "user:" + user_id;
				/* std::string cache_values =	row["username"].as<std::string>() + "," +
											row["email"].as<std::string>() + "," +
											row["first_name"].as<std::string>() + "," +
											row["last_name"].as<std::string>() + "," +
											company_id + "," +
											permission_level; */

				redisReply *redis_result = (redisReply *)redisCommand(cache_context.get(), "JSON.SET %s . %s", cache_key.c_str(), cache_values.c_str());
				std::cout << GREEN_TEXT << "[REDIS]" << RESET_COLOR << ": " << redis_result->str << std::endl;
				freeReplyObject(redis_result);
				redisReply *expire_reply = (redisReply *)redisCommand(cache_context.get(), "EXPIRE %s 3600", cache_key.c_str());
				std::cout << GREEN_TEXT << "[REDIS]" << RESET_COLOR << ": " << expire_reply->str << std::endl;
				freeReplyObject(expire_reply);
				redis_pool.release(cache_context);

				std::string jwt = generate_jwt(user_id, company_id, permission_level);
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				res.body() = R"({"message": "Login successful", "jwt": ")" + jwt + R"("})";
			}
			else
			{
				res.result(http::status::unauthorized);
				res.set(http::field::content_type, "application/json");
				res.body() = R"({"message": "Invalid username or password"})";
			}

			txn.commit();
			connection_pool.release(conn);
		}
		catch (const std::exception &e)
		{
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			res.body() = "Database error: " + std::string(e.what());
		}
	}
	// ROTAS PROTEGIDAS A PARTIR DAQUI
	else if (req.method() == http::verb::get && req.target().to_string().starts_with("/id="))
	{
		try
		{

			std::cout << GREEN_TEXT << "[ARGUMENTO RECEBIDO]" << RESET_COLOR << ": " << req.target() << std::endl;

			auto auth_header = req.base()["Authorization"];
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado!");
			std::string token = auth_header.to_string().substr(7); // SUBSTR depois de bearer

			/* std::cout << GREEN_TEXT << "[JWT TOKEN]" << RESET_COLOR << ": " << token << std::endl; */

			jwt_data token_data = jwt_checker(token);

			std::string path_argument = req.target().to_string().substr(4);

			if (token_data.permission_level == "0" || token_data.permission_level == "1")
				throw std::runtime_error("Sem permissao!");

			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			conn->prepare("get_user_data", "SELECT * FROM users WHERE id = $1");

			pqxx::result result = txn.exec_prepared("get_user_data", path_argument);

			if (result.size() == 0)
				throw std::runtime_error("Usuario nao encontrado!");

			pqxx::row row = result[0];
			std::string company = row["company_id"].as<std::string>();
			std::cout << GREEN_TEXT << "[PERMISSION LEVEL]: " << token_data.permission_level << std::endl;
			std::cout << GREEN_TEXT << "[DEBUG - COMPANY_ID]" << RESET_COLOR << ": extraido da resposta " << company << " e extraido do jwt_data " << token_data.company_id << std::endl;
			if (token_data.permission_level != "3" && company != token_data.company_id)
				throw std::runtime_error(
					"Usuario nao tem permissao para acessar dados de outra empresa!");

			json json_result = json::array();
			for (const auto &row : result)
			{
				json json_row;
				for (const auto &field : row)
				{
					json_row[field.name()] = field.c_str();
				}
				json_result.push_back(json_row);

				// Send the response
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				res.body() = json_result.dump();
			}
			txn.commit();
			connection_pool.release(conn);
			std::cout << GREEN_TEXT << "[GET]" << RESET_COLOR << ": " << res.body() << std::endl;
		}
		catch (const std::exception &e)
		{
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			res.body() = "Database error: " + std::string(e.what());
		}
	}
	else if (req.method() == http::verb::get)
	{
		try
		{

			auto auth_header = req.base()["Authorization"];
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado!");
			std::string token = auth_header.to_string().substr(7); // SUBSTR depois de bearer

			/* std::cout << GREEN_TEXT << "[JWT TOKEN]" << RESET_COLOR << ": " << token << std::endl; */

			jwt_data token_data = jwt_checker(token);

			bool dado_extraido = false;

			auto cache_context = redis_pool.acquire();
			if (!cache_context || cache_context->err)
			{
				redis_pool.release(cache_context);
				throw std::runtime_error("Falha ao conectar com o redis!");
			}

			std::string cache_key = "user:" + token_data.user_id;
			redisReply *redis_result = (redisReply *)redisCommand(cache_context.get(), "JSON.GET %s", cache_key.c_str());
			if (redis_result)
			{
				if (redis_result->type == REDIS_REPLY_STRING)
				{
				std::cout << GREEN_TEXT << "[REDIS]" << RESET_COLOR << ": dado encontrado!" << std::endl;
				dado_extraido = true;
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				res.body() = redis_result->str;
				}
				else
				{
					std::cout << GREEN_TEXT << "[REDIS]" << RESET_COLOR << ": dado nao encontrado!" << std::endl;
					res.result(http::status::internal_server_error);
                    res.body() = "Internal Server Error";
				}
			}
			freeReplyObject(redis_result);
			redis_pool.release(cache_context);

			if (!dado_extraido)
			{
			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			conn->prepare("get_user_data", "SELECT * FROM users WHERE id = $1");

			pqxx::result result = txn.exec_prepared("get_user_data", token_data.user_id);

			//DEFINITIVAMENTE REDUNDANTE POREM PODE PREVINIR UM CRASH
			//EM CASO DE OVERLOAD NO BANCO DE DADOS OU AUSENCIA DE RESPOSTA
			if (result.size() == 0)
				throw std::runtime_error("Falha ao buscar no banco de dados");

			json json_result = json::array();
			for (const auto &row : result)
			{
				json json_row;
				for (const auto &field : row)
				{
					json_row[field.name()] = field.c_str();
				}
				json_result.push_back(json_row);

				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				res.body() = json_result.dump();
			}
			txn.commit();
			connection_pool.release(conn);
			std::cout << GREEN_TEXT << "[GET]" << RESET_COLOR << ": " << res.body() << std::endl;
			}
		}
		catch (const std::exception &e)
		{
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			res.body() = "Database error: " + std::string(e.what());
		}
	}
	else if (req.method() == http::verb::put)
	{
		try
		{
			Putdata data = parse_put(req.body());
			auto auth_header = req.base()["Authorization"];
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado!");
			std::string token = auth_header.to_string().substr(7);

			jwt_data token_data = jwt_checker(token);

			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			conn->prepare("put_user_data", "UPDATE users SET username = $1, email = $2, password = $3, first_name = $4, last_name = $5 WHERE id = $6");

			pqxx::result result = txn.exec_prepared("put_user_data", data.username, data.email, data.password, data.first_name, data.last_name, token_data.user_id);

			txn.commit();
			connection_pool.release(conn);

			std::cout << GREEN_TEXT << "[PUT]" << RESET_COLOR << ": Usuario " << token_data.user_id << " atualizou seu perfil!" << std::endl;

			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			res.body() = "Dados atualizados!";
		}
		catch (const std::exception &e)
		{
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			res.body() = "Database error: " + std::string(e.what());
		}
	}
	else
	{
		res.result(http::status::bad_request);
		res.set(http::field::content_type, "application/json");
		res.body() = "Invalid request method";
	}

	res.prepare_payload();
}

void do_session(tcp::socket socket)
{
	try
	{
		beast::flat_buffer buffer;
		http::request<http::string_body> req;
		http::response<http::string_body> res;

		http::read(socket, buffer, req);
		handle_request(req, res);
		http::write(socket, res);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}
