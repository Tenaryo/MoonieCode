#include <gtest/gtest.h>

#include <fstream>

#include "tool_executor.hpp"

#include <nlohmann/json.hpp>

TEST(ToolExecutorTest, ExecuteReadExistingFile) {
    const std::string kTestPath = "/tmp/test_read.txt";
    const std::string kContent = "hello from test file";

    {
        std::ofstream file(kTestPath);
        file << kContent;
    }

    ToolCall tool_call;
    tool_call.name = "Read";
    tool_call.arguments = nlohmann::json::object({{"file_path", kTestPath}});

    EXPECT_EQ(ToolExecutor::execute(tool_call), kContent);
}
