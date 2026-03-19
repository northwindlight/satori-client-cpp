#pragma once
#include <vector>
#include <string>
#include "../Satori/Satori.h"

class Bot;
class MessageApi
{
private:
    Bot* bot;
    std::function<std::string(const std::string&)> postProcess = nullptr;
public:
    MessageApi(Bot* b);
    std::vector<satori::Message> create(const std::string& channel_id, const std::string& content);
    void setPostProcess(std::function<std::string(const std::string&)> fn);
    void asyncLLM(const std::string& channel_id, const std::string& content);
};
