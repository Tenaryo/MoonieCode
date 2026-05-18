cmake_minimum_required(VERSION 3.21)

project(
    claude-code
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (DEFINED VCPKG_TARGET_TRIPLET)
    find_package(cpr CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(GTest CONFIG REQUIRED)
else ()
    find_package(nlohmann_json REQUIRED)

    include(FetchContent)
    FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.5
    )
    set(CPR_ENABLE_SSL
        OFF
        CACHE BOOL "" FORCE
    )
    set(CPR_USE_SYSTEM_CURL
        ON
        CACHE BOOL "" FORCE
    )
    FetchContent_MakeAvailable(cpr)

    find_package(GTest REQUIRED)
endif ()

add_library(claude-code_core INTERFACE)
target_include_directories(claude-code_core INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(claude-code_core INTERFACE cxx_std_23)
target_compile_options(
    claude-code_core
    INTERFACE -Wall
              -Wextra
              -Wpedantic
              -Werror
              -Wshadow
              -Wconversion
              $<$<CONFIG:Debug>:-g
              -O0>
              $<$<CONFIG:Release>:-O3>
)

add_executable(claude-code src/main.cpp)
target_link_libraries(
    claude-code PRIVATE claude-code_core cpr::cpr nlohmann_json::nlohmann_json
)

option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

if (ENABLE_SANITIZERS)
    target_compile_options(
        claude-code_core INTERFACE -fsanitize=address,undefined
                                   -fno-omit-frame-pointer
    )
    target_link_options(claude-code_core INTERFACE -fsanitize=address,undefined)
endif ()

option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
