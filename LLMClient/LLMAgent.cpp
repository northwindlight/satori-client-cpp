#include "LLMAgent.h"

 
LLMAgent::LLMAgent(LLMClient& llm, const std::string& systemPrompt) : llm(llm)
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
        cb(reply);
    });
}
 
void LLMAgent::clearHistory()
{
    auto system = history.front();
    history.clear();
    history.push_back(system);
}