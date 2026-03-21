#pragma once

#include <string>
#include <functional>
#include <vector>
#include "LLMClient/LLMClient.h"

class LLMAgent
{
private:
    LLMClient& llm;
    std::string systemPrompt;
    std::vector<std::string> registryTools;
    struct ToolCall {
        std::string tool;
        std::string args;
    };

    bool parseToolCall(const std::string& reply, ToolCall& out);
    void runToolAsync(const ToolCall& tc, std::function<void(const std::string& output)> cb);

    std::vector<LLMClient::Message> buildMessages(
        const std::string& userInput,
        const std::string& assistantReply = "",
        const std::string& toolOutput = "");

public:
    LLMAgent(LLMClient& llm, const std::string& systemPrompt);
    void toolRegistry(std::vector<std::string> tools);
    void ask(const std::string& userInput, std::function<void(const std::string& reply)> cb);
};
