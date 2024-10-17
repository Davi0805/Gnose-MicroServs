#include "Includes/CargasInfo_utils.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <pqxx/pqxx>

#include <nlohmann/json.hpp>

/* CREATE TABLE shipments (
    costumer VARCHAR(255),
    company_id VARCHAR(255),
    shipping_status VARCHAR(255),
    type_of_load VARCHAR(255),
    origin VARCHAR(255),
    destination VARCHAR(255),
    weight VARCHAR(255),
    cost VARCHAR(255),
    ce_mercante VARCHAR(255),
    ce_mercante_filename VARCHAR(255),
    ce_mercante_filepath VARCHAR(255),
    packinglist_filename VARCHAR(255),
    packinglist_filepath VARCHAR(255),
    ncm VARCHAR(255),
    cntr_id VARCHAR(255),
    referencia_id VARCHAR(255),
    afrmmpago VARCHAR(255),
    afrmm_filename VARCHAR(255),
    afrmm_filepath VARCHAR(255),
    blnum VARCHAR(255),
    bl_filename VARCHAR(255),
    bl_filepath VARCHAR(255),
    num_nf VARCHAR(255)
); */

/* INSERT INTO shipments (
    costumer, company_id, shipping_status, type_of_load, origin, destination, weight, cost,
    ce_mercante, ce_mercante_filename, ce_mercante_filepath, packinglist_filename, packinglist_filepath,
    ncm, cntr_id, referencia_id, afrmmpago, afrmm_filename, afrmm_filepath, blnum, bl_filename,
    bl_filepath, num_nf
) VALUES
(
    'John Doe', '12345', 'In Transit', 'Full Load', 'New York', 'Los Angeles', '1000kg', '5000',
    '123456789', 'ce_mercante.pdf', '/path/to/ce_mercante.pdf', 'packinglist.pdf', '/path/to/packinglist.pdf',
    '1234.56.78', 'CNTR12345', 'REF12345', 'Paid', 'afrmm.pdf', '/path/to/afrmm.pdf', 'BL12345', 'bl.pdf',
    '/path/to/bl.pdf', 'NF12345'
),
(
    'Jane Smith', '67890', 'Delivered', 'Partial Load', 'Chicago', 'Houston', '500kg', '2500',
    '987654321', 'ce_mercante2.pdf', '/path/to/ce_mercante2.pdf', 'packinglist2.pdf', '/path/to/packinglist2.pdf',
    '8765.43.21', 'CNTR67890', 'REF67890', 'Unpaid', 'afrmm2.pdf', '/path/to/afrmm2.pdf', 'BL67890', 'bl2.pdf',
    '/path/to/bl2.pdf', 'NF67890'
),
(
    'Alice Johnson', '54321', 'Pending', 'Full Load', 'Miami', 'Seattle', '1500kg', '7500',
    '112233445', 'ce_mercante3.pdf', '/path/to/ce_mercante3.pdf', 'packinglist3.pdf', '/path/to/packinglist3.pdf',
    '5678.12.34', 'CNTR54321', 'REF54321', 'Paid', 'afrmm3.pdf', '/path/to/afrmm3.pdf', 'BL54321', 'bl3.pdf',
    '/path/to/bl3.pdf', 'NF54321'
),
(
    'Bob Brown', '98765', 'In Transit', 'Partial Load', 'San Francisco', 'Denver', '800kg', '4000',
    '998877665', 'ce_mercante4.pdf', '/path/to/ce_mercante4.pdf', 'packinglist4.pdf', '/path/to/packinglist4.pdf',
    '4321.65.87', 'CNTR98765', 'REF98765', 'Unpaid', 'afrmm4.pdf', '/path/to/afrmm4.pdf', 'BL98765', 'bl4.pdf',
    '/path/to/bl4.pdf', 'NF98765'
),
(
    'Charlie Davis', '11223', 'Delivered', 'Full Load', 'Boston', 'Phoenix', '2000kg', '10000',
    '556677889', 'ce_mercante5.pdf', '/path/to/ce_mercante5.pdf', 'packinglist5.pdf', '/path/to/packinglist5.pdf',
    '8765.43.21', 'CNTR11223', 'REF11223', 'Paid', 'afrmm5.pdf', '/path/to/afrmm5.pdf', 'BL11223', 'bl5.pdf',
    '/path/to/bl5.pdf', 'NF11223'
),
(
    'Diana Evans', '33445', 'Pending', 'Partial Load', 'Atlanta', 'Dallas', '1200kg', '6000',
    '223344556', 'ce_mercante6.pdf', '/path/to/ce_mercante6.pdf', 'packinglist6.pdf', '/path/to/packinglist6.pdf',
    '3456.78.90', 'CNTR33445', 'REF33445', 'Unpaid', 'afrmm6.pdf', '/path/to/afrmm6.pdf', 'BL33445', 'bl6.pdf',
    '/path/to/bl6.pdf', 'NF33445'
),
(
    'Eve Foster', '55667', 'In Transit', 'Full Load', 'Philadelphia', 'San Diego', '1800kg', '9000',
    '667788990', 'ce_mercante7.pdf', '/path/to/ce_mercante7.pdf', 'packinglist7.pdf', '/path/to/packinglist7.pdf',
    '6789.01.23', 'CNTR55667', 'REF55667', 'Paid', 'afrmm7.pdf', '/path/to/afrmm7.pdf', 'BL55667', 'bl7.pdf',
    '/path/to/bl7.pdf', 'NF55667'
),
(
    'Frank Green', '77889', 'Delivered', 'Partial Load', 'Detroit', 'Las Vegas', '700kg', '3500',
    '334455667', 'ce_mercante8.pdf', '/path/to/ce_mercante8.pdf', 'packinglist8.pdf', '/path/to/packinglist8.pdf',
    '7890.12.34', 'CNTR77889', 'REF77889', 'Unpaid', 'afrmm8.pdf', '/path/to/afrmm8.pdf', 'BL77889', 'bl8.pdf',
    '/path/to/bl8.pdf', 'NF77889'
),
(
    'Grace Harris', '99001', 'Pending', 'Full Load', 'Orlando', 'Portland', '1600kg', '8000',
    '445566778', 'ce_mercante9.pdf', '/path/to/ce_mercante9.pdf', 'packinglist9.pdf', '/path/to/packinglist9.pdf',
    '8901.23.45', 'CNTR99001', 'REF99001', 'Paid', 'afrmm9.pdf', '/path/to/afrmm9.pdf', 'BL99001', 'bl9.pdf',
    '/path/to/bl9.pdf', 'NF99001'
),
(
    'Hank Irving', '22334', 'In Transit', 'Partial Load', 'Charlotte', 'San Antonio', '900kg', '4500',
    '556677889', 'ce_mercante10.pdf', '/path/to/ce_mercante10.pdf', 'packinglist10.pdf', '/path/to/packinglist10.pdf',
    '9012.34.56', 'CNTR22334', 'REF22334', 'Unpaid', 'afrmm10.pdf', '/path/to/afrmm10.pdf', 'BL22334', 'bl10.pdf',
    '/path/to/bl10.pdf', 'NF22334'
); */

using json = nlohmann::json;
extern ConnectionPool connection_pool;
extern RedisConnectionPool redis_pool;

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

Postdata parse_post(const std::string req_body)
{
	Postdata result;
	json data_body = json::parse(req_body);

	// OBJECT PARSER DA LIB NLOHMANN JSON
	auto get_string = [&](const std::string &key, const std::string &default_value)
	{
		return data_body.contains(key) && !data_body[key].is_null() ? data_body[key].get<std::string>() : default_value;
	};

	result.costumer = get_string("costumer", "unknown");
	result.company_id = get_string("company_id", "");
	result.shipping_status = get_string("shipping_status", "pending");
	result.type_of_load = get_string("type_of_load", "standard");
	result.origin = get_string("origin", "unknown");
	result.destination = get_string("destination", "unknown");
	result.weight = get_string("weight", "0");
	result.cost = get_string("cost", "0");
	result.ce_mercante = get_string("ce_mercante", "");
	result.ce_mercante_filename = get_string("ce_mercante_filename", "");
	result.ce_mercante_filepath = get_string("ce_mercante_filepath", "");
	result.packinglist_filename = get_string("packinglist_filename", "");
	result.packinglist_filepath = get_string("packinglist_filepath", "");
	result.ncm = get_string("ncm", "");
	result.cntr_id = get_string("cntr_id", "");
	result.referencia_id = get_string("referencia_id", "");
	result.afrmmpago = get_string("afrmmpago", "false");
	result.afrmm_filename = get_string("afrmm_filename", "");
	result.afrmm_filepath = get_string("afrmm_filepath", "");
	result.blnum = get_string("blnum", "");
	result.bl_filename = get_string("bl_filename", "");
	result.bl_filepath = get_string("bl_filepath", "");
	result.num_nf = get_string("num_nf", "");
	return result;
}

void handle_request(http::request<http::string_body> const &req, http::response<http::string_body> &res)
{
	if (req.method() == http::verb::post)
	{
		try
		{
			auto auth_header = req.base()["Authorization"];
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado!");

			std::string token = auth_header.to_string().substr(7);
			jwt_data token_data = jwt_checker(token);

			// CHECAR SESSION NO REDIS
			auto cache_context = redis_pool.acquire();
			if (!cache_context || cache_context->err)
			{
				redis_pool.release(cache_context);
				throw std::runtime_error("Falha ao conectar com o redis!");
			}

			std::string cache_key = "user:" + token_data.user_id;
			redisReply *redis_result = (redisReply *)redisCommand(cache_context.get(), "JSON.GET %s $.session_jwt", cache_key.c_str());
			if (redis_result)
			{
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": consultando session!" << std::endl;
				if (redis_result->str != token)
					throw std::runtime_error("Token nao compativel!");
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": Session encontrada!" << std::endl;
			}
			else
			{
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": session nao encontrada!" << std::endl;
				throw std::runtime_error("Session nao encontrada!");
			}
			redis_pool.release(cache_context);

			Postdata data = parse_post(req.body());
			std::cout << GREEN_TEXT << "[POST]" << RESET_COLOR << ": " << req.body() << std::endl;

			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn); // INICIA COMUNICACAO

			conn->prepare("insert_shipments", "INSERT INTO shipments (costumer, company_id, shipping_status, type_of_load, origin, destination, weight, cost) VALUES ($1, $2, $3, $4, $5, $6, $7, $8)");

			txn.exec_prepared("insert_shipments", data.costumer, data.company_id, data.shipping_status, data.type_of_load, data.origin, data.destination, data.weight, data.cost);

			txn.commit();
			connection_pool.release(conn);

			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			res.body() = "Post funcionando";
		}
		catch (const std::exception &e)
		{
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			res.body() = "error: " + std::string(e.what());
		}
	}
	else if (req.method() == http::verb::get && req.target().to_string().starts_with("/id="))
	{
		try
		{
			auto auth_header = req.base()["Authorization"];
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado!");

			std::string token = auth_header.to_string().substr(7);
			jwt_data token_data = jwt_checker(token);

			auto conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			std::string path_arg = req.target().to_string().substr(4);

			std::cout << GREEN_TEXT << "[ARGUMENTO RECEBIDO]" << RESET_COLOR << ": id = " << path_arg << std::endl;

			conn->prepare("get_specific_data", "SELECT * FROM shipments WHERE id = $1");

			pqxx::result result = txn.exec_prepared("get_specific_data", path_arg);

			if (result.size() == 1)
			{
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
			else
			{
				res.result(http::status::not_found);
				res.set(http::field::content_type, "application/json");
				res.body() = "Nao encontrado";
			}
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
			/* std::cout << GREEN_TEXT << "[AUTH HEADER]" << RESET_COLOR << ": " << auth_header << std::endl; */
			if (auth_header == "")
				throw std::runtime_error("Token nao encontrado");
			std::string token = auth_header.to_string().substr(7);

			jwt_data token_data = jwt_checker(token);

			// CHECAR SESSION NO REDIS
			auto cache_context = redis_pool.acquire();
			if (!cache_context || cache_context->err)
			{
				redis_pool.release(cache_context);
				throw std::runtime_error("Falha ao conectar com o redis!");
			}

			std::string cache_key = "user:" + token_data.user_id;
			redisReply *redis_result = (redisReply *)redisCommand(cache_context.get(), "JSON.GET %s $[0].session_jwt", cache_key.c_str());
			if (redis_result)
			{
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": consultando session!" << std::endl;
				std::string redisreply_converted(redis_result->str, redis_result->len);
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": token em caching = " << redisreply_converted << " | token no jwt = " << token << std::endl;
				if (redisreply_converted.substr(2, 189) != token)
					throw std::runtime_error("Token nao compativel!");
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": Session encontrada!" << std::endl;
			}
			else
			{
				std::cout << GREEN_TEXT << "[AUTH-REDIS]" << RESET_COLOR << ": session nao encontrada!" << std::endl;
				throw std::runtime_error("Session nao encontrada!");
			}
			redis_pool.release(cache_context);

			std::shared_ptr conn = connection_pool.acquire();
			pqxx::work txn(*conn);

			std::cout << GREEN_TEXT << "[PATH DO REQUEST]" << RESET_COLOR << ": " << req.target() << std::endl;

			pqxx::result result = txn.exec("SELECT * FROM shipments");

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
