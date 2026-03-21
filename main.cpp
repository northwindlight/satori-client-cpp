#include <ixwebsocket/IXNetSystem.h>
#include <iostream>
#include "bot.h"
#include "LLMClient/LLMClient.h"
#include "LLMClient/LLMAgent.h"

int main()
{
    ix::initNetSystem();

    //如果需要llm功能，需要创建llmclient对象，参数为url, api_key, model
    LLMClient llm(
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
        "sk-xxxx",
        "qwen3-14b"
    );

    //创建llmagent并绑定llmclient对象和提示词
    //如果需要调用工具，请约定为{"tool": xxxx, "args": xxxx}
    //如果需要修改约定格式，调整LLMAgent的parseToolCall方法
    LLMAgent agent(llm,
        "你是服务器管理助手，你的名字叫做小小北风。\n"
        "规则：\n"
        "1. 用户询问系统信息时，立刻直接输出JSON指令，禁止询问用户任何问题，禁止解释，禁止确认：\n"
        "   格式：{\"tool\": \"fastfetch\", \"args\": \"--logo none --structure XX\"}\n"
        "   可用structure：CPU / Memory / Disk / OS / Host / Uptime\n"
        "2. 根据用户意图自行判断使用哪个structure\n"
        "3. 日常闲聊时以小小北风身份友好回复，不输出JSON\n"
    );

    //satori bot客户端，参数分别为satori服务器地址，token，平台名称，平台账号
    Bot rin("127.0.0.1:5600", "TOUHOUPROJECTFOREVER", "QQ", "3824302087");
    //satori bot消息回调，通过addOnMessageCallback来自行实现bot功能
    rin.addOnMessageCallback([&rin, &agent](const satori::Event& event)
    {
        if (!event.message.has_value() || !event.channel.has_value()) return;
        if (event.channel->id.find("private") == std::string::npos) return;

        const std::string& channelId = event.channel->id;
        const std::string& content   = event.message->content;
        std::cout << "新消息 [" << channelId << "]: " << content << std::endl;

        agent.ask(content, [&rin, channelId](const std::string& reply) 
        {
            rin.message.create(channelId, reply);
        });
    });

    rin.start();
    ix::uninitNetSystem();
    return 0;
}
