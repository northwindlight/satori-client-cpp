#include "MessageApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"


MessageApi::MessageApi(Bot* b) : bot(b) {}
std::vector<satori::Message> MessageApi::create(const std::string& channel_id, const std::string& content)
{
    nlohmann::json body;
    body["channel_id"] = channel_id;
    body["content"] = content;
    std::optional<std::string> response = bot->httpPost(bot->getHttpAddr() + "/message.create", body.dump());
    if (response.has_value())
    {
        //std::cout << "HTTP POST 响应: " << response.value() << std::endl;
        nlohmann::json jsonResponse = nlohmann::json::parse(response.value());
        std::vector<satori::Message> messages = jsonResponse.get<std::vector<satori::Message>>();
        return messages;
    }
    return {};

}

void MessageApi::setPostProcess(std::function<std::string(const std::string&)> fn)
{
    postProcess = std::move(fn);
}

