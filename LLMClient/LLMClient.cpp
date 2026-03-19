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
        if (response->errorCode != ix::HttpErrorCode::Ok)
        {
            std::cerr << "LLM HTTP POST 错误: " << response->statusCode << std::endl;
        }
        std::cout << "Status: " << response->statusCode << std::endl;
        std::cout << "Error code: " << (int)response->errorCode << std::endl;
        std::cout << "Error msg: " << response->errorMsg << std::endl;
        std::cout << "Body length: " << response->body.size() << std::endl;
        std::cout << "Body: " << response->body << std::endl;
        nlohmann::json jsonResponse = nlohmann::json::parse(response->body);
        req.callback(jsonResponse["choices"][0]["message"]["content"].get<std::string>());
    }
}
