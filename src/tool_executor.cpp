#include "tool_executor.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

auto ToolExecutor::execute(const ToolCall& tool_call) -> std::string {
    if (tool_call.name == "Read") {
        return handle_read(tool_call.arguments);
    }
    if (tool_call.name == "Write") {
        return handle_write(tool_call.arguments);
    }
    throw std::runtime_error("Unknown tool: " + tool_call.name);
}

auto ToolExecutor::handle_read(const nlohmann::json& arguments) -> std::string {
    if (!arguments.contains("file_path")) {
        throw std::runtime_error("Read tool: missing file_path");
    }

    const auto path = arguments["file_path"].get<std::string>();
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Read tool: cannot open file: " + path);
    }

    file.seekg(0, std::ios::end);
    std::string content;
    content.resize(static_cast<std::size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    file.read(content.data(), static_cast<std::streamsize>(content.size()));

    return content;
}

auto ToolExecutor::handle_write(const nlohmann::json& arguments)
    -> std::string {
    if (!arguments.contains("file_path")) {
        throw std::runtime_error("Write tool: missing file_path");
    }
    if (!arguments.contains("content")) {
        throw std::runtime_error("Write tool: missing content");
    }

    const auto path = arguments["file_path"].get<std::string>();
    const auto content = arguments["content"].get<std::string>();

    std::filesystem::create_directories(
        std::filesystem::path(path).parent_path());

    std::ofstream file(path);
    if (!file.is_open()) [[unlikely]] {
        throw std::runtime_error("Write tool: cannot write file: " + path);
    }
    file << content;

    return "File written successfully: " + path;
}
