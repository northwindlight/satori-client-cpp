#include "UserApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

UserApi::UserApi(Bot* b) : bot(b), channel(b) {}

UserApi::ChannelApi::ChannelApi(Bot* b) : bot(b) {}

satori::event::User UserApi::get(const std::string& user_id)
{
    nlohmann::json body;
    body["user_id"] = user_id;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/user.get", body.dump());
        return nlohmann::json::parse(response).get<satori::event::User>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("UserApi::get error: ") + e.what());
    }
}

satori::event::Channel UserApi::ChannelApi::create(const std::string& user_id, const std::optional<std::string>& guild_id)
{
    nlohmann::json body;
    body["user_id"] = user_id;
    if (guild_id.has_value()) body["guild_id"] = guild_id.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/user.channel.create", body.dump());
        return nlohmann::json::parse(response).get<satori::event::Channel>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("UserApi::ChannelApi::create error: ") + e.what());
    }
}
