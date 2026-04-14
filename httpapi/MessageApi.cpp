#include "MessageApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"
#include <iostream>


MessageApi::MessageApi(Bot* b) : bot(b) {}
std::vector<satori::event::Message> MessageApi::create(const std::string& channel_id, const std::string& content)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["content"] = content;
    //std::cout << body.dump() << std::endl;
    try 
    {
        std::string response = bot->httpPost(bot->getHttpAddr() + "/message.create", body.dump());
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
        std::string response = bot->httpPost(bot->getHttpAddr() + "/message.get", body.dump());
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
        bot->httpPost(bot->getHttpAddr() + "/message.delete", body.dump());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("MessageApi::delete_ 错误: ") + e.what());
    }
}

void MessageApi::setPostProcess(std::function<std::string(const std::string&)> fn)
{
    //用于后处理消息内容，例如屏蔽关键词等，默认为直接返回内容不变
    postProcess = std::move(fn);
}

