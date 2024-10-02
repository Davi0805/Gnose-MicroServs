#include "Includes/third_party/jwt-cpp/include/jwt-cpp/jwt.h"
#include <iostream>
#include <string>
#include "Includes/CargasInfo_utils.hpp"


std::string generate_jwt(const std::string& user_id, const std::string& company_id, const std::string& permission_level) {

    auto token = jwt::create()
        .set_issuer("Auth")
        .set_type("JWT")
        .set_payload_claim("id", jwt::claim(user_id))
        .set_payload_claim("company_id", jwt::claim(company_id))
		.set_payload_claim("permission", jwt::claim(permission_level))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{60})
        .sign(jwt::algorithm::hs256{"token-secreto"});

    return token;
}

jwt_data jwt_checker(const std::string& token) {
    jwt_data result;
    try {

    auto decoded = jwt::decode(token);


    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{"token-secreto"})
        .with_issuer("Auth");

    verifier.verify(decoded);


    auto exp = decoded.get_expires_at();
    if (exp < std::chrono::system_clock::now()) {
        throw std::runtime_error("Token expirado");
    }

    result.user_id = decoded.get_payload_claim("id").as_string();
    result.company_id = decoded.get_payload_claim("company_id").as_string();
	result.permission_level = decoded.get_payload_claim("permission").as_string();
}
catch (const jwt::error::token_verification_exception& e) {
    std::cerr << "Verificacao do tokem falhou: " << e.what() << std::endl;
    throw;
}
return result;
}

