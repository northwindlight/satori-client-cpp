#include "bot.h"
#include <iostream>
#include <stdexcept>

Bot::Bot(const std::string& baseAddr, const std::string& token, const std::string& platform, const std::string& userId)
    : baseAddr_(baseAddr), token_(token), userId_(userId), platform_(platform)
{
    webSocket_.setUrl("ws://" + baseAddr_ + "/v1/events");
    args_ = httpClient_.createRequest();
    args_->connectTimeout = 5;
    args_->transferTimeout = 60;
    headers_["Content-Type"] = "application/json";
    headers_["Satori-Platform"] = platform_;
    headers_["Authorization"] = "Bearer " + token_;
    headers_["Satori-User-ID"] = userId_;
    args_->extraHeaders = headers_;
    identify(token_);
    setupErrorHandlers();
}

void Bot::addWsCallback(ix::WebSocketMessageType type, std::function<void(const ix::WebSocketMessagePtr&)> callback)
{
    if (running_) throw std::logic_error("cannot add callbacks after launch()");
    callbacks_[type].emplace_back(std::move(callback));
}

void Bot::addOnMessageCallback(std::function<void(const satori::event::Event&)> callback)
{
    if (running_) throw std::logic_error("cannot add callbacks after launch()");
    addWsCallback(ix::WebSocketMessageType::Message, [this, callback = std::move(callback)](const ix::WebSocketMessagePtr& msg)
    {
        try
        {
            auto json = nlohmann::json::parse(msg->str);
            Opcode op = json["op"];
            if (op == EVENT)
            {
                satori::event::Event event = json["body"].get<satori::event::Event>();
                sn_ = event.sn;
                callback(event);
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("解析消息失败: ") + e.what());
        }
    });
}

void Bot::init()
{
    webSocket_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        auto it = callbacks_.find(msg->type);
        if (it != callbacks_.end()) {
            for (const auto& callback : it->second) {
                try
                {
                    callback(msg);
                }
                catch (const std::exception& e)
                {
                    reportError(std::string("setOnMessageCallback error: ") + e.what());
                }
            }
        }
    });
    webSocket_.start();
}

void Bot::launch()
{
    if (running_) return;
    running_ = true;
    init();
    pingLoop();
}

void Bot::launchAsync()
{
    if (running_) return;
    running_ = true;
    init();
    pingThread_ = std::jthread(&Bot::pingLoop, this);
}

bool Bot::wsSend(const std::string& message)
{
    return webSocket_.send(message).success;
}

void Bot::pingLoop()
{
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(8));

        if (webSocket_.getReadyState() == ix::ReadyState::Open)
        {
            nlohmann::json ping;
            ping["op"] = PING;
            if (!wsSend(ping.dump()))
            {
                reportError("发送 PING 失败");
            }
        }
    }
}

void Bot::identify(const std::string& token)
{
    token_ = token;
    addWsCallback(ix::WebSocketMessageType::Open, [this, token](const ix::WebSocketMessagePtr& msg)
    {
        std::cout << "连接已建立，发送 IDENTIFY..." << std::endl;
        running_ = true;
        nlohmann::json identify;
        identify["op"] = IDENTIFY;
        identify["body"]["token"] = token;
        if (sn_) identify["body"]["sn"] = sn_.load();
        if (!wsSend(identify.dump()))
        {
            reportError("发送 IDENTIFY 失败");
        }
    });
    addWsCallback(ix::WebSocketMessageType::Message, [this](const ix::WebSocketMessagePtr& msg)
    {
        nlohmann::json received = nlohmann::json::parse(msg->str);
        Opcode op = received["op"];
        if (op == READY)
        {
            std::cout << "鉴权成功，开始接收事件" << std::endl;
        }
    });
}

void Bot::setupErrorHandlers()
{
    addWsCallback(ix::WebSocketMessageType::Error, [this](const ix::WebSocketMessagePtr& msg)
    {
        reportError("WebSocket error: " + msg->errorInfo.reason);
    });

    addWsCallback(ix::WebSocketMessageType::Close, [this](const ix::WebSocketMessagePtr& msg)
    {
        reportError("WebSocket closed: " + std::to_string(msg->closeInfo.code) + " - " + msg->closeInfo.reason);
    });
}

std::string Bot::httpGet(const std::string& url)
{
    auto response = httpClient_.get(url, args_);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {
        throw std::runtime_error("HTTP GET 失败: " + std::to_string(response->statusCode)+ ": " + response->errorMsg + "\n响应内容: " + response->body);
    }
    return response->body;
}

std::string Bot::httpPost(const std::string& url, const std::string& body)
{
    auto response = httpClient_.post(url, body, args_);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {
        throw std::runtime_error("HTTP POST 失败: " + std::to_string(response->statusCode) + ": " + response->errorMsg + "\n响应内容: " + response->body);
    }
    return response->body;
}

std::string Bot::userId()
{
    return userId_;
}

Bot::~Bot()
{
    running_ = false;
    webSocket_.close();
}
