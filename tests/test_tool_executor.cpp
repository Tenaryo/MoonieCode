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

TEST(ToolExecutorTest, ExecuteBashSimpleCommand) {
    ToolCall tool_call;
    tool_call.name = "Bash";
    tool_call.arguments = nlohmann::json::object({{"command", "echo hello"}});

    const auto result = ToolExecutor::execute(tool_call);
    EXPECT_NE(result.find("hello"), std::string::npos);
    EXPECT_NE(result.find("[exit code: 0]"), std::string::npos);
}

TEST(ToolExecutorTest, ExecuteBashDeleteFile) {
    const std::string kTestPath = "/tmp/test_bash_delete_me.txt";

    {
        std::ofstream file(kTestPath);
        file << "temp content";
    }
    ASSERT_TRUE(fs::exists(kTestPath));

    ToolCall tool_call;
    tool_call.name = "Bash";
    tool_call.arguments
        = nlohmann::json::object({{"command", "rm " + kTestPath}});

    const auto result = ToolExecutor::execute(tool_call);
    EXPECT_NE(result.find("[exit code: 0]"), std::string::npos);
    EXPECT_FALSE(fs::exists(kTestPath));
}

TEST(ToolExecutorTest, ExecuteBashFailingCommand) {
    ToolCall tool_call;
    tool_call.name = "Bash";
    tool_call.arguments
        = nlohmann::json::object({{"command", "ls /nonexistent_path_xyz"}});

    const auto result = ToolExecutor::execute(tool_call);
    const auto exit_pos = result.find("[exit code: ");
    ASSERT_NE(exit_pos, std::string::npos);
    const auto code_str = result.substr(
        exit_pos + 12, result.find(']', exit_pos) - exit_pos - 12);
    const int exit_code = std::stoi(code_str);
    EXPECT_NE(exit_code, 0);
}
