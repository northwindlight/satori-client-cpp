#include "ReactionApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

ReactionApi::ReactionApi(Bot* b) : bot(b) {}

void ReactionApi::create(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    body["emoji_id"] = emoji_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/reaction.create", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ReactionApi::create error: ") + e.what());
    }
}

void ReactionApi::delete_(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id, const std::optional<std::string>& user_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    body["emoji_id"] = emoji_id;
    if (user_id.has_value()) body["user_id"] = user_id.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/reaction.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ReactionApi::delete_ error: ") + e.what());
    }
}

void ReactionApi::clear(const std::string& channel_id, const std::string& message_id, const std::optional<std::string>& emoji_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    if (emoji_id.has_value()) body["emoji_id"] = emoji_id.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/reaction.clear", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ReactionApi::clear error: ") + e.what());
    }
}

satori::List<satori::event::User> ReactionApi::list(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id, const std::optional<std::string>& next)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    body["emoji_id"] = emoji_id;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/reaction.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::User>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("ReactionApi::list error: ") + e.what());
    }
}
