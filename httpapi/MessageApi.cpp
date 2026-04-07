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
    std::optional<std::string> response = bot->httpPost(bot->getHttpAddr() + "/message.create", body.dump());
    if (response.has_value())
    {
        //std::cout << "HTTP POST 响应: " << response.value() << std::endl;
        nlohmann::json jsonResponse = nlohmann::json::parse(response.value());
        std::vector<satori::event::Message> messages = jsonResponse.get<std::vector<satori::event::Message>>();
        return messages;
    }
    else 
    {
        std::cerr << "HTTP POST 请求失败，无法创建消息" << std::endl;
        return {};
    }
}

void MessageApi::setPostProcess(std::function<std::string(const std::string&)> fn)
{
    //用于后处理消息内容，例如屏蔽关键词等，默认为直接返回内容不变
    postProcess = std::move(fn);
}

