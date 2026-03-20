#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <ixwebsocket/IXHttpClient.h>
#include "../subprocess.hpp"

class LLMClient
{
private:
    ix::HttpClient httpClient;
    ix::HttpRequestArgsPtr args;
    ix::WebSocketHttpHeaders headers;
    std::string url;
    std::string prompt;
    std::string api_key;
    std::string model;
    std::unique_ptr<std::jthread> worker = nullptr;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> tools;
    bool running = false;
    unsigned timeout = 300;
    void workerThread();
    struct Request {
        std::string content;
        std::function<void(const std::string& response)> callback;
    };
    std::queue<Request> requestQueue;
public:
    LLMClient(const std::string& url);
    LLMClient(const std::string& url, const std::string& api_key, const std::string& model);
    struct Message {
        std::string role;
        std::string content;
    };
    void setPrompt(const std::string& prompt);
    void sendLLM(const std::string& content, std::function<void(const std::string& response)> cb);
    void stop();
    void setCallback(std::function<void(const std::string& response)> cb);
    void setTools(const std::vector<std::string>& tools);
    std::string useTool(const std::string& tool, const std::string& args);
    void useToolAsync(std::vector<std::string> cmd, std::function<void(subprocess::CompletedProcess)> callback);
};
