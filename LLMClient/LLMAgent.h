#pragma once
 
#include <string>
#include <functional>
#include <vector>
#include "LLMClient.h"
#include <memory>
 
class LLMAgent : public std::enable_shared_from_this<LLMAgent>
{
    struct PrivateKey { PrivateKey() = default; };
private:
    LLMClient& llm;
    std::vector<LLMClient::Message> history;
    std::mutex historyMtx;
 
public:
    LLMAgent(PrivateKey, LLMClient& llm, const std::string& systemPrompt);
 
    void ask(const std::string& userInput, std::function<void(const std::string& reply)> cb);
    static std::shared_ptr<LLMAgent> create(LLMClient& llm, const std::string& systemPrompt) { return std::make_shared<LLMAgent>(PrivateKey{}, llm, systemPrompt); }
    void clearHistory();
};
