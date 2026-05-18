#include <cpr/cpr.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

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

        cpr::Response response
            = cpr::Post(cpr::Url{base_url + "/chat/completions"},
                        cpr::Header{{"Authorization", "Bearer " + api_key},
                                    {"Content-Type", "application/json"}},
                        cpr::Body{request_body.dump()});

        if (response.status_code != 200) [[unlikely]] {
            std::cerr << "HTTP error: " << response.status_code << '\n';
            return EXIT_FAILURE;
        }

        json result = json::parse(response.text);

        if (!result.contains("choices") || result["choices"].empty())
            [[unlikely]] {
            std::cerr << "No choices in response\n";
            return EXIT_FAILURE;
        }

        const auto& choice = result["choices"][0];
        if (!choice.contains("message")
            || !choice["message"].contains("content")) [[unlikely]] {
            std::cerr
                << "Invalid response format: missing message or content\n";
            return EXIT_FAILURE;
        }

        std::cout << choice["message"]["content"].get<std::string>() << '\n';
    } catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
