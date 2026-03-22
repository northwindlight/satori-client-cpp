#pragma once
 
#include <string>
#include <functional>
#include <vector>
#include "LLMClient/LLMClient.h"
 
class LLMAgent
{
private:
    LLMClient& llm;
    std::vector<LLMClient::Message> history;
 
public:
    LLMAgent(LLMClient& llm, const std::string& systemPrompt);
 
    void ask(const std::string& userInput, std::function<void(const std::string& reply)> cb);
 
    void clearHistory();
};
 