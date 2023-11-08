cmake_minimum_required(VERSION 3.27)
project(json-combine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(json-combine src/main.cpp)

set_target_properties(json-combine PROPERTIES CXX_STANDARD 23)
target_compile_definitions(json-combine PRIVATE FMT_HEADER_ONLY)
target_compile_options(json-combine PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(json-combine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/json/include ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)
