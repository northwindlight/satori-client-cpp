#include "LLMAgent.h"
#include <iostream>

 
LLMAgent::LLMAgent(PrivateKey, LLMClient& llm, const std::string& systemPrompt) : llm(llm)
{
    history.push_back({"system", systemPrompt});
}
 
void LLMAgent::ask(const std::string& userInput, std::function<void(const std::string& reply)> cb)
{
    {
        std::lock_guard<std::mutex> lock(historyMtx);
        history.push_back({"user", userInput});
    }
    
    llm.chat(history, [self = shared_from_this(), cb](const std::string& reply)
    {
        {
            std::lock_guard<std::mutex> lock(self->historyMtx);
            self->history.push_back({"assistant", reply});
        }
        try 
        {
            cb(reply);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("LLMAgent::ask 回调错误: ") + e.what());
        }
        
    });
}
 
void LLMAgent::clearHistory()
{
    std::lock_guard<std::mutex> lock(historyMtx);
    auto system = history.front();
    history.clear();
    history.push_back(system);
}