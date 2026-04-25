#include "MessageApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"


MessageApi::MessageApi(Bot* b) : bot(b) {}
std::vector<satori::event::Message> MessageApi::create(const std::string& channel_id, const std::string& content)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["content"] = content;
    //std::cout << body.dump() << std::endl;
    try 
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/message.create", body.dump());
        //std::cout << "HTTP POST 响应: " << response.value() << std::endl;
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        std::vector<satori::event::Message> messages = jsonResponse.get<std::vector<satori::event::Message>>();
        return messages;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::create 错误: ") + e.what());
    }
}

satori::event::Message MessageApi::get(const std::string& channel_id, const std::string& message_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    //std::cout << body.dump() << std::endl;
    try 
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/message.get", body.dump());
        //std::cout << "HTTP POST 响应: " << response.value() << std::endl;
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        satori::event::Message message = jsonResponse.get<satori::event::Message>();
        return message;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::get 错误: ") + e.what());
    }
}

void MessageApi::delete_(const std::string& channel_id, const std::string& message_id)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    try 
    {
        bot->httpPost(bot->baseUrl() + "/message.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::delete_ 错误: ") + e.what());
    }
}

satori::event::Message MessageApi::update(const std::string& channel_id, const std::string& message_id, const std::string& content)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["message_id"] = message_id;
    body["content"] = content;
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/message.update", body.dump());
        return nlohmann::json::parse(response).get<satori::event::Message>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::update error: ") + e.what());
    }
}

satori::BidiList<satori::event::Message> MessageApi::list(const std::string& channel_id, const std::optional<std::string>& next, const std::optional<std::string>& direction, const std::optional<int>& limit, const std::optional<std::string>& order)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    if (next.has_value()) body["next"] = next.value();
    if (direction.has_value()) body["direction"] = direction.value();
    if (limit.has_value()) body["limit"] = limit.value();
    if (order.has_value()) body["order"] = order.value();
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/message.list", body.dump());
        return nlohmann::json::parse(response).get<satori::BidiList<satori::event::Message>>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::list error: ") + e.what());
    }
}

void MessageApi::setPostProcess(std::function<std::string(const std::string&)> fn)
{
    postProcess = std::move(fn);
}

