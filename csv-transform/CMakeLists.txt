 cmake_minimum_required(VERSION 3.27)
project(csv-transform)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(csv-transform src/main.cpp)

set_target_properties(csv-transform PROPERTIES CXX_STANDARD 23)
target_compile_options(csv-transform PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(csv-transform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/json/include ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/rapidcsv/src)
