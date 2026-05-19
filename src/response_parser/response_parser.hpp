#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

struct ContentResult {
    std::string content;
};

struct ToolCall {
    std::string id;
    std::string name;
    nlohmann::json arguments;
};

using ParsedResponse = std::variant<ContentResult, std::vector<ToolCall>>;

class ResponseParser {
  public:
    [[nodiscard]] static auto parse(std::string_view response_text)
        -> ParsedResponse;
};
