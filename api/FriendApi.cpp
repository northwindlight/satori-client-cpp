#include "FriendApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

FriendApi::FriendApi(Bot* b) : bot(b) {}

satori::List<satori::event::Friend> FriendApi::list(const std::optional<std::string>& next)
{
    nlohmann::json body;
    if (next.has_value()) body["next"] = next.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/friend.list", body.dump());
        return nlohmann::json::parse(response).get<satori::List<satori::event::Friend>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("FriendApi::list error: ") + e.what());
    }
}

void FriendApi::delete_(const std::string& user_id)
{
    nlohmann::json body;
    body["user_id"] = user_id;
    try
    {
        bot->httpPost(bot->baseUrl() + "/friend.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("FriendApi::delete_ error: ") + e.what());
    }
}

void FriendApi::approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment)
{
    nlohmann::json body;
    body["message_id"] = message_id;
    body["approve"] = approve_;
    if (comment.has_value()) body["comment"] = comment.value();
    try
    {
        bot->httpPost(bot->baseUrl() + "/friend.approve", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("FriendApi::approve error: ") + e.what());
    }
}
