#include <ixwebsocket/IXNetSystem.h>
#include <iostream>
#include "Satori/Satori.h"
#include "bot.h"
#include "LLMClient/LLMClient.h"
#include "LLMClient/LLMAgent.h"

namespace se = satori::event;
namespace sel = satori::element;


int main()
{
    //这是ix的全局网络系统初始化，如果不是Windows平台可以删除
    ix::initNetSystem();

    //如果需要llm功能，需要创建llmclient对象，参数为url, api_key, model
    //请自己获取api_key
    LLMClient llm(
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
        "sk-xxxxx",
        "qwen3-14b"
    );

    //satori bot客户端，参数分别为satori服务器地址，token，平台名称，平台账号
    //这些由satori服务端sdk提供
    Bot rin("127.0.0.1:5600", "TOUHOUPROJECTFOREVER", "QQ", "3824302087");
    //satori bot消息回调，通过addOnMessageCallback来自行实现bot功能
    rin.addOnMessageCallback([&rin, &llm](const se::Event& event)
    {
        if (!event.message.has_value() || !event.channel.has_value()) return;

        //若需要解析content，这里提供了satori::parseContent来处理satori的标准元素
        sel::Elements elements = sel::parse(event.message->content);
        std::cout << "新消息 [" << event.channel->id << "]: " << event.message->content << std::endl;
        
        auto it = std::ranges::find_if(elements.ats, [&](const auto& at) {
            return at.id.has_value() && at.id.value() == rin.getUserID();
        });

        if (event.channel->id.find("private") != std::string::npos || it != elements.ats.end()) {
            //使用shared_ptr保证在回调中的生命周期问题
            auto agent = std::make_shared<LLMAgent>(llm,
                "你是服务器管理助手，名字叫小小北风。\n\n"
            );
            agent->ask(elements.plainText , [&rin, event](const std::string& llmreply) 
            {
                //如果你想调用工具，可以再这里实现自己的调用工具功能
                //然后再次调用ask来进行下一轮会话
                //使用builder构造标准元素消息
                auto reply = sel::Builder()
                .at(event.user->id, event.user->name.value())
                .text(llmreply)
                .build();
                rin.message.create(event.channel->id, reply);
            });
        }

    });

    rin.start();
    ix::uninitNetSystem();
    return 0;
}
