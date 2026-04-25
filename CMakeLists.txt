cmake_minimum_required(VERSION 3.10)

project("satori_client")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ixwebsocket REQUIRED)

set(COMMON_SOURCES
    bot.cpp
    Satori/Satori.cpp
    api/MessageApi.cpp
    api/ChannelApi.cpp
    api/UserApi.cpp
    api/GuildApi.cpp
    api/FriendApi.cpp
    api/LoginApi.cpp
    api/ReactionApi.cpp
    LLMClient/LLMClient.cpp
    LLMClient/LLMAgent.cpp
    tinyxml2/tinyxml2.cpp
)

add_executable(${PROJECT_NAME}
    demo.cpp
    ${COMMON_SOURCES}
)

add_executable(demo_toolcall
    demo_toolcall.cpp
    ${COMMON_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    ixwebsocket::ixwebsocket
)

target_link_libraries(demo_toolcall
    ixwebsocket::ixwebsocket
)