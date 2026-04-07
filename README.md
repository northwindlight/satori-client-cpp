# satori-client-cpp

一个基于 C++ 的 [Satori 协议](https://satori.chat/zh-CN/introduction.html) 客户端框架。

**本框架不能独立工作**，需要配合额外的 Satori 服务器（如 [LLBot](https://www.llonebot.com/) 等）使用。

这是一个简单的个人项目，代码使用现代 C++，**必须使用 C++20 或更高标准**。目前存在诸多不足，正在持续更新。

**本框架不是开箱即用的 Bot 框架**，不会自动帮你实现任何机器人功能。你需要熟悉 C++，自己在回调中实现业务逻辑。

**本项目几乎没有使用AI**，我承认现在编程离不开AI，但是不要把我的劳动和vibe创作混为一谈，其中98%的代码是我独立创作的，我希望大家能够尊重作者的创作价值。

---

## 当前实现状态

**Satori 资源结构**：已完整实现解析（Event、Message、Channel、User、Guild、GuildMember 等）

**Satori 标准元素**：已基本实现解析和封装（ `<at>`、`<sharp>`、`<file>`、`<img>`、`<video>` 等）

**Satori API**：目前仅实现 `message.create`

**LLMClient**：可选功能。鉴于 AI 热潮，目前也是主要支持方向。

---

## 示例

### 不使用 LLMClient

```cpp
#include "bot.h"
#include "Satori/Satori.h"
#include <ixwebsocket/IXNetSystem.h>

namespace se = satori::event;
namespace sel = satori::element;

int main()
{
    ix::initNetSystem();

    Bot bot("127.0.0.1:5600", "YOUR_TOKEN", "QQ", "YOUR_BOT_QQ");

    bot.addOnMessageCallback([&bot](const se::Event& event)
    {
        if (!event.message || !event.channel) return;
        //如果需要解析消息元素使用satori::element::parse
        sel::Elements elems = sel::parse(event.message->content);
        // 在这里实现你的业务逻辑
        bot.message.create(event.channel->id, "收到：" + elems.plainText);
    });

    bot.launch(); // 阻塞主线程
    // bot.launchAsync()异步执行
    ix::uninitNetSystem();
}
```

### 使用 LLMClient

```cpp
#include "bot.h"
#include "Satori/Satori.h"
#include "LLMClient/LLMClient.h"
#include "LLMClient/LLMAgent.h"
#include <ixwebsocket/IXNetSystem.h>

namespace se = satori::event;
namespace sel = satori::element;

int main()
{
    ix::initNetSystem();

    LLMClient llm(
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
        "sk-xxxxx",
        "qwen3-14b"
    );


    Bot bot("127.0.0.1:5600", "YOUR_TOKEN", "QQ", "YOUR_BOT_QQ");

    bot.addOnMessageCallback([&bot, &llm](const se::Event& event)
    {
        if (!event.message || !event.channel) return;

        sel::Elements elems = sel::parse(event.message->content);
        //创建agent管理llm上下文
        //必须使用shared_ptr保证在回调中的生命周期问题
        //如果agent放在外部，则所有对话保持都保持上下文，放在内部则每个对话独立保持上下文
        auto agent = std::make_shared<LLMAgent>(llm,
            "你是服务器管理助手，名字叫小小北风。\n"
        );
        agent->ask(elems.plainText, [&bot, event](const std::string& llmreply)
        {
            //使用builder构造标准元素消息
            auto reply = sel::Builder()
                .img("weburl")
                .text(llmreply)
                .build();
            bot.message.create(event.channel->id, reply);
        });
    });

    bot.launch();
    ix::uninitNetSystem();
}
```

---
更多示例请参照demo.cpp
## 依赖

| 依赖 | 说明 |
|---|---|
| [ixwebsocket](https://github.com/machinezone/IXWebSocket) | WebSocket + HTTP，外部库，需要启用TLS支持 |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON 解析，已内嵌 |
| [tinyxml2](https://github.com/leethomason/tinyxml2) | XML 解析，已内嵌 |

```bash
# Ubuntu
sudo apt install libixwebsocket-dev

cmake -B build && cmake --build build
```

---

## License

MIT License © 2026 Zhang Feng
