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

    ASSERT_TRUE(std::holds_alternative<std::vector<ToolCall>>(result));
    const auto& tool_calls = std::get<std::vector<ToolCall>>(result);
    ASSERT_EQ(tool_calls.size(), 1);
    EXPECT_EQ(tool_calls[0].id, "call_abc123");
    EXPECT_EQ(tool_calls[0].name, "Read");
    EXPECT_EQ(tool_calls[0].arguments["file_path"].get<std::string>(),
              "/tmp/test.txt");
}

TEST(ResponseParserTest, ParseMultipleToolCalls) {
    auto response = json::parse(R"({
        "choices": [{
            "message": {
                "role": "assistant",
                "content": null,
                "tool_calls": [
                    {
                        "id": "call_1",
                        "type": "function",
                        "function": {
                            "name": "Read",
                            "arguments": "{\"file_path\": \"/tmp/a.txt\"}"
                        }
                    },
                    {
                        "id": "call_2",
                        "type": "function",
                        "function": {
                            "name": "Read",
                            "arguments": "{\"file_path\": \"/tmp/b.txt\"}"
                        }
                    }
                ]
            }
        }]
    })");

    auto result = ResponseParser::parse(response.dump());

    ASSERT_TRUE(std::holds_alternative<std::vector<ToolCall>>(result));
    const auto& tool_calls = std::get<std::vector<ToolCall>>(result);
    ASSERT_EQ(tool_calls.size(), 2);
    EXPECT_EQ(tool_calls[0].id, "call_1");
    EXPECT_EQ(tool_calls[0].name, "Read");
    EXPECT_EQ(tool_calls[0].arguments["file_path"].get<std::string>(),
              "/tmp/a.txt");
    EXPECT_EQ(tool_calls[1].id, "call_2");
    EXPECT_EQ(tool_calls[1].name, "Read");
    EXPECT_EQ(tool_calls[1].arguments["file_path"].get<std::string>(),
              "/tmp/b.txt");
}

TEST(ResponseParserTest, ParseInvalidResponseThrows) {
    auto response = json::parse(R"({"error": "something went wrong"})");

    EXPECT_THROW(
        { [[maybe_unused]] auto res = ResponseParser::parse(response.dump()); },
        std::exception);
}
