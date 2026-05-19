#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "tool_executor/tool_executor.hpp"

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

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

TEST(ToolExecutorTest, ExecuteWriteNewFile) {
    const std::string kTestPath = "/tmp/test_write_new.txt";
    fs::remove(kTestPath);

    ToolCall tool_call;
    tool_call.name = "Write";
    tool_call.arguments = nlohmann::json::object(
        {{"file_path", kTestPath}, {"content", "hello world"}});

    EXPECT_EQ(ToolExecutor::execute(tool_call),
              "File written successfully: " + kTestPath);

    std::ifstream file(kTestPath);
    ASSERT_TRUE(file.is_open());
    std::string content;
    std::getline(file, content, '\0');
    EXPECT_EQ(content, "hello world");

    fs::remove(kTestPath);
}

TEST(ToolExecutorTest, ExecuteWriteOverwriteFile) {
    const std::string kTestPath = "/tmp/test_write_overwrite.txt";

    {
        std::ofstream file(kTestPath);
        file << "old content";
    }

    ToolCall tool_call;
    tool_call.name = "Write";
    tool_call.arguments = nlohmann::json::object(
        {{"file_path", kTestPath}, {"content", "new content"}});

    EXPECT_EQ(ToolExecutor::execute(tool_call),
              "File written successfully: " + kTestPath);

    std::ifstream file(kTestPath);
    ASSERT_TRUE(file.is_open());
    std::string content;
    std::getline(file, content, '\0');
    EXPECT_EQ(content, "new content");

    fs::remove(kTestPath);
}

TEST(ToolExecutorTest, ExecuteWriteCreateParentDirs) {
    const std::string kTestDir = "/tmp/test_write_subdir";
    const std::string kTestPath = kTestDir + "/nested/file.txt";
    fs::remove_all(kTestDir);

    ToolCall tool_call;
    tool_call.name = "Write";
    tool_call.arguments = nlohmann::json::object(
        {{"file_path", kTestPath}, {"content", "nested content"}});

    EXPECT_EQ(ToolExecutor::execute(tool_call),
              "File written successfully: " + kTestPath);

    std::ifstream file(kTestPath);
    ASSERT_TRUE(file.is_open());
    std::string content;
    std::getline(file, content, '\0');
    EXPECT_EQ(content, "nested content");

    fs::remove_all(kTestDir);
}
