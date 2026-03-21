#include "LLMClient.h"
#include "../nlohmann/json.hpp"
#include <iostream>

void from_json(const nlohmann::json& j, LLMClient::Message& m) 
{
    j.at("role").get_to(m.role);
    j.at("content").get_to(m.content);
}

void to_json(nlohmann::json& j, const LLMClient::Message& m) 
{
    j = nlohmann::json{{"role", m.role}, {"content", m.content}};
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

LLMClient::~LLMClient() 
{
    stop();
}

void LLMClient::stop() 
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
    }
    cv.notify_all();
}

void LLMClient::chat(std::vector<Message> messages, std::function<void(const std::string&)> cb) 
{
    std::lock_guard<std::mutex> lock(mtx);
    requestQueue.push({std::move(messages), std::move(cb)});
    cv.notify_one();
}

void LLMClient::workerThread() {
    while (running) 
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !requestQueue.empty() || !running; });
        if (!running) break;

        Request req = std::move(requestQueue.front());
        requestQueue.pop();
        lock.unlock();

        nlohmann::json body;
        body["messages"] = req.messages;
        if (!model.empty()) body["model"] = model;
        body["enable_thinking"] = false;
        body["stream"] = false;

        auto response = httpClient.post(url, body.dump(), args);
        if (response->errorCode != ix::HttpErrorCode::Ok || response->statusCode != 200) 
        {
            std::cerr << "LLM HTTP POST 错误: " << response->statusCode << std::endl;
            continue;
        }

        try 
        {
            nlohmann::json jsonResponse = nlohmann::json::parse(response->body);
            std::string reply = jsonResponse["choices"][0]["message"]["content"].get<std::string>();
            req.callback(reply);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "LLM 响应解析错误: " << e.what() << std::endl;
            std::cerr << response->body << std::endl;
        }
    }
}
