#pragma once
#include <string>
#include <optional>
#include "../Satori/Satori.h"

class Bot;

class GuildApi
{
private:
    Bot* bot;

public:
    GuildApi(Bot* b);
    satori::event::Guild get(const std::string& guild_id);
    satori::List<satori::event::Guild> list(const std::optional<std::string>& next = std::nullopt);
    void approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment = std::nullopt);

    class MemberApi {
    private:
        Bot* bot;
    public:
        MemberApi(Bot* b);
        satori::event::GuildMember get(const std::string& guild_id, const std::string& user_id);
        satori::List<satori::event::GuildMember> list(const std::string& guild_id, const std::optional<std::string>& next = std::nullopt);
        void kick(const std::string& guild_id, const std::string& user_id, const std::optional<bool>& permanent = std::nullopt);
        void mute(const std::string& guild_id, const std::string& user_id, int duration);
        void approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment = std::nullopt);

        class RoleApi {
        private:
            Bot* bot;
        public:
            RoleApi(Bot* b);
            void set(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
            void unset(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
        } role;
    } member;

    class RoleApi {
    private:
        Bot* bot;
    public:
        RoleApi(Bot* b);
        satori::List<satori::event::GuildRole> list(const std::string& guild_id, const std::optional<std::string>& next = std::nullopt);
        satori::event::GuildRole create(const std::string& guild_id, const satori::event::GuildRole& role);
        void update(const std::string& guild_id, const std::string& role_id, const satori::event::GuildRole& role);
        void delete_(const std::string& guild_id, const std::string& role_id);
    } role;
};
