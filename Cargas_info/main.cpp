#include "Includes/CargasInfo_utils.hpp"
#include <iostream>
#include <thread>
#include <pqxx/pqxx>
#include <cstdlib>

/* g++ -std=c++17 -o server main.cpp CargasInfo_utils.cpp -lboost_system -lpthread -lpqxx -lpq */

const char *db_host = std::getenv("DB_HOST");
const char *db_port = std::getenv("DB_PORT");
const char *db_name = std::getenv("DB_NAME");
const char *db_user = std::getenv("DB_USER");
const char *db_password = std::getenv("DB_PASSWORD");

std::string connection_string = "host=" + std::string(db_host) +
								" port=" + std::string(db_port) +
								" dbname=" + std::string(db_name) +
								" user=" + std::string(db_user) +
								" password=" + std::string(db_password);

#define PORTA 8000
#define REDIS_PORTA 6379

// LEMBRAR DE MODIFICAR CREDENCIAIS PARA DB DO MICROSERVICO
/* ConnectionPool connection_pool("host=localhost port=5432 dbname=mydatabase user=myuser password=mypassword", 10); */
ConnectionPool connection_pool(connection_string, 10);
RedisConnectionPool redis_pool("redis", REDIS_PORTA, 10);

// FAZENDO LISTENING POR RECURSAO
void start_accepting(net::io_context &ioc, tcp::acceptor &acceptor)
{
	acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket)
						  {
        if (!ec) {
            std::thread(do_session, std::move(socket)).detach();
        }
        start_accepting(ioc, acceptor); });
}

int main()
{
	try
	{
		net::io_context ioc;

		tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), PORTA));
		std::cout << GREEN_TEXT << "[CARGAS]" << RESET_COLOR << ": Iniciando servidor na porta " << PORTA << " | Initializing server on port " << PORTA << "!" << std::endl;

		start_accepting(ioc, acceptor);

		ioc.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
