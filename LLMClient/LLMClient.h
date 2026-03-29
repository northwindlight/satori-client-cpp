#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <vector>
#include <ixwebsocket/IXHttpClient.h>

class LLMClient
{
public:
    struct Message {
        std::string role;
        std::string content;
    };

    LLMClient(const std::string& url);
    LLMClient(const std::string& url, const std::string& api_key, const std::string& model);
    ~LLMClient();

    void chat(std::vector<Message> messages,
              std::function<void(const std::string& reply)> cb);

    void stop();

private:
    struct Request {
        std::vector<Message> messages;
        std::function<void(const std::string&)> callback;
    };

    ix::HttpClient httpClient;
    ix::HttpRequestArgsPtr args;
    ix::WebSocketHttpHeaders headers;
    std::string url;
    std::string api_key;
    std::string model;
    unsigned timeout = 300;

    std::unique_ptr<std::jthread> worker;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Request> requestQueue;
    std::atomic<bool> running = false;

    void workerThread();
};
