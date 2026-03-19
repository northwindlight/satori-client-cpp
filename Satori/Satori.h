#pragma once
#include <string>
#include <optional>
#include <vector>
#include "../nlohmann/json.hpp"

namespace satori
{
    struct User
    {
        std::string id;
        std::optional<std::string>  name;
        std::optional<std::string>  nick;
        std::optional<std::string>  avatar;
        std::optional<bool>         is_bot;
    };
    void from_json(const nlohmann::json& j, User& u);

    enum ChannelType
    {
        TEXT,
        DIRECT,
        CATEGORY,
        VOICE,
    };
    void from_json(const nlohmann::json& j, ChannelType& t);

    struct Channel
    {
        std::string id;
        ChannelType  type;
        std::optional<std::string>  name;
        std::optional<std::string>  parent_id;
    };
    void from_json(const nlohmann::json& j, Channel& c);

    struct Guild
    {
        std::string id;
        std::optional<std::string>  name;
        std::optional<std::string>  avatar;
    };
    void from_json(const nlohmann::json& j, Guild& g);

    struct GuildRole
    {
        std::string id;
        std::optional<std::string>  name;
    };
    void from_json(const nlohmann::json& j, GuildRole& r);

    struct GuildMember
    {
        std::optional<User> user;
        std::optional<std::string>  nick;
        std::optional<std::string>  avatar;
        std::optional<long long> joined_at;
        std::optional<std::vector<GuildRole>> role;
    };
    void from_json(const nlohmann::json& j, GuildMember& m);

    struct Message
    {
        std::string id;
        std::string content;
        std::optional<Channel> channel;
        std::optional<Guild>  guild;
        std::optional<GuildMember> member;
        std::optional<User> user;
        std::optional<long long>  created_at;
        std::optional<long long>  updated_at;
    };
    void from_json(const nlohmann::json& j, Message& m);

    enum LoginStatus
    {
        OFFLINE,
        ONLINE,
        CONNECT,
        DISCONNECT,
        RECONNECT,
    };
    void from_json(const nlohmann::json& j, LoginStatus& s);

    struct Login
    {
        int sn;
        std::optional<std::string> platform;
        std::optional<User> user;
        LoginStatus status;
        std::string adapter;
        std::optional<std::vector<std::string>> features;
    };
    void from_json(const nlohmann::json& j, Login& l);

    struct Argv
    {
        std::string name;
        std::vector<nlohmann::json> arguments; //array of any type
        nlohmann::json options; // can be any type
    };
    void from_json(const nlohmann::json& j, Argv& a);

    struct Button
    {
        std::string id;
    };
    void from_json(const nlohmann::json& j, Button& b);

    struct Event
    {
        int sn;
        std::string type;
        long long timestamp; 
        std::optional<Login> login;
        std::optional<Argv> argv;
        std::optional<Button> button;
        std::optional<Channel> channel;
        std::optional<Guild> guild;
        std::optional<GuildMember> member;
        std::optional<Message> message;
        std::optional<User> operator_; // "operator" is a reserved keyword in C++
        std::optional<GuildRole> role;
        std::optional<User> user;
        std::optional<nlohmann::json> referrer; // can be any type
    };
    void from_json(const nlohmann::json& j, Event& e);
}