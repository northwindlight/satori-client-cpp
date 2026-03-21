#include <ixwebsocket/IXNetSystem.h>
#include <iostream>
#include "Satori/Satori.h"
#include "bot.h"
#include "LLMClient/LLMClient.h"
#include "LLMClient/LLMAgent.h"

int main()
{
    ix::initNetSystem();

    //如果需要llm功能，需要创建llmclient对象，参数为url, api_key, model
    LLMClient llm(
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
        "sk-xxxxx",
        "qwen3-14b"
    );

    //创建llmagent并绑定llmclient对象和提示词
    //如果需要调用工具，请约定为{"tool": xxxx, "args": xxxx}
    //如果需要修改约定格式，调整LLMAgent的parseToolCall方法
    LLMAgent agent(llm,
        "你是服务器管理助手，名字叫小小北风。\n\n"
        "【系统信息查询】\n"
        "关于本机的任何系统信息，你一概不知，严禁凭知识猜测或编造，任何信息必须来自工具。\n"
        "你就是本机代表，当用户询问关于你的系统信息，你必须调用工具"
        "只要涉及系统信息，立即输出以下JSON，禁止解释、确认或提问：\n"
        "{\"tool\":\"fastfetch\",\"args\":\"--logo none --structure <XX>\"}\n\n"
        "structure选择：\n"
        "OS(操作系统/发行版) | CPU(处理器) | Memory(内存) | Disk(磁盘) | Host(主机名) | Uptime(运行时间)\n\n"
        "【日常对话】\n"
        "非系统信息的闲聊，以小小北风身份友好回复，不输出JSON。\n"
    );
    agent.toolRegistry({"fastfetch"});

    //satori bot客户端，参数分别为satori服务器地址，token，平台名称，平台账号
    Bot rin("127.0.0.1:5600", "TOUHOUPROJECTFOREVER", "QQ", "3824302087");
    //satori bot消息回调，通过addOnMessageCallback来自行实现bot功能
    rin.addOnMessageCallback([&rin, &agent](const satori::Event& event)
    {
        if (!event.message.has_value() || !event.channel.has_value()) return;

        const std::string& channelId = event.channel->id;
        const std::string& content   = event.message->content;
        std::cout << "新消息 [" << channelId << "]: " << content << std::endl;
        
        satori::Elements elements = satori::parseContent(content);
        auto it = std::ranges::find_if(elements.ats, [&](const auto& at) {
            return at.id.has_value() && at.id.value() == rin.getUserID();
        });

        if (event.channel->id.find("private") != std::string::npos && it != elements.ats.end()) {
            agent.ask(content, [&rin, channelId](const std::string& reply) 
            {
                rin.message.create(channelId, reply);
            });
        }

    });

    rin.start();
    ix::uninitNetSystem();
    return 0;
}
