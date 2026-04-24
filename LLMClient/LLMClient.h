#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <vector>
#include <optional>
#include <ixwebsocket/IXHttpClient.h>
#include "../nlohmann/json.hpp"

class LLMClient
{
public:
    struct ToolCall 
    {
        std::string id;         // call_abc123
        std::string name;       // function name
        std::string arguments;  // JSON string，由模型生成
    };

    // ── 消息结构（兼容 system/user/assistant/tool）────────────
    struct Message 
    {
        std::string role;
        std::optional<std::string>            content;       // tool_calls 时可为 nullopt
        std::optional<std::vector<ToolCall>>  tool_calls;    // role=assistant 且调用工具时
        std::optional<std::string>            tool_call_id;  // role=tool 时关联 id
    };

    // ── 请求结果 ──────────────────────────────────────────────
    struct ChatResult 
    {
        enum class FinishReason { Stop, ToolCalls, Error };
        FinishReason                finish_reason;
        std::optional<std::string>  content;
        std::optional<std::vector<ToolCall>> tool_calls;
        std::string                 error_message;
    };

    LLMClient(const std::string& url, const std::string& api_key, const std::string& model);
    ~LLMClient();

    // tools 为空时不携带工具定义（兼容原来的用法）
    void chat( std::vector<Message> messages, std::function<void(const ChatResult&)> cb, const nlohmann::json& tools = nlohmann::json::array());

    void switchModel(const std::string& newModel);
    void stop();

private:
    struct Request {
        std::vector<Message>                    messages;
        std::function<void(const ChatResult&)>  callback;
        nlohmann::json                          tools;
    };

    ix::HttpClient           httpClient;
    ix::HttpRequestArgsPtr   args;
    ix::WebSocketHttpHeaders headers;
    std::string url, api_key, model;
    unsigned timeout = 300;

    std::unique_ptr<std::jthread> worker;
    std::mutex                    mtx;
    std::condition_variable       cv;
    std::queue<Request>           requestQueue;
    std::atomic<bool>             running = false;

    void workerThread();
};