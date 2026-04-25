#include <ixwebsocket/IXNetSystem.h>
#include <iostream>
#include <ctime>
#include "Satori/Satori.h"
#include "bot.h"
#include "LLMClient/LLMClient.h"
#include "LLMClient/LLMAgent.h"
#include "LLMClient/tool.hpp"

namespace se  = satori::event;
namespace sel = satori::element;


static const std::string SYSTEM_PROMPT =
    "你是智能助手，性格友好简洁。你可以使用以下工具来帮助用户：\n\n"
    "【get_time】\n"
    "当用户询问时间、日期时，调用 get_time 工具获取当前系统时间。\n\n"
    "【calc】\n"
    "当用户要求计算数学表达式时，调用 calc 工具进行计算。\n\n"
    "【echo】\n"
    "当用户要求回显某段文本时，调用 echo 工具。\n";

// ── 工具注册 ─────────────────────────────────────────────────────

static void registerTools(Tool& tool)
{
    tool.toolRegister(
        "get_time",
        "获取当前系统时间",
        {},
        [](const nlohmann::json&) -> std::string {
            auto now = std::chrono::system_clock::now();
            auto t   = std::chrono::system_clock::to_time_t(now);
            std::string s = std::ctime(&t);
            if (!s.empty() && s.back() == '\n') s.pop_back();
            return s;
        }
    );

    tool.toolRegister(
        "calc",
        "计算数学表达式（支持 + - * /）",
        {{"expression", "string", "要计算的数学表达式，例如 1 + 2 * 3", true}},
        [](const nlohmann::json& args) -> std::string {
            std::string expr = args.value("expression", "");
            if (expr.empty()) return "[错误: 表达式为空]";
            std::string cmd = "python3 -c \"print(eval('" + expr + "'))\" 2>&1";
            auto p = popen(cmd.c_str(), "r");
            if (!p) return "[执行失败]";
            std::string result;
            char buf[128];
            while (fgets(buf, sizeof(buf), p)) result += buf;
            pclose(p);
            if (result.empty()) return "[无结果]";
            if (!result.empty() && result.back() == '\n') result.pop_back();
            return result;
        }
    );

    tool.toolRegister(
        "echo",
        "回显输入内容，用于测试",
        {{"text", "string", "要回显的文本", true}},
        [](const nlohmann::json& args) -> std::string {
            return "你说的是: " + args.value("text", "");
        }
    );
}


int main()
{
    ix::initNetSystem();

    LLMClient llm(
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
        "sk-xxxxx",
        "qwen3-14b"
    );

    Tool tool;
    registerTools(tool);
    auto agent = LLMAgent::create(llm, SYSTEM_PROMPT, tool);
    Bot rin("127.0.0.1:5600", "TOUHOUPROJECTFOREVER", "QQ", "3824302087");

    rin.addOnMessageCallback([&rin, agent](const se::Event& event)
    {
        if (!event.message.has_value() || !event.channel.has_value()) return;

        sel::Elements elements = sel::parse(event.message->content.value_or(""));
        std::cout << "新消息 [" << event.channel->id << "]: " << event.message->content.value_or("") << std::endl;

        auto it = std::ranges::find_if(elements.ats, [&](const auto& at) {
            return at.id.has_value() && at.id.value() == rin.getUserID();
        });

        bool triggered = event.channel->id.find("private") != std::string::npos || it != elements.ats.end();
        if (!triggered) return;

        if (event.user.has_value())
        {
            std::string systemPrompt = SYSTEM_PROMPT + "\n当前用户 ID：" + event.user->id + "\n";
            agent->setSystemPrompt(systemPrompt);
        }

        auto buffer = std::make_shared<sel::Builder>();
        std::string channelId = event.channel->id;

        agent->ask(elements.plainText, [&rin, buffer, channelId](const std::string& reply)
        {
            buffer->text(reply);
            std::string finalMsg = buffer->build();
            std::cout << "回复: " << finalMsg << std::endl;
            try {
                rin.message.create(channelId, finalMsg);
            } catch (const std::exception& e) {
                std::cerr << "发送消息失败: " << e.what() << std::endl;
            }
        });
    });

    rin.launch();
    ix::uninitNetSystem();
    return 0;
}
