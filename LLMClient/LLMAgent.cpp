#include "LLMAgent.h"

LLMAgent::LLMAgent(PrivateKey, LLMClient& llm, const std::string systemPrompt): llm(llm)
{
    history.push_back({ "system", systemPrompt });
}

LLMAgent::LLMAgent(PrivateKey, LLMClient& llm, const std::string systemPrompt, Tool& tool): llm(llm), tool(std::ref(tool))
{
    history.push_back({ "system", systemPrompt });
}


void LLMAgent::ask(const std::string& userInput, std::function<void(const std::string& reply)> cb)
{
    {
        std::lock_guard<std::mutex> lock(historyMtx);
        history.push_back({ "user", userInput });
    }
    doChat(std::move(cb));
}

void LLMAgent::clearHistory()
{
    std::lock_guard<std::mutex> lock(historyMtx);
    auto system = history.front();
    history.clear();
    history.push_back(system);
}

void LLMAgent::trimHistory()
{
    std::lock_guard<std::mutex> lock(historyMtx);
    while (history.size() > max_turns + 1)
    {
        auto it = std::find_if(history.begin() + 1, history.end(),
            [](const LLMClient::Message& m) { return m.role == "user"; });
        if (it == history.end()) break;

        auto next = std::find_if(it + 1, history.end(),
            [](const LLMClient::Message& m) { return m.role == "user"; });

        history.erase(it, next);
    }
}

void LLMAgent::doChat(std::function<void(const std::string& reply)> cb)
{
    nlohmann::json schema = (tool.has_value() ? tool->get().toSchema() : nlohmann::json::array());

    std::vector<LLMClient::Message> snapshot;
    {
        std::lock_guard<std::mutex> lock(historyMtx);
        snapshot = history;
    }

    llm.chat(snapshot,[self = shared_from_this(), cb](const LLMClient::ChatResult& result)
        {
            using FR = LLMClient::ChatResult::FinishReason;

            if (result.finish_reason == FR::Error)
            {
                cb("[LLM错误: " + result.error_message + "]");
                return;
            }

            if (result.finish_reason == FR::Stop)
            {
                std::string reply = result.content.value_or("");
                {
                    std::lock_guard<std::mutex> lock(self->historyMtx);
                    self->history.push_back({ "assistant", reply });
                }
                cb(reply);
                return;
            }

            // finish_reason == tool_calls
            // 1. 把 assistant 的 tool_calls message 追加进 history
            {
                std::lock_guard<std::mutex> lock(self->historyMtx);
                LLMClient::Message assistantMsg;
                assistantMsg.role       = "assistant";
                assistantMsg.content    = std::nullopt;
                assistantMsg.tool_calls = result.tool_calls;
                self->history.push_back(std::move(assistantMsg));
            }

            // 2. 并行执行所有工具，全部完成后继续循环
            self->executeToolCalls(result.tool_calls.value(), [self, cb]() 
            { 
                self->doChat(cb); 
            }
            );
        },
        schema
    );
}

void LLMAgent::executeToolCalls(const std::vector<LLMClient::ToolCall>& calls, std::function<void()> onAllDone)
{
    if (!tool.has_value() || calls.empty())
    {
        onAllDone();
        return;
    }

    int n = static_cast<int>(calls.size());

    // shared_ptr 计数器 + 结果槽，线程安全收集
    auto counter = std::make_shared<std::atomic<int>>(0);
    auto results = std::make_shared<std::vector<std::pair<std::string, std::string>>>(n);
    // results[i] = { tool_call_id, output }

    for (int i = 0; i < n; ++i)
    {
        Tool::Call call = Tool::fromToolCall(calls[i]);
        std::string callId = calls[i].id;

        tool->get().run(call,[self = shared_from_this(), i, n, call, callId, counter, results, onAllDone](const std::string& output)
            {
                if (self->toolHook.has_value()) (*self->toolHook)(call, output);
                (*results)[i] = { callId, output };

                // 最后一个完成的线程负责收尾
                if (counter->fetch_add(1) + 1 == n)
                {
                    // 将所有 tool result 追加进 history（保序）
                    {
                        std::lock_guard<std::mutex> lock(self->historyMtx);
                        for (auto& [id, out] : *results)
                        {
                            self->history.push_back({
                                "tool",
                                out,
                                std::nullopt,
                                id
                            });
                        }
                    }
                    onAllDone();
                }
            });
    }
}