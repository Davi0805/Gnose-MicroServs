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

#define GREEN_TEXT "\033[32m"
#define RESET_COLOR "\033[0m"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

struct Postdata
{
	std::string costumer;
	std::string company_id;
	std::string shipping_status;
	std::string type_of_load;
	std::string origin;
	std::string destination;
	std::string weight;
	std::string cost;
	std::string ce_mercante;
	std::string ce_mercante_filename;
	std::string ce_mercante_filepath;
	std::string packinglist_filename;
	std::string packinglist_filepath;
	std::string ncm;
	std::string cntr_id;
	std::string referencia_id;
	std::string afrmmpago;
	std::string afrmm_filename;
	std::string afrmm_filepath;
	std::string blnum;
	std::string bl_filename;
	std::string bl_filepath;
	std::string num_nf;
};

/* class ConnectionPool {
public:
    ConnectionPool(const std::string& connection_string, size_t pool_size) {
        for (size_t i = 0; i < pool_size; ++i) {
            connections.push(std::make_unique<pqxx::connection>(connection_string));
        }
    }

    std::unique_ptr<pqxx::connection> acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !connections.empty(); });
        auto conn = std::move(connections.front());
        connections.pop();
        return conn;
    }

    void release(std::unique_ptr<pqxx::connection> conn) {
        std::unique_lock<std::mutex> lock(mutex_);
        connections.push(std::move(conn));
        condition_.notify_one();
    }

private:
    std::queue<std::unique_ptr<pqxx::connection>> connections;
    std::mutex mutex_;
    std::condition_variable condition_;
}; */

class ConnectionPool {
public:
    ConnectionPool(const std::string& conn_info, std::size_t pool_size)
        : conn_info_(conn_info), pool_size_(pool_size), semaphore_(pool_size) {
        for (std::size_t i = 0; i < pool_size_; ++i) {
            connections_.push_back(std::make_shared<pqxx::connection>(conn_info_));
        }
    }

    std::shared_ptr<pqxx::connection> acquire() {
        semaphore_.acquire();  // Espera até que haja uma conexão disponível
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
        semaphore_.release();  // Libera o semáforo para indicar que há uma conexão disponível
    }

private:
    std::string conn_info_;
    std::size_t pool_size_;
    std::vector<std::shared_ptr<pqxx::connection>> connections_;
    std::mutex mtx_;
    std::counting_semaphore<> semaphore_;
};

struct jwt_data
{
	std::string company_id;
	std::string user_id;
};

void handle_request(http::request<http::string_body> const& req, http::response<http::string_body>& res);
void do_session(tcp::socket socket);
jwt_data jwt_checker(const std::string& token);

#endif // CARGASINFO_UTILS_HPP
