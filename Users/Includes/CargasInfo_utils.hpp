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

struct Putdata
{
	std::string username;
	std::string password;
	std::string email;
	std::string first_name;
	std::string last_name;
}

struct jwt_data
{
	std::string company_id;
	std::string user_id;
};



class ConnectionPool {
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
};

void handle_request(http::request<http::string_body> const& req, http::response<http::string_body>& res);
void do_session(tcp::socket socket);
std::string generate_jwt(const std::string& user_id, const std::string& company_id);
jwt_data jwt_checker(const std::string& token);
Postdata parse_post(const std::string req_body);
Putdata parse_put(const std::string req_body);

#endif
