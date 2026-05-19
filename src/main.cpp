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
                  {"required", json::array({"file_path"})}}}}}},
             {{{"type", "function"},
               {"function",
                {{"name", "Write"},
                 {"description", "Write content to a file"},
                 {"parameters",
                  {{"type", "object"},
                   {"properties",
                    {{"file_path",
                      {{"type", "string"},
                       {"description", "The path of the file to write to"}}},
                     {"content",
                      {{"type", "string"},
                       {"description", "The content to write to the file"}}}},
                    {"required",
                     json::array({"file_path", "content"})}}}}}}}}});

        json messages = json::array({{{"role", "user"}, {"content", prompt}}});

        HttpClient client(api_key, base_url);

        constexpr int kMaxIterations = 30;
        int iteration = 0;
        bool done = false;

        while (!done) {
            if (++iteration > kMaxIterations) [[unlikely]] {
                std::cerr << "Error: exceeded maximum iterations ("
                          << kMaxIterations << ")\n";
                return EXIT_FAILURE;
            }

            json request_body = {{"model", "anthropic/claude-haiku-4.5"},
                                 {"messages", messages},
                                 {"tools", tools_array}};

            auto response_text = client.chat_completion(request_body);
            auto parsed = ResponseParser::parse(response_text);

            std::visit(
                overloaded{
                    [&](const ContentResult& result) {
                        messages.push_back({{"role", "assistant"},
                                            {"content", result.content}});
                        std::cout << result.content << '\n';
                        done = true;
                    },
                    [&](const std::vector<ToolCall>& tool_calls) {
                        json tc_array = json::array();
                        for (const auto& call : tool_calls) {
                            tc_array.push_back(
                                {{"id", call.id},
                                 {"type", "function"},
                                 {"function",
                                  {{"name", call.name},
                                   {"arguments", call.arguments.dump()}}}});
                        }
                        messages.push_back({{"role", "assistant"},
                                            {"content", nullptr},
                                            {"tool_calls", tc_array}});

                        for (const auto& call : tool_calls) {
                            std::string result;
                            try {
                                result = ToolExecutor::execute(call);
                            } catch (const std::exception& e) {
                                result = std::string("Error: ") + e.what();
                            }
                            messages.push_back({{"role", "tool"},
                                                {"tool_call_id", call.id},
                                                {"content", result}});
                        }
                    },
                },
                parsed);
        }

    } catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
