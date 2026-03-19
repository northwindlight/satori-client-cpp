#include "bot.h"
#include <iostream>

void Bot::init()
{
    webSocket.setUrl("ws://" + baseAddr + "/v1/events");
    args = httpClient.createRequest();
    args->connectTimeout = 5;
    args->transferTimeout = 10;
    headers["Content-Type"] = "application/json";
    headers["Satori-Platform"] = "QQ";
    headers["Authorization"] = "Bearer " + token;
    headers["Satori-User-ID"] = userID;
    args->extraHeaders = headers;
    identify(token);
    exceptionHandling();
}
Bot::Bot(const std::string& baseAddr, const std::string& token, const std::string& userID) : baseAddr(baseAddr), token(token), userID(userID)
{
    init();
}

Bot::Bot(const std::string& baseAddr, const std::string& token, const std::string& userID, LLMClient& client) : Bot(baseAddr, token, userID)
{
    this->client = &client;
}

void Bot::addWSCallback(ix::WebSocketMessageType type, std::function<void(const ix::WebSocketMessagePtr&)> callback)
{
    callbacksByType[type].emplace_back(std::move(callback));
}

void Bot::addOnMessageCallback(std::function<void(const satori::Event&)> callback)
{
    addWSCallback(ix::WebSocketMessageType::Message, [this, callback = std::move(callback)](const ix::WebSocketMessagePtr& msg)
    {
        try
        {
            auto json = nlohmann::json::parse(msg->str);
            Opcode op = json["op"];
            if (op == EVENT)
            {
                satori::Event event = json["body"].get<satori::Event>();
                sn = event.sn;
                callback(event);
            }
            
        }
        catch (const std::exception& e)
        {
            std::cerr << "JSON parse error in addOnMessageCallback: " << e.what() << std::endl;
        }
    });
}

void Bot::start()
{   
    if (running) return;
    running = true;
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
                    std::cerr << "callback error: " << e.what() << std::endl;
                }
            }
        }
    });
    webSocket.start();
    pingLoop();
}

void Bot::wsSend(const std::string& message)
{
    webSocket.send(message);
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
            webSocket.send(ping.dump());
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
        if (sn) identify["body"]["sn"] = sn;
        wsSend(identify.dump());
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
        running = false;
    });
}

std::optional<std::string> Bot::httpGet(const std::string& url)
{
    auto response = httpClient.get(url, args);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {   
        std::cerr << "HTTP GET 错误: " << response->statusCode << std::endl;
        return std::nullopt;
    }
    return response->body;
}

std::optional<std::string> Bot::httpPost(const std::string& url, const std::string& body)
{
    auto response = httpClient.post(url, body, args);
    if (response->errorCode != ix::HttpErrorCode::Ok)
    {
        std::cerr << "HTTP POST 错误: " << response->statusCode << std::endl;
        return std::nullopt;
    }
    return response->body;
}