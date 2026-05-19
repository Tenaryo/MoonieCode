#pragma once

#include <string>

#include "response_parser.hpp"

class ToolExecutor {
  public:
    [[nodiscard]] static auto execute(const ToolCall& tool_call) -> std::string;

  private:
    [[nodiscard]] static auto handle_read(const nlohmann::json& arguments)
        -> std::string;
    [[nodiscard]] static auto handle_write(const nlohmann::json& arguments)
        -> std::string;
};
