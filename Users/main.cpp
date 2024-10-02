#include "Includes/CargasInfo_utils.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <pqxx/pqxx>

/* g++ -std=c++17 -o server main.cpp User_utils.cpp Auth_utils.cpp -lboost_system -lpthread -lpqxx -lpq -Ithird_party/jwt-cpp/include -lcrypto
 */
/*
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    first_name VARCHAR(255),
    last_name VARCHAR(255),
    company_id VARCHAR(255) NOT NULL,
    permission VARCHAR(1)
); */
/*
INSERT INTO users (username, password, email, first_name, last_name, company_id, permission) VALUES
('user1', 'password1', 'user1@example.com', 'John', 'Doe', 'company1', '0'),
('user2', 'password2', 'user2@example.com', 'Jane', 'Smith', 'company1', '1'),
('user3', 'password3', 'user3@example.com', 'Alice', 'Johnson', 'company2', '2'),
('user4', 'password4', 'user4@example.com', 'Bob', 'Brown', 'company2', '3'),
('user5', 'password5', 'user5@example.com', 'Charlie', 'Davis', 'company3', '0'),
('user6', 'password6', 'user6@example.com', 'David', 'Miller', 'company3', '1'),
('user7', 'password7', 'user7@example.com', 'Eve', 'Wilson', 'company4', '2'),
('user8', 'password8', 'user8@example.com', 'Frank', 'Moore', 'company4', '3'),
('user9', 'password9', 'user9@example.com', 'Grace', 'Taylor', 'company5', '0'),
('user10', 'password10', 'user10@example.com', 'Hank', 'Anderson', 'company5', '1'); */

/* docker exec -it gnose-microservs-redis-1 redis-cli */

/* docker exec -it gnose-microservs-db-1 psql -U myuser -d mydatabase */




const char* db_host = std::getenv("DB_HOST");
const char* db_port = std::getenv("DB_PORT");
const char* db_name = std::getenv("DB_NAME");
const char* db_user = std::getenv("DB_USER");
const char* db_password = std::getenv("DB_PASSWORD");

    std::string connection_string = "host=" + std::string(db_host) +
                                    " port=" + std::string(db_port) +
                                    " dbname=" + std::string(db_name) +
                                    " user=" + std::string(db_user) +
                                    " password=" + std::string(db_password);



// LEMBRAR DE MODIFICAR CREDENCIAIS PARA DB DO MICROSERVICO
/* ConnectionPool connection_pool("host=db port=5432 dbname=mydatabase user=myuser password=mypassword", 10); */
ConnectionPool connection_pool(connection_string, 10);
RedisConnectionPool redis_pool("redis", 6379, 10);


#define PORTA 8001

// FAZENDO LISTENING POR RECURSAO
void start_accepting(net::io_context &ioc, tcp::acceptor &acceptor)
{
	acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket)
						  {
        if (!ec) {
            std::thread(do_session, std::move(socket)).detach(); // Usa nova thread ou conexao
        }
        start_accepting(ioc, acceptor); });
}

int main()
{
	try
	{

		net::io_context ioc;

		tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), PORTA));
		std::cout << GREEN_TEXT << "[LOG]" << RESET_COLOR << ": Iniciando servidor na porta " << PORTA << " | Initializing server on port " << PORTA << "!" << std::endl;
/* 		redisContext *cache_context = redisConnect("127.0.0.1", 6379);
		if (cache_context == nullptr || cache_context->err)
		{
			std::cout << "Error: " << cache_context->errstr << std::endl;
			return 1;
		}

		redisReply *reply = (redisReply *)redisCommand(cache_context, "PING");
		std::cout << "PING: " << reply->str << std::endl;
		freeReplyObject(reply); */

		start_accepting(ioc, acceptor);

		ioc.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
