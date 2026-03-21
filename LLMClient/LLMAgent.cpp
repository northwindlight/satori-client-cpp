#include "LLMAgent.h"
#include "LLMClient/LLMClient.h"
#include "subprocess.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>


LLMAgent::LLMAgent(LLMClient& llm, const std::string& systemPrompt) : llm(llm), systemPrompt(systemPrompt) {}

void LLMAgent::ask(const std::string& userInput, std::function<void(const std::string& reply)> cb) 
{
    auto messages = buildMessages(userInput);
    llm.chat(std::move(messages), [this, userInput, cb](const std::string& reply) 
    {
        ToolCall tc;
        if (parseToolCall(reply, tc)) 
        {
            runToolAsync(tc, [this, userInput, assistantReply = reply, cb](const std::string& toolOutput) 
            {
                // 把工具结果拼回上下文，二次请求
                auto messages2 = buildMessages(userInput, assistantReply, toolOutput);
                llm.chat(std::move(messages2), [cb](const std::string& finalReply) 
                {
                    cb(finalReply);
                });
            });
        } 
        else 
        {
            cb(reply);
        }
    });
}

bool LLMAgent::parseToolCall(const std::string& reply, ToolCall& out) 
{
    if (!nlohmann::json::accept(reply)) return false;
    auto json = nlohmann::json::parse(reply);
    if (!json.is_object()
        || !json.contains("tool") || !json["tool"].is_string()
        || !json.contains("args") || !json["args"].is_string())
        return false;
    out.tool = json["tool"].get<std::string>();
    out.args = json["args"].get<std::string>();
    return true;
}

void LLMAgent::runToolAsync(const ToolCall& tc, std::function<void(const std::string&)> cb) {
    // 把 args 按空格拆成 argv
    auto it = std::ranges::find(registryTools, tc.tool);
    if (it == registryTools.end()) 
    {
        cb("[该工具不在白名单]");
        return;
    }
    std::vector<std::string> cmd = {tc.tool};
    std::istringstream iss(tc.args);
    std::string token;
    while (iss >> token) cmd.push_back(token);
    std::thread([cmd = std::move(cmd), cb = std::move(cb)]() 
    {
        try {
            auto result = subprocess::run(cmd, {
                .cout = subprocess::PipeOption::pipe,
                .cerr = subprocess::PipeOption::pipe
            });
            cb(result.cout);
        } catch (const std::exception& e) {
            std::cerr << "工具执行错误: " << e.what() << std::endl;
            cb("[工具执行失败]");
        }
    }).detach();
    
}

std::vector<LLMClient::Message> LLMAgent::buildMessages(
    const std::string& userInput,
    const std::string& assistantReply,
    const std::string& toolOutput)
{
    std::vector<LLMClient::Message> messages;
    messages.push_back({"system", systemPrompt});
    messages.push_back({"user", userInput});
    if (!assistantReply.empty()) 
    {
        messages.push_back({"assistant", assistantReply});
        messages.push_back({"user", "工具执行结果：\n" + toolOutput});
    }
    return messages;
}

void LLMAgent::toolRegistry(std::vector<std::string> tools)
{
    registryTools.insert(registryTools.end(), tools.begin(), tools.end());
}