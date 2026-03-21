cmake_minimum_required(VERSION 3.10)

project("Rin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ixwebsocket REQUIRED)

file(GLOB SUBPROCESS_SOURCES "subprocess/*.cpp")

add_executable(${PROJECT_NAME}
    main.cpp
    bot.cpp
    Satori/Satori.cpp
    httpapi/MessageApi.cpp
    LLMClient/LLMClient.cpp
    LLMClient/LLMAgent.cpp
    ${SUBPROCESS_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ixwebsocket::ixwebsocket
)