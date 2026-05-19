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
        std::vector<ToolCall> tool_calls;
        for (const auto& call : message["tool_calls"]) {
            const auto& func = call["function"];
            auto args_str = func["arguments"].get<std::string>();
            tool_calls.push_back(ToolCall{
                .id = call["id"].get<std::string>(),
                .name = func["name"].get<std::string>(),
                .arguments = nlohmann::json::parse(args_str),
            });
        }
        return tool_calls;
    }

    if (!message.contains("content") || message["content"].is_null()) {
        throw std::runtime_error("Invalid response: no content or tool_calls");
    }

    return ContentResult{
        .content = message["content"].get<std::string>(),
    };
}
