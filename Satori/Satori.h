#pragma once
#include <string>
#include <optional>
#include <vector>
#include <variant>
#include "../tinyxml2/tinyxml2.h"
#include "../nlohmann/json.hpp"

namespace satori {
    namespace event {
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
            std::optional<std::vector<GuildRole>> roles;
        };
        void from_json(const nlohmann::json& j, GuildMember& m);

        struct Message
        {
            std::string id;
            std::optional<std::string> content;
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

        // status 和 adapter 在协议定义中为非可选字段，
        // 但部分下游 Satori 服务端实现在某些场景下可能不提供这两个字段，
        // 因此设为 optional 以兼容实际数据。
        struct Login
        {
            int sn;
            std::optional<std::string> platform;
            std::optional<User> user;
            std::optional<LoginStatus> status;
            std::optional<std::string> adapter;
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

        struct Emoji
        {
            std::string id;
            std::optional<std::string> name;
        };
        void from_json(const nlohmann::json& j, Emoji& e);

        struct Friend
        {
            std::optional<User> user;
            std::optional<std::string> nick;
        };
        void from_json(const nlohmann::json& j, Friend& f);

        struct Event
        {
            int sn;
            std::string type;
            long long timestamp; 
            std::optional<Login> login;
            std::optional<Argv> argv;
            std::optional<Button> button;
            std::optional<Channel> channel;
            std::optional<Emoji> emoji;
            std::optional<Friend> friend_;
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

    namespace element {
        struct At {
            std::optional<std::string> id;
            std::optional<std::string> name;
            std::optional<std::string> role;
            std::optional<std::string> type;
        };

        struct Sharp {
            std::string id;
            std::optional<std::string> name;
        };

        struct Emoji {
            std::optional<std::string> id;
            std::optional<std::string> name;
        };

        struct Link {
            std::string href;
        };

        // 资源元素基类字段
        struct Resource {
            std::string src;
            std::optional<std::string> title;
        };

        struct Image : Resource {
            std::optional<int> width;
            std::optional<int> height;
        };

        struct Audio : Resource {
            std::optional<double> duration;
            std::optional<std::string> poster;
        };

        struct Video : Resource {
            std::optional<int> width;
            std::optional<int> height;
            std::optional<double> duration;
            std::optional<std::string> poster;
        };

        struct File : Resource {
            std::optional<std::string> poster;
        };

        struct Quote {
            std::optional<std::string> id;
        };

        struct Author {
            std::optional<std::string> id;
            std::optional<std::string> name;
            std::optional<std::string> avatar;
        };

        struct Button {
            std::optional<std::string> id;
            std::optional<std::string> type;
            std::optional<std::string> link;
            std::optional<std::string> text;
            std::optional<std::string> theme;
            std::string label; // 子文本内容
        };

        // 装饰性文本：b/i/u/s/code/spl/sup/sub
        struct Decorated {
            enum class Style { Bold, Italic, Underline, Strikethrough, Spoiler, Code, Sup, Sub };
            Style style;
            std::string text;
        };

        struct LineBreak {};

        struct Elements {
            std::string plainText;
            std::vector<At>    ats;
            std::vector<Image> images;
            std::vector<Audio> audios;
            std::vector<Video> videos;
            std::vector<File>  files;
            std::vector<Quote> quotes;
        };
        Elements parse(const std::string& content);

        class Builder {
        public:
            Builder();
            Builder& at(const std::string& id, const std::string& name = "");
            Builder& atAll();
            Builder& atHere();
            Builder& sharp(const std::string& id, const std::string& name = "");
            Builder& text(const std::string& content);
            Builder& img(const std::string& src);
            Builder& audio(const std::string& src);
            Builder& video(const std::string& src);
            Builder& file(const std::string& src, const std::string& title = "");
            Builder& br();
            Builder& bold(const std::string& content);
            Builder& italic(const std::string& content);
            Builder& code(const std::string& content);
            Builder& quote(const std::string& id);
            std::string build();

        private:
            tinyxml2::XMLDocument doc;
            tinyxml2::XMLElement* root;
        };
    }
}