#pragma once

#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXWebSocket.h>
#include "httpapi/MessageApi.h"
#include "Satori/Satori.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <optional>
#include <atomic>


class Bot
{
private:
    ix::HttpClient httpClient;
    ix::HttpRequestArgsPtr args;
    ix::WebSocket webSocket;
    ix::WebSocketHttpHeaders headers;
    std::string baseAddr;
    std::string token;
    std::string userID;
    std::string platform;
    std::unordered_map<ix::WebSocketMessageType, std::vector<std::function<void(const ix::WebSocketMessagePtr&)>>> callbacksByType;
    std::atomic<bool> running = false;
    int sn = 0;
    void init();
    void pingLoop();
    void exceptionHandling();
public:
    enum Opcode
    {
        EVENT,
        PING,
        PONG,
        IDENTIFY,
        READY
    };
    Bot(const std::string& baseAddr, const std::string& token, const std::string& platform, const std::string& userID);
    void addWSCallback(ix::WebSocketMessageType, std::function<void(const ix::WebSocketMessagePtr&)> callback);
    void addOnMessageCallback(std::function<void(const satori::event::Event&)> callback);
    std::string getHttpAddr() const { return "http://" + baseAddr + "/v1"; }
    std::optional<std::string> httpGet(const std::string& url);
    std::optional<std::string> httpPost(const std::string& url, const std::string& body);
    void identify(const std::string& token);
    void launch();
    void launchAsync();
    bool wsSend(const std::string& message);
    MessageApi message{this};
    std::string getUserID();
    ~Bot();
};