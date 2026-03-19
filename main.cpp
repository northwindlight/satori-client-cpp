#include <ixwebsocket/IXNetSystem.h>

#include <iostream>
#include "nlohmann/json.hpp"
#include "bot.h"
#include "LLMClient/LLMClient.h"

using json = nlohmann::json;

int main()
{
    ix::initNetSystem();
    LLMClient llmClient("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions", "sk-0803a21b2bb24ad4b339bddd5e574aa9", "qwen3-14b");
    llmClient.setPrompt("你是服务器管理助手，你的名字叫做小小北风。\n\
        规则：\n\
        1. 用户询问系统信息时，立刻直接输出JSON指令，禁止询问用户任何问题，禁止解释，禁止确认：格式：{\"tool\": \"fastfetch\", \"args\": \"--logo none --structure XX\"}\n\
        可用structure：CPU / Memory / Disk / OS / Host / Uptime\n\
        2. 根据用户意图自行判断使用哪个structure，例如：\n\
        - 问CPU型号/处理器 → structure CPU\n\
        - 问内存/运行内存/RAM → structure Memory\n\
        - 问硬盘/存储/磁盘 → structure Disk\n\
        - 问系统/操作系统 → structure OS\n\
        - 问运行时间/开机多久 → structure Uptime\n\
        3. 日常闲聊时以小小北风身份友好回复，不输出JSON，不主动提及系统信息\n\
        ");
    Bot rin("127.0.0.1:5600", "TOUHOUPROJECTFOREVER", "3824302087", llmClient);
    rin.addOnMessageCallback([&rin](const satori::Event& event)
    {
        if (event.message.has_value() && event.channel.has_value())
        {
            std::cout << "新消息: " << event.message->content << std::endl;
            std::cout << "消息来自频道: " << event.channel->id << std::endl;
            if (event.channel->id.find("private") == std::string::npos)
            {
                return;
            }
            rin.message.asyncLLM(event.channel->id, event.message.value().content);
        }
    });

    rin.start();
    ix::uninitNetSystem();
    return 0;
}