cmake_minimum_required(VERSION 3.10)

# 如果支持，请为 MSVC 编译器启用热重载。
project("Rin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(ixwebsocket REQUIRED)

add_executable(${PROJECT_NAME} "main.cpp" "bot.cpp" "bot.h" "nlohmann/json.hpp" "Satori/Satori.cpp" "Satori/Satori.h" "httpapi/MessageApi.cpp" "httpapi/MessageApi.h" "LLMClient/LLMClient.cpp" "LLMClient/LLMClient.h")

target_link_libraries(${PROJECT_NAME}
    ixwebsocket::ixwebsocket
)
