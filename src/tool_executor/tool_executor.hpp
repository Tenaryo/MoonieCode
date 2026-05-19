#pragma once

#include <string>

#include "response_parser/response_parser.hpp"

#include <nlohmann/json.hpp>

class ToolExecutor {
  public:
    [[nodiscard]] static auto execute(const ToolCall& tool_call) -> std::string;

    [[nodiscard]] static auto tools_schema() -> nlohmann::json;

  private:
    [[nodiscard]] static auto handle_read(const nlohmann::json& arguments)
        -> std::string;
    [[nodiscard]] static auto handle_write(const nlohmann::json& arguments)
        -> std::string;
};
