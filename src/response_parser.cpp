#include "response_parser.hpp"

#include <stdexcept>
#include <string>

auto ResponseParser::parse(std::string_view response_text) -> ParsedResponse {
    auto result = nlohmann::json::parse(response_text);

    if (!result.contains("choices") || result["choices"].empty()) {
        throw std::runtime_error("No choices in response");
    }

    const auto& message = result["choices"][0]["message"];

    if (message.contains("tool_calls") && !message["tool_calls"].empty()) {
        const auto& tool_call = message["tool_calls"][0];
        const auto& func = tool_call["function"];

        auto args_str = func["arguments"].get<std::string>();
        return ToolCall{
            .name = func["name"].get<std::string>(),
            .arguments = nlohmann::json::parse(args_str),
        };
    }

    if (!message.contains("content") || message["content"].is_null()) {
        throw std::runtime_error("Invalid response: no content or tool_calls");
    }

    return ContentResult{
        .content = message["content"].get<std::string>(),
    };
}
