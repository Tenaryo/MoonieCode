# MoonieCode

A minimal, educational implementation of an LLM-powered AI coding assistant in C++23. This is not production-grade software — it's a learning project that demonstrates the core concepts behind AI coding tools: tool-calling, agent loops, and integrating LLMs with real-world actions.

## Architecture

```
moonie-code
├── src/
│   ├── main.cpp                  Entry point & agent loop (up to 30 iterations)
│   ├── http_client/              HTTP client for LLM API (OpenRouter / Anthropic-compatible)
│   ├── response_parser/          Parses LLM response into content or tool calls
│   ├── tool_executor/            Executes tool calls: Read, Write, Bash
│   └── util/                     overloaded pattern for std::visit
├── tests/                        GoogleTest unit tests
├── build.sh                      Build script (debug/release)
├── run_tests.sh                  Build and run all tests
└── CMakeLists.txt                CMake 3.21+, C++23, Ninja
```

### Tools

| Tool | Description |
|---|---|
| **Read** | Reads file contents from a given path |
| **Write** | Writes content to a file, creating parent directories as needed |
| **Bash** | Executes shell commands and captures stdout, stderr, and exit code |

## Requirements

- **GCC 13** (or any C++23-compliant compiler)
- **CMake 3.21+**
- **Ninja**
- **nlohmann_json** (system package)
- **libcurl** (system package, for HTTP requests)

Install on Ubuntu/Debian:

```sh
sudo apt install g++-13 ninja-build cmake nlohmann-json3-dev libcurl4-openssl-dev
```

## Build

```sh
./build.sh            # Debug build (default, -O0 -g)
./build.sh Release    # Release build (-O3)
```

The output binary is `build/moonie-code`.

### Sanitizer Build

```sh
cmake -B build-san -G Ninja -DENABLE_SANITIZERS=ON
cmake --build build-san
```

## Usage

Set your OpenRouter API key:

```sh
export OPENROUTER_API_KEY="your-api-key"
```

Run a prompt:

```sh
./build/moonie-code -p "list all .cpp files in the project"
```

Optionally set a custom base URL:

```sh
export OPENROUTER_BASE_URL="https://openrouter.ai/api/v1"
```

## Test

```sh
./run_tests.sh
```

This configures, builds, and runs all GoogleTest tests with colored output.

## License

MIT © 2026 Tenaryo
