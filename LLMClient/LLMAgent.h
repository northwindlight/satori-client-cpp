#pragma once

#include <optional>
#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include "LLMClient.h"
#include "tool.hpp"

class LLMAgent : public std::enable_shared_from_this<LLMAgent>
{
    struct PrivateKey { PrivateKey() = default; };

public:
    LLMAgent(PrivateKey, LLMClient& llm, const std::string& systemPrompt);
    LLMAgent(PrivateKey, LLMClient& llm, const std::string& systemPrompt, Tool& tool);

    void ask(const std::string& userInput, std::function<void(const std::string& reply)> cb);

    void clearHistory();

    static std::shared_ptr<LLMAgent> create( LLMClient& llm, const std::string& systemPrompt)
    {
        return std::make_shared<LLMAgent>(PrivateKey{}, llm, systemPrompt);
    }

    static std::shared_ptr<LLMAgent> create( LLMClient& llm, const std::string& systemPrompt, Tool& tool)
    {
        return std::make_shared<LLMAgent>(PrivateKey{}, llm, systemPrompt, tool);
    }


private:
    // 内部循环：发请求 → 处理 tool_calls → 再发请求 → 直到 stop
    void doChat(std::function<void(const std::string& reply)> cb);

    // 并行执行本轮所有 tool_calls，全部完成后继续循环
    void executeToolCalls( const std::vector<LLMClient::ToolCall>& calls, std::function<void()> onAllDone);
    void trimHistory();
    LLMClient& llm;
    std::optional<std::reference_wrapper<Tool>> tool;   // 不持有所有权
    std::vector<LLMClient::Message> history;
    std::mutex historyMtx;
    const size_t max_turns = 50;
};