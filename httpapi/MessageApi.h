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
    //delete是C++的关键字，所以这里命名为delete_
    void delete_(const std::string& channel_id, const std::string& message_id); 
    void setPostProcess(std::function<std::string(const std::string&)> fn);
};