cmake_minimum_required(VERSION 3.10)

project("satori_client")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ixwebsocket REQUIRED)

add_executable(${PROJECT_NAME}
    demo.cpp
    bot.cpp
    Satori/Satori.cpp
    httpapi/MessageApi.cpp
    LLMClient/LLMClient.cpp
    LLMClient/LLMAgent.cpp
    tinyxml2/tinyxml2.cpp
)


target_link_libraries(${PROJECT_NAME}
    ixwebsocket::ixwebsocket
)