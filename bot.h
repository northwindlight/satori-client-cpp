#pragma once

#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXWebSocket.h>
#include "httpapi/MessageApi.h"
#include "LLMClient/LLMClient.h"
#include "Satori/Satori.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <optional>


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
    std::unordered_map<ix::WebSocketMessageType, std::vector<std::function<void(const ix::WebSocketMessagePtr&)>>> callbacksByType;

    bool running = false;
    int sn = 0;
    void run();
    void pingLoop();
    void exceptionHandling();

    void init();
public:
    enum Opcode
    {
        EVENT,
        PING,
        PONG,
        IDENTIFY,
        READY
    };
    Bot(const std::string& baseAddr, const std::string& token, const std::string& userID);
    Bot(const std::string& baseAddr, const std::string& token, const std::string& userID, LLMClient& client);
    //~Bot();
    LLMClient* client = nullptr;
    void addWSCallback(ix::WebSocketMessageType, std::function<void(const ix::WebSocketMessagePtr&)> callback);
    void addOnMessageCallback(std::function<void(const satori::Event&)> callback);
    std::string getHttpAddr() const { return "http://" + baseAddr + "/v1"; }
    std::optional<std::string> httpGet(const std::string& url);
    std::optional<std::string> httpPost(const std::string& url, const std::string& body);
    void setBaseAddr(const std::string& addr) { baseAddr = addr; }
    void setUserID(const std::string& id) { userID = id; }
    void identify(const std::string& token);
    void start();
    void wsSend(const std::string& message);

    MessageApi message{this};
};