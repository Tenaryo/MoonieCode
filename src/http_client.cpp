#include "http_client.hpp"

#include <cpr/cpr.h>

#include <stdexcept>

HttpClient::HttpClient(std::string api_key, std::string base_url)
    : api_key_(std::move(api_key)), base_url_(std::move(base_url)) {
}

auto HttpClient::chat_completion(const nlohmann::json& request_body)
    -> std::string {
    cpr::Response response
        = cpr::Post(cpr::Url{base_url_ + "/chat/completions"},
                    cpr::Header{{"Authorization", "Bearer " + api_key_},
                                {"Content-Type", "application/json"}},
                    cpr::Body{request_body.dump()});

    if (response.status_code != 200) {
        throw std::runtime_error("HTTP error: "
                                 + std::to_string(response.status_code));
    }

    return response.text;
}
