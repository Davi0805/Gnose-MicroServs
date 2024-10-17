#include "Includes/third_party/jwt-cpp/include/jwt-cpp/jwt.h"
#include <iostream>
#include <string>
#include "Includes/CargasInfo_utils.hpp"

class Error_handler
{
	public:
		Error_handler(string cerr, http::response<http::string_body> &res)
		{
			erro = cerr;
			error_res = res;
		}
		http::response<http::string_body> gerar_resposta()
		{
			if (erro == "Token nao encontrado!")
				error_res.result(http::status::unauthorized);
				error_res.set(http::field::content_type, "application/json");
				error_res.body() = "Unauthorized";

			return(error_res);
		}
	private:
		string erro;
		http::response<http::string_body> error_res;
};
