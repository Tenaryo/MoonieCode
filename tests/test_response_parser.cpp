#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>

#include "response_parser.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(ResponseParserTest, ParseContentResponse) {
    auto response = json::parse(R"({
        "choices": [{
            "message": {
                "role": "assistant",
                "content": "Hello, world!"
            }
        }]
    })");

    auto result = ResponseParser::parse(response.dump());

    ASSERT_TRUE(std::holds_alternative<ContentResult>(result));
    EXPECT_EQ(std::get<ContentResult>(result).content, "Hello, world!");
}

TEST(ResponseParserTest, ParseToolCallResponse) {
    auto response = json::parse(R"({
        "choices": [{
            "message": {
                "role": "assistant",
                "content": null,
                "tool_calls": [{
                    "id": "call_abc123",
                    "type": "function",
                    "function": {
                        "name": "Read",
                        "arguments": "{\"file_path\": \"/tmp/test.txt\"}"
                    }
                }]
            }
        }]
    })");

    auto result = ResponseParser::parse(response.dump());

    ASSERT_TRUE(std::holds_alternative<ToolCall>(result));
    const auto& tool_call = std::get<ToolCall>(result);
    EXPECT_EQ(tool_call.name, "Read");
    EXPECT_EQ(tool_call.arguments["file_path"].get<std::string>(),
              "/tmp/test.txt");
}

TEST(ResponseParserTest, ParseInvalidResponseThrows) {
    auto response = json::parse(R"({"error": "something went wrong"})");

    EXPECT_THROW(
        { [[maybe_unused]] auto res = ResponseParser::parse(response.dump()); },
        std::exception);
}
