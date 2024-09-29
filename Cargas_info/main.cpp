#include "Includes/CargasInfo_utils.hpp"
#include <iostream>
#include <thread>
#include <pqxx/pqxx>

/* g++ -std=c++17 -o server main.cpp CargasInfo_utils.cpp -lboost_system -lpthread -lpqxx -lpq */

#define PORTA 8000
// Global connection pool
ConnectionPool connection_pool("host=localhost port=5432 dbname=mydatabase user=myuser password=mypassword", 10);

void start_accepting(net::io_context& ioc, tcp::acceptor& acceptor) {
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            // Handle the connection in the io_context
            std::thread(do_session, std::move(socket)).detach(); // Use a new thread for each session
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
