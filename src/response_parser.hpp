#pragma once

#include <string>
#include <string_view>
#include <variant>

#include <nlohmann/json.hpp>

struct ContentResult {
    std::string content;
};

struct ToolCall {
    std::string name;
    nlohmann::json arguments;
};

using ParsedResponse = std::variant<ContentResult, ToolCall>;

class ResponseParser {
  public:
    [[nodiscard]] static auto parse(std::string_view response_text)
        -> ParsedResponse;
};
