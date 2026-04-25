#include "GuildApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

// ── GuildApi ─────────────────────────────────────────────

GuildApi::GuildApi(Bot* b) : bot(b), member(b), role(b) {}

satori::event::Guild GuildApi::get(const std::string& guild_id)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.get", body.dump());
        return nlohmann::json::parse(response).get<satori::event::Guild>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::get error: ") + e.what());
    }
}

satori::List<satori::event::Guild> GuildApi::list(const std::optional<std::string>& next)
{
    nlohmann::json body;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::Guild>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::list error: ") + e.what());
    }
}

void GuildApi::approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment)
{
    nlohmann::json body;
    body["message_id"] = message_id;
    body["approve"] = approve_;
    if (comment.has_value()) body["comment"] = comment.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.approve", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::approve error: ") + e.what());
    }
}

// ── GuildApi::MemberApi ──────────────────────────────────

GuildApi::MemberApi::MemberApi(Bot* b) : bot(b), role(b) {}

satori::event::GuildMember GuildApi::MemberApi::get(const std::string& guild_id, const std::string& user_id)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["user_id"] = user_id;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.member.get", body.dump());
        return nlohmann::json::parse(response).get<satori::event::GuildMember>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::get error: ") + e.what());
    }
}

satori::List<satori::event::GuildMember> GuildApi::MemberApi::list(const std::string& guild_id, const std::optional<std::string>& next)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.member.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::GuildMember>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::list error: ") + e.what());
    }
}

void GuildApi::MemberApi::kick(const std::string& guild_id, const std::string& user_id, const std::optional<bool>& permanent)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["user_id"] = user_id;
    if (permanent.has_value()) body["permanent"] = permanent.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.member.kick", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::kick error: ") + e.what());
    }
}

void GuildApi::MemberApi::mute(const std::string& guild_id, const std::string& user_id, int duration)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["user_id"] = user_id;
    body["duration"] = duration;
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.member.mute", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::mute error: ") + e.what());
    }
}

void GuildApi::MemberApi::approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment)
{
    nlohmann::json body;
    body["message_id"] = message_id;
    body["approve"] = approve_;
    if (comment.has_value()) body["comment"] = comment.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.member.approve", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::approve error: ") + e.what());
    }
}

// ── GuildApi::MemberApi::RoleApi ─────────────────────────

GuildApi::MemberApi::RoleApi::RoleApi(Bot* b) : bot(b) {}

void GuildApi::MemberApi::RoleApi::set(const std::string& guild_id, const std::string& user_id, const std::string& role_id)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["user_id"] = user_id;
    body["role_id"] = role_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.member.role.set", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::RoleApi::set error: ") + e.what());
    }
}

void GuildApi::MemberApi::RoleApi::unset(const std::string& guild_id, const std::string& user_id, const std::string& role_id)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["user_id"] = user_id;
    body["role_id"] = role_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.member.role.unset", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::MemberApi::RoleApi::unset error: ") + e.what());
    }
}

// ── GuildApi::RoleApi ────────────────────────────────────

GuildApi::RoleApi::RoleApi(Bot* b) : bot(b) {}

satori::List<satori::event::GuildRole> GuildApi::RoleApi::list(const std::string& guild_id, const std::optional<std::string>& next)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.role.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::GuildRole>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::RoleApi::list error: ") + e.what());
    }
}

satori::event::GuildRole GuildApi::RoleApi::create(const std::string& guild_id, const satori::event::GuildRole& role)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["role"] = role;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/guild.role.create", body.dump());
        return nlohmann::json::parse(response).get<satori::event::GuildRole>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::RoleApi::create error: ") + e.what());
    }
}

void GuildApi::RoleApi::update(const std::string& guild_id, const std::string& role_id, const satori::event::GuildRole& role)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["role_id"] = role_id;
    body["role"] = role;
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.role.update", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::RoleApi::update error: ") + e.what());
    }
}

void GuildApi::RoleApi::delete_(const std::string& guild_id, const std::string& role_id)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["role_id"] = role_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/guild.role.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("GuildApi::RoleApi::delete_ error: ") + e.what());
    }
}
