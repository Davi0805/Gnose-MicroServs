#include "Includes/CargasInfo_utils.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <pqxx/pqxx>


#include <nlohmann/json.hpp>

/* CREATE TABLE shipments (
  id SERIAL PRIMARY KEY,
  company_id VARCHAR(255) NOT NULL,
  costumer VARCHAR(255) NOT NULL,
  shipping_status VARCHAR(255) NOT NULL,
  type_of_load VARCHAR(255) NOT NULL,
  origin VARCHAR(255) NOT NULL,
  destination VARCHAR(255) NOT NULL,
  weight DECIMAL(10, 2) NOT NULL,
  cost DECIMAL(10, 2) NOT NULL,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  ce_mercante VARCHAR(255),
  ce_m_filename VARCHAR(255),
  ce_m_filepath VARCHAR(255),
  packinglist_filename VARCHAR(255),
  packinglist_filepath VARCHAR(255),
  ncm VARCHAR(255),
  cntrnum VARCHAR(255),
  referenciaid VARCHAR(255),
  afrmmpago VARCHAR(255),
  afrmmfilename VARCHAR(255),
  afrmmfilepath VARCHAR(255),
  blnum VARCHAR(255),
  bl_filename VARCHAR(255),
  bl_filepath VARCHAR(255),
  nfnum VARCHAR(255)
); */

/* {
    "costumer": "Jao te",
    "company_id": "2",
    "shipping_status": "Transito",
    "type_of_load": "copos",
    "origin": "Pequim",
    "destination": "Rio de Janeiro",
    "weight": "2230.00",
    "cost": "223.00"
} */


using json = nlohmann::json;
extern ConnectionPool connection_pool;

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

Postdata parse_post(const std::string req_body)
{
	Postdata result;
	json data_body = json::parse(req_body);

    auto get_string = [&](const std::string& key, const std::string& default_value) {
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


void handle_request(http::request<http::string_body> const& req, http::response<http::string_body>& res)
{
    if (req.method() == http::verb::post)
    {
        try
        {
            // Here you would parse the request body if needed
            /* json data_body = json::parse(req.body()); */
			Postdata data = parse_post(req.body());
			std::cout << GREEN_TEXT << "[GET]" << RESET_COLOR << ": " << req.body() << std::endl;

            // Acquire a connection from the pool
            auto conn = connection_pool.acquire();
            pqxx::work txn(*conn); // Start a transaction

            // Example database operation
			// Prepare the statement with the correct SQL syntax
            conn->prepare("insert_shipments", "INSERT INTO shipments (costumer, company_id, shipping_status, type_of_load, origin, destination, weight, cost) VALUES ($1, $2, $3, $4, $5, $6, $7, $8)");

            // Execute the prepared statement with parameters using exec_prepared
            txn.exec_prepared("insert_shipments", data.costumer, data.company_id, data.shipping_status, data.type_of_load, data.origin, data.destination, data.weight, data.cost);

            // Commit the transaction
            txn.commit();

            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = "Post funcionando";
        }
        catch (const std::exception& e)
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
		auto conn = connection_pool.acquire();
        pqxx::work txn(*conn);


		pqxx::result result = txn.exec("SELECT * FROM shipments");

        // Convert the result to JSON
        json json_result = json::array();
			for (const auto& row : result)
			{
				json json_row;
				for (const auto& field : row)
				{
					json_row[field.name()] = field.c_str();
				}
				json_result.push_back(json_row);

				// Commit the transaction
				txn.commit();

				// Send the response
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				res.body() = json_result.dump();
			}
			std::cout << GREEN_TEXT << "[GET]" << RESET_COLOR << ": " << res.body() << std::endl;
        }
		catch (const std::exception& e)
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
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}
