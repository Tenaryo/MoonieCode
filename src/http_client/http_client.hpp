#pragma once

#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

class HttpClient {
  public:
    HttpClient(std::string api_key, std::string base_url);

    [[nodiscard]] auto chat_completion(const nlohmann::json& request_body)
        -> std::string;

  private:
    std::string api_key_;
    std::string base_url_;
};
