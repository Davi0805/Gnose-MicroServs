#include "Includes/CargasInfo_utils.hpp"
#include <iostream>
#include <thread>
#include <pqxx/pqxx>

/* g++ -std=c++17 -o server main.cpp User_utils.cpp Auth_utils.cpp -lboost_system -lpthread -lpqxx -lpq -Ithird_party/jwt-cpp/include -lcrypto
 */


#define PORTA 8001
// Global connection pool
ConnectionPool connection_pool("host=localhost port=5432 dbname=mydatabase user=myuser password=mypassword", 10);

void start_accepting(net::io_context& ioc, tcp::acceptor& acceptor) {
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::thread(do_session, std::move(socket)).detach(); // Usa nova thread ou conexao
        }
        // Continue accepting more connections
        start_accepting(ioc, acceptor);
    });
}

int main()
{
    try
    {
        net::io_context ioc;

        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), PORTA));
        std::cout << GREEN_TEXT << "[LOG]" << RESET_COLOR << ": Iniciando servidor na porta " << PORTA << " | Initializing server on port " << PORTA << "!" << std::endl;

        // Start accepting connections asynchronously
        start_accepting(ioc, acceptor);

        // Run the io_context
        ioc.run();
    }
    catch (const std::exception& e) {
        // Handle any errors and display a message
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
