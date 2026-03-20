#include "LLMClient.h"
#include "../nlohmann/json.hpp"
#include <iostream>
#include <mutex>

void from_json(const nlohmann::json& j, LLMClient::Message& m)
{
    j.at("role").get_to(m.role);
    j.at("content").get_to(m.content);
}

void to_json(nlohmann::json& j, const LLMClient::Message& m) {
    j = nlohmann::json{ {"role", m.role}, {"content", m.content} };
}

LLMClient::LLMClient(const std::string& url) : url(url)
{
    args = httpClient.createRequest();
    args->connectTimeout = 5;
    args->transferTimeout = timeout;
    headers["Content-Type"] = "application/json";
    args->extraHeaders = headers;
    running = true;
    worker = std::make_unique<std::jthread>(&LLMClient::workerThread, this);
}

LLMClient::LLMClient(const std::string& url, const std::string& api_key, const std::string& model) : url(url), api_key(api_key), model(model)
{
    args = httpClient.createRequest();
    args->connectTimeout = 5;
    args->transferTimeout = timeout;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + api_key;
    args->extraHeaders = headers;
    running = true;
    worker = std::make_unique<std::jthread>(&LLMClient::workerThread, this);
}

void LLMClient::sendLLM(const std::string& content, std::function<void(const std::string& response)> cb) {
    std::lock_guard<std::mutex> lock(mtx);
    requestQueue.push({content, std::move(cb)});
    cv.notify_one();
}

void LLMClient::setPrompt(const std::string& prompt)
{
    std::lock_guard<std::mutex> lock(mtx);
    this->prompt = prompt;
}

bool isToolCall(const std::string& str) {
    if (!nlohmann::json::accept(str)) return false;
    
    auto json = nlohmann::json::parse(str);
    return json.is_object() 
        && json.contains("tool") 
        && json.contains("args")
        && json["tool"].is_string()
        && json["args"].is_string();
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
        lock.unlock();

        nlohmann::json body;
        std::vector<Message> messages;
        messages.push_back({"system", prompt});
        messages.push_back({"user", req.content});
        body["messages"] = messages;
        if (!model.empty()) {
            body["model"] = model;
        }
        body["enable_thinking"] = false;
        body["stream"] = false;
        auto response = httpClient.post(url, body.dump(), args);
        if (response->errorCode != ix::HttpErrorCode::Ok && response->statusCode != 200)
        {
            std::cerr << "LLM HTTP POST 错误: " << response->statusCode << std::endl;
        }
        nlohmann::json jsonResponse = nlohmann::json::parse(response->body);
        std::string reply = jsonResponse["choices"][0]["message"]["content"].get<std::string>();
        if (isToolCall(reply)) {
            auto json = nlohmann::json::parse(reply);
            std::string tool = json["tool"].get<std::string>();
            std::string toolArgs = json["args"].get<std::string>();

            // 把 args 按空格拆开
            std::vector<std::string> cmd = {tool};
            std::istringstream iss(toolArgs);
            std::string token;
            while (iss >> token) {
                cmd.push_back(token);
            }

            useToolAsync(cmd, [this, req = std::move(req), userContent = req.content, assistantReply = reply](subprocess::CompletedProcess result) {
                std::string toolOutput = result.cout;

                nlohmann::json followBody;
                std::vector<Message> followMessages;
                followMessages.push_back({"system", prompt});
                followMessages.push_back({"user", userContent});
                // 把工具结果用 user 角色包一层喂回去
                followMessages.push_back({"user", "工具 [" + assistantReply + "] 执行结果：\n" + toolOutput});
                followBody["messages"] = followMessages;
                if (!model.empty()) followBody["model"] = model;
                followBody["enable_thinking"] = false;
                followBody["stream"] = false;

                auto followResponse = httpClient.post(url, followBody.dump(), args);
                if (followResponse->errorCode != ix::HttpErrorCode::Ok && followResponse->statusCode != 200) {
                    std::cerr << "LLM 二次请求错误: " << followResponse->statusCode << std::endl;
                    return;
                }

                nlohmann::json followJson = nlohmann::json::parse(followResponse->body);
                std::string finalReply = followJson["choices"][0]["message"]["content"].get<std::string>();
                req.callback(finalReply);
            });
        }
        else 
        {
            req.callback(reply);
        }
    }
}

void LLMClient::useToolAsync(std::vector<std::string> cmd, std::function<void(subprocess::CompletedProcess)> callback)
{
    std::thread([cmd = std::move(cmd), callback = std::move(callback)]() {
        auto result = subprocess::run(cmd, {
            .cout = subprocess::PipeOption::pipe,
            .cerr = subprocess::PipeOption::pipe
        });
        callback(result);
    }).detach();
}

std::string LLMClient::useTool(const std::string& tool, const std::string& args)
{
    auto result = subprocess::run({tool, args}, {
        .cout = subprocess::PipeOption::pipe,
        .check = true
    });
    return result.cout;
}