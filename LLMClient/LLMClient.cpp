#include "LLMClient.h"
#include <iostream>

static void to_json(nlohmann::json& j, const LLMClient::ToolCall& tc)
{
    j = {
        {"id",   tc.id},
        {"type", "function"},
        {"function", {{"name", tc.name}, {"arguments", tc.arguments}}}
    };
}

static void to_json(nlohmann::json& j, const LLMClient::Message& m)
{
    j = nlohmann::json::object();
    j["role"] = m.role;

    if (m.content.has_value()) j["content"] = m.content.value();
    else j["content"] = nullptr;

    if (m.tool_calls.has_value())
    {
        nlohmann::json arr = nlohmann::json::array();
        for (auto& tc : m.tool_calls.value())
        {
            nlohmann::json item;
            to_json(item, tc);
            arr.push_back(item);
        }
        j["tool_calls"] = arr;
    }

    if (m.tool_call_id.has_value())
        j["tool_call_id"] = m.tool_call_id.value();
}

LLMClient::LLMClient(const std::string& url, const std::string& api_key, const std::string& model): url(url), api_key(api_key), model(model)
{
    args = httpClient.createRequest();
    args->connectTimeout  = 5;
    args->transferTimeout = timeout;
    headers["Content-Type"]  = "application/json";
    headers["Authorization"] = "Bearer " + api_key;
    args->extraHeaders = headers;
    running = true;
    worker = std::make_unique<std::jthread>(&LLMClient::workerThread, this);
}

LLMClient::~LLMClient() 
{
    stop(); 
}


void LLMClient::stop()
{
    running = false;
    cv.notify_all();
}

void LLMClient::chat(std::vector<Message> messages, std::function<void(const ChatResult&)> cb, const nlohmann::json& tools)
{
    std::lock_guard<std::mutex> lock(mtx);
    requestQueue.push({std::move(messages), std::move(cb), tools});
    cv.notify_one();
}

void LLMClient::switchModel(const std::string& newModel)
{
    std::lock_guard<std::mutex> lock(mtx);
    model = newModel;
}


void LLMClient::workerThread()
{
    while (running)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !requestQueue.empty() || !running; });
        if (!running) break;

        Request req = std::move(requestQueue.front());
        requestQueue.pop();

        // 构建请求体
        nlohmann::json body;
        body["messages"] = nlohmann::json::array();
        body["enable_thinking"] = false;
        body["stream"] = false;
        body["model"] = model;

        for (auto& msg : req.messages)
        {
            nlohmann::json jmsg;
            to_json(jmsg, msg);
            body["messages"].push_back(jmsg);
        }

        if (!req.tools.empty())
        {
            body["tools"]       = req.tools;
            body["tool_choice"] = "auto";
        }

        lock.unlock();

        auto response = httpClient.post(url, body.dump(), args);

        if (response->errorCode != ix::HttpErrorCode::Ok || response->statusCode != 200)
        {
            std::cerr << "LLM HTTP POST 错误: " << response->statusCode << std::endl;
            req.callback(ChatResult{
                ChatResult::FinishReason::Error, std::nullopt, std::nullopt,
                "HTTP错误: " + std::to_string(response->statusCode)
            });
            continue;
        }

        try
        {
            auto j       = nlohmann::json::parse(response->body);
            auto& choice = j["choices"][0];
            std::string finish = choice["finish_reason"].get<std::string>();
            auto& message      = choice["message"];

            if (finish == "tool_calls")
            {
                std::vector<LLMClient::ToolCall> calls;
                for (auto& tc : message["tool_calls"])
                {
                    calls.push_back({
                        tc["id"].get<std::string>(),
                        tc["function"]["name"].get<std::string>(),
                        tc["function"]["arguments"].get<std::string>()
                    });
                }
                req.callback(ChatResult{
                    ChatResult::FinishReason::ToolCalls,
                    std::nullopt, calls, ""
                });
            }
            else // stop
            {
                req.callback(ChatResult{
                    ChatResult::FinishReason::Stop,
                    message["content"].get<std::string>(),
                    std::nullopt, ""
                });
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("LLM 响应解析错误: ") + e.what());
        }
    }
}