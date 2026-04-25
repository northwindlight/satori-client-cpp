#pragma once
#include <string>
#include <vector>
#include <functional>
#include "../Satori/Satori.h"

class Bot;

class MessageApi
{
private:
    Bot* bot;
    std::function<std::string(const std::string&)> postProcess;

public:
    MessageApi(Bot* b);
    std::vector<satori::event::Message> create(const std::string& channel_id, const std::string& content);
    satori::event::Message get(const std::string& channel_id, const std::string& message_id);
    void delete_(const std::string& channel_id, const std::string& message_id);
    satori::event::Message update(const std::string& channel_id, const std::string& message_id, const std::string& content);
    satori::BidiList<satori::event::Message> list(const std::string& channel_id, const std::optional<std::string>& next = std::nullopt, const std::optional<std::string>& direction = std::nullopt, const std::optional<int>& limit = std::nullopt, const std::optional<std::string>& order = std::nullopt);
    void setPostProcess(std::function<std::string(const std::string&)> fn);
};