#pragma once

#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXWebSocket.h>
#include "api/MessageApi.h"
#include "api/ChannelApi.h"
#include "api/UserApi.h"
#include "api/GuildApi.h"
#include "api/FriendApi.h"
#include "api/LoginApi.h"
#include "api/ReactionApi.h"
#include "Satori/Satori.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <atomic>

class Bot
{
private:
    ix::HttpClient httpClient_;
    ix::HttpRequestArgsPtr args_;
    ix::WebSocket webSocket_;
    ix::WebSocketHttpHeaders headers_;
    std::string baseAddr_;
    std::string token_;
    std::string userId_;
    std::string platform_;
    std::unordered_map<ix::WebSocketMessageType, std::vector<std::function<void(const ix::WebSocketMessagePtr&)>>> callbacks_;
    std::atomic<bool> running_ = false;
    std::atomic<int> sn_ = 0;
    std::jthread pingThread_;
    void init();
    void pingLoop();
    void setupErrorHandlers();

public:
    enum Opcode
    {
        EVENT,
        PING,
        PONG,
        IDENTIFY,
        READY
    };
    Bot(const std::string& baseAddr, const std::string& token, const std::string& platform, const std::string& userId);
    void addWsCallback(ix::WebSocketMessageType, std::function<void(const ix::WebSocketMessagePtr&)> callback);
    void addOnMessageCallback(std::function<void(const satori::event::Event&)> callback);
    std::string baseUrl() const { return "http://" + baseAddr_ + "/v1"; }
    std::string httpGet(const std::string& url);
    std::string httpPost(const std::string& url, const std::string& body);
    void identify(const std::string& token);
    void launch();
    void launchAsync();
    bool wsSend(const std::string& message);

    MessageApi message{this};
    ChannelApi channel{this};
    UserApi user{this};
    GuildApi guild{this};
    FriendApi friend_{this};
    LoginApi login{this};
    ReactionApi reaction{this};

    std::string userId();
    ~Bot();
};
