#include "bot.h"
#include <iostream>

Bot::Bot(const std::string& baseAddr, const std::string& token, const std::string& platform, const std::string& userID) : baseAddr(baseAddr), token(token), userID(userID), platform(platform)
{
    webSocket.setUrl("ws://" + baseAddr + "/v1/events");
    args = httpClient.createRequest();
    args->connectTimeout = 5;
    args->transferTimeout = 10;
    headers["Content-Type"] = "application/json";
    headers["Satori-Platform"] = platform;
    headers["Authorization"] = "Bearer " + token;
    headers["Satori-User-ID"] = userID;
    args->extraHeaders = headers;
    identify(token);
    exceptionHandling();
}

void Bot::addWSCallback(ix::WebSocketMessageType type, std::function<void(const ix::WebSocketMessagePtr&)> callback)
{
    callbacksByType[type].emplace_back(std::move(callback));
}

void Bot::addOnMessageCallback(std::function<void(const satori::event::Event&)> callback)
{
    addWSCallback(ix::WebSocketMessageType::Message, [this, callback = std::move(callback)](const ix::WebSocketMessagePtr& msg)
    {
        try
        {
            auto json = nlohmann::json::parse(msg->str);
            Opcode op = json["op"];
            if (op == EVENT)
            {
                satori::event::Event event = json["body"].get<satori::event::Event>();
                sn = event.sn;
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
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        auto it = callbacksByType.find(msg->type);
        if (it != callbacksByType.end()) {
            for (const auto& callback : it->second) {
                try 
                {
                    callback(msg);
                } 
                catch (const std::exception& e) 
                {
                    std::cerr << "setOnMessageCallback 错误: " << e.what() << std::endl;
                }
            }
        }
    });
    webSocket.start();
}

void Bot::launch()
{   
    if (running) return;
    running = true;
    init();
    pingLoop();
}

void Bot::launchAsync()
{
    if (running) return;
    running = true;
    init();
    pingThread = std::jthread(&Bot::pingLoop, this);
}

bool Bot::wsSend(const std::string& message)
{
    return webSocket.send(message).success;
}

void Bot::pingLoop()
{
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(8));

        if (webSocket.getReadyState() == ix::ReadyState::Open)
        {
            nlohmann::json ping;
            ping["op"] = PING;
            if (!wsSend(ping.dump()))
            {
                std::cerr << "发送 PING 失败" << std::endl;
            }
        }
    }
}

void Bot::identify(const std::string& token)
{
    this->token = token;
    addWSCallback(ix::WebSocketMessageType::Open, [this, token](const ix::WebSocketMessagePtr& msg)
    {
        std::cout << "连接已建立，发送 IDENTIFY..." << std::endl;
        running = true;
        nlohmann::json identify;
        identify["op"] = IDENTIFY;
        identify["body"]["token"] = token;
        if (sn) identify["body"]["sn"] = sn.load();
        if (!wsSend(identify.dump()))
        {
            std::cerr << "发送 IDENTIFY 失败" << std::endl;
        }
    });
    addWSCallback(ix::WebSocketMessageType::Message, [this](const ix::WebSocketMessagePtr& msg)
    {
        nlohmann::json received = nlohmann::json::parse(msg->str);
        Opcode op = received["op"];
        if (op == READY)
        {
            std::cout << "鉴权成功，开始接收事件" << std::endl;
        }
    });
}

void Bot::exceptionHandling()
{
    addWSCallback(ix::WebSocketMessageType::Error, [this](const ix::WebSocketMessagePtr& msg)
    {
        std::cerr << "WebSocket 错误: " << msg->errorInfo.reason << std::endl;
    });

    addWSCallback(ix::WebSocketMessageType::Close, [this](const ix::WebSocketMessagePtr& msg)
    {
        std::cerr << "WebSocket 连接关闭: " << msg->closeInfo.code << " - " << msg->closeInfo.reason << std::endl;
    });
}

std::string Bot::httpGet(const std::string& url)
{
    auto response = httpClient.get(url, args);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {   
        throw std::runtime_error("HTTP GET 失败: " + std::to_string(response->statusCode)+ ": " + response->errorMsg + "\n响应内容: " + response->body);
    }
    return response->body;
}

std::string Bot::httpPost(const std::string& url, const std::string& body)
{
    auto response = httpClient.post(url, body, args);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {
        throw std::runtime_error("HTTP POST 失败: " + std::to_string(response->statusCode) + ": " + response->errorMsg + "\n响应内容: " + response->body);
    }
    return response->body;
}

std::string Bot::getUserID()
{
    return userID;
}

Bot::~Bot()
{
    running = false;
    webSocket.close();
}

