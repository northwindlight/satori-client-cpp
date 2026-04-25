#include "ChannelApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

ChannelApi::ChannelApi(Bot* b) : bot(b) {}

satori::event::Channel ChannelApi::get(const std::string& channel_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/channel.get", body.dump());
        return nlohmann::json::parse(response).get<satori::event::Channel>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::get error: ") + e.what());
    }
}

satori::List<satori::event::Channel> ChannelApi::list(const std::string& guild_id, const std::optional<std::string>& next)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/channel.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::Channel>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::list error: ") + e.what());
    }
}

satori::event::Channel ChannelApi::create(const std::string& guild_id, const satori::event::Channel& data)
{
    nlohmann::json body;
    body["guild_id"] = guild_id;
    body["data"] = data;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/channel.create", body.dump());
        return nlohmann::json::parse(response).get<satori::event::Channel>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::create error: ") + e.what());
    }
}

void ChannelApi::update(const std::string& channel_id, const satori::event::Channel& data)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["data"] = data;
    try
    {
        bot->httpPost(bot->baseUrl() + "/channel.update", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::update error: ") + e.what());
    }
}

void ChannelApi::delete_(const std::string& channel_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/channel.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::delete_ error: ") + e.what());
    }
}

void ChannelApi::mute(const std::string& channel_id, int duration)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["duration"] = duration;
    try
    {
        bot->httpPost(bot->baseUrl() + "/channel.mute", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ChannelApi::mute error: ") + e.what());
    }
}
