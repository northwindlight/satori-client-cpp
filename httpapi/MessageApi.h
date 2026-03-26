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
    void setPostProcess(std::function<std::string(const std::string&)> fn);

};