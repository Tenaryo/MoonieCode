#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>

#include "http_client.hpp"
#include "response_parser.hpp"
#include "tool_executor.hpp"
#include "util/overloaded.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3 || std::string_view{argv[1]} != "-p") [[unlikely]] {
        std::cerr << "Expected first argument to be '-p'\n";
        return EXIT_FAILURE;
    }

    std::string prompt = argv[2];

    if (prompt.empty()) [[unlikely]] {
        std::cerr << "Prompt must not be empty\n";
        return EXIT_FAILURE;
    }

    const char* api_key_env = std::getenv("OPENROUTER_API_KEY");
    const char* base_url_env = std::getenv("OPENROUTER_BASE_URL");

    std::string api_key = api_key_env != nullptr ? api_key_env : "";
    std::string base_url = base_url_env != nullptr
                               ? base_url_env
                               : "https://openrouter.ai/api/v1";

    if (api_key.empty()) [[unlikely]] {
        std::cerr << "OPENROUTER_API_KEY is not set\n";
        return EXIT_FAILURE;
    }

    try {
        const json tools_array = json::array(
            {{{"type", "function"},
              {"function",
               {{"name", "Read"},
                {"description", "Read and return the contents of a file"},
                {"parameters",
                 {{"type", "object"},
                  {"properties",
                   {{"file_path",
                     {{"type", "string"},
                      {"description", "The path to the file to read"}}}}},
                  {"required", json::array({"file_path"})}}}}}}});

        json request_body
            = {{"model", "anthropic/claude-haiku-4.5"},
               {"messages",
                json::array({{{"role", "user"}, {"content", prompt}}})},
               {"tools", tools_array}};

        HttpClient client(api_key, base_url);
        auto response_text = client.chat_completion(request_body);

        auto parsed = ResponseParser::parse(response_text);

        std::visit(overloaded{
                       [](const ContentResult& result) {
                           std::cout << result.content << '\n';
                       },
                       [](const ToolCall& tool_call) {
                           std::cout << ToolExecutor::execute(tool_call)
                                     << '\n';
                       },
                   },
                   parsed);

    } catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
