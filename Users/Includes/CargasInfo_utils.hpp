#ifndef CARGASINFO_UTILS_HPP
#define CARGASINFO_UTILS_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <queue>
#include <mutex>
#include <pqxx/pqxx>
#include <condition_variable>
#include <string>
#include <semaphore>
#include <thread>
#include <hiredis/hiredis.h>

#define GREEN_TEXT "\033[32m"
#define RESET_COLOR "\033[0m"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

struct Postdata
{
	std::string username;
	std::string password;
};

struct Registro_func_data
{
	std::string username;
	std::string password;
	std::string email;
	std::string first_name;
	std::string last_name;
	std::string permission;
};

struct Putdata
{
	std::string username;
	std::string password;
	std::string email;
	std::string first_name;
	std::string last_name;
};

struct jwt_data
{
	std::string company_id;
	std::string user_id;
	std::string permission_level;
};


/* class ConnectionPool {
public:
    ConnectionPool(const std::string& conn_info, std::size_t pool_size)
        : conn_info_(conn_info), pool_size_(pool_size), semaphore_(pool_size) {
        for (std::size_t i = 0; i < pool_size_; ++i) {
            connections_.push_back(std::make_shared<pqxx::connection>(conn_info_));
        }
    }

    std::shared_ptr<pqxx::connection> acquire() {
        semaphore_.acquire();
        std::lock_guard<std::mutex> lock(mtx_);

        auto conn = connections_.back();
        connections_.pop_back();
        return conn;
    }

    void release(std::shared_ptr<pqxx::connection> conn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            connections_.push_back(conn);
        }
        semaphore_.release();
    }

private:
    std::string conn_info_;
    std::size_t pool_size_;
    std::vector<std::shared_ptr<pqxx::connection>> connections_;
    std::mutex mtx_;
    std::counting_semaphore<> semaphore_;
};

class RedisConnectionPool {
public:
    RedisConnectionPool(const std::string& host, int port, std::size_t pool_size)
        : host_(host), port_(port), pool_size_(pool_size), semaphore_(pool_size) {
        for (std::size_t i = 0; i < pool_size_; ++i) {
            redisContext* context = redisConnect(host_.c_str(), port_);
            if (context == nullptr || context->err) {
                throw std::runtime_error("Failed to connect to Redis");
            }
            connections_.push_back(std::shared_ptr<redisContext>(context, redisFree));
        }
    }

    std::shared_ptr<redisContext> acquire() {
        semaphore_.acquire();
        std::lock_guard<std::mutex> lock(mtx_);

        auto conn = connections_.back();
        connections_.pop_back();
        return conn;
    }

    void release(std::shared_ptr<redisContext> conn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            connections_.push_back(conn);
        }
        semaphore_.release();
    }

private:
    std::string host_;
    int port_;
    std::size_t pool_size_;
    std::vector<std::shared_ptr<redisContext>> connections_;
    std::mutex mtx_;
    std::counting_semaphore<> semaphore_;
}; */

class ConnectionPool {
public:
    ConnectionPool(const std::string& conn_info, std::size_t pool_size, int max_retries = 3, int retry_delay_ms = 1000)
        : conn_info_(conn_info), pool_size_(pool_size), semaphore_(pool_size) {
        for (std::size_t i = 0; i < pool_size_; ++i) {
            std::shared_ptr<pqxx::connection> conn = nullptr;
            for (int attempt = 0; attempt < max_retries; ++attempt) {
                try {
                    conn = std::make_shared<pqxx::connection>(conn_info_);
                    if (conn->is_open()) {
                        break;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Tentativa de conexao " << attempt + 1 << " falhou: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
                }
            }
            if (!conn || !conn->is_open()) {
                throw std::runtime_error("Falha ao conectar ao PostgreSQL depois de " + std::to_string(max_retries) + " tentativas");
            }
            connections_.push_back(conn);
        }
    }

    std::shared_ptr<pqxx::connection> acquire() {
        semaphore_.acquire();
        std::lock_guard<std::mutex> lock(mtx_);

        auto conn = connections_.back();
        connections_.pop_back();
        return conn;
    }

    void release(std::shared_ptr<pqxx::connection> conn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            connections_.push_back(conn);
        }
        semaphore_.release();
    }

private:
    std::string conn_info_;
    std::size_t pool_size_;
    std::vector<std::shared_ptr<pqxx::connection>> connections_;
    std::mutex mtx_;
    std::counting_semaphore<> semaphore_;
};

class RedisConnectionPool {
public:
    RedisConnectionPool(const std::string& host, int port, std::size_t pool_size, int max_retries = 3, int retry_delay_ms = 1000)
        : host_(host), port_(port), pool_size_(pool_size), semaphore_(pool_size) {
        for (std::size_t i = 0; i < pool_size_; ++i) {
            redisContext* context = nullptr;
            for (int attempt = 0; attempt < max_retries; ++attempt) {
                context = redisConnect(host_.c_str(), port_);
                if (context != nullptr && !context->err) {
                    break;
                }
                std::cerr << "Tentativa de conexao " << attempt + 1 << " falhou: " << (context ? context->errstr : "erro desconhecido") << std::endl;
                if (context) {
                    redisFree(context);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
            }
            if (context == nullptr || context->err) {
                throw std::runtime_error("Falha ao conectar ao redis depois de " + std::to_string(max_retries) + " tentativas");
            }
            connections_.push_back(std::shared_ptr<redisContext>(context, redisFree));
        }
    }

    std::shared_ptr<redisContext> acquire() {
        semaphore_.acquire();
        std::lock_guard<std::mutex> lock(mtx_);

        auto conn = connections_.back();
        connections_.pop_back();
        return conn;
    }

    void release(std::shared_ptr<redisContext> conn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            connections_.push_back(conn);
        }
        semaphore_.release();
    }

private:
    std::string host_;
    int port_;
    std::size_t pool_size_;
    std::vector<std::shared_ptr<redisContext>> connections_;
    std::mutex mtx_;
    std::counting_semaphore<> semaphore_;
};

class Error_handler
{
	public:
		Error_handler(std::string cerr, http::response<http::string_body> &res)
		{
			erro = cerr;
			error_res = res;
		}
		http::response<http::string_body> gerar_resposta()
		{
			if (erro == "Token nao encontrado!")
			{	error_res.result(http::status::unauthorized);
				error_res.set(http::field::content_type, "application/json");
				error_res.body() = "Unauthorized";
			}
			else if (erro == "Falha ao conectar com o redis!")
			{
				error_res.result(http::status::internal_server_error);
				error_res.set(http::field::content_type, "application/json");
				error_res.body() = erro;
			}
			
			return(error_res);
		}
	private:
		std::string erro;
		http::response<http::string_body> error_res;
};

void handle_request(http::request<http::string_body> const& req, http::response<http::string_body>& res);
void do_session(tcp::socket socket);
std::string generate_jwt(const std::string& user_id, const std::string& company_id, const std::string& permission_level);
jwt_data jwt_checker(const std::string& token);
Postdata parse_post(const std::string req_body);
Putdata parse_put(const std::string req_body);

#endif
