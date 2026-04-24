#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <thread>
#include "../nlohmann/json.hpp"
#include "LLMClient.h"

class Tool
{
public:
    struct ParamDef {
        std::string name;
        std::string type;        // "string" | "integer" | "boolean"
        std::string description;
        bool        required = true;
    };

    struct Call {
        std::string         id;    // tool_call_id，回传给模型用
        std::string         name;
        nlohmann::json      args;  // 结构化参数，不再是裸字符串
    };

    using Handler = std::function<std::string(const nlohmann::json& args)>;

    void toolRegister(const std::string& name, const std::string& description, std::vector<ParamDef> params, Handler handler)
    {
        entries[name] = { description, std::move(params), std::move(handler) };
    }

    // ── 生成发给模型的 tools[] schema ────────────────────────
    nlohmann::json toSchema() const
    {
        nlohmann::json arr = nlohmann::json::array();
        for (auto& [name, entry] : entries)
        {
            nlohmann::json props  = nlohmann::json::object();
            nlohmann::json req    = nlohmann::json::array();

            for (auto& p : entry.params)
            {
                props[p.name] = {{"type", p.type}, {"description", p.description}};
                if (p.required) req.push_back(p.name);
            }

            arr.push_back({
                {"type", "function"},
                {"function", {
                    {"name",        name},
                    {"description", entry.description},
                    {"parameters",  {
                        {"type",       "object"},
                        {"properties", props},
                        {"required",   req}
                    }}
                }}
            });
        }
        return arr;
    }

    // ── 从 ToolCall 构造 Call（LLMAgent 内部用）──────────────
    static Call fromToolCall(const LLMClient::ToolCall& tc)
    {
        nlohmann::json args;
        if (!tc.arguments.empty() && nlohmann::json::accept(tc.arguments)) args = nlohmann::json::parse(tc.arguments);
        return { tc.id, tc.name, args };
    }

    // ── 异步执行，回调返回结果字符串 ─────────────────────────
    void run(const Call& call, std::function<void(const std::string& output)> cb)
    {
        auto it = entries.find(call.name);
        if (it == entries.end())
        {
            cb("[工具未注册: " + call.name + "]");
            return;
        }
        std::thread([handler = it->second.handler, args = call.args, cb = std::move(cb)]()
        {
            try 
            {
                cb(handler(args)); 
            }
            catch (const std::exception& e)
            { 
                cb("[工具执行失败: " + std::string(e.what()) + "]"); 
            }
        }).detach();
    }

private:
    struct Entry {
        std::string           description;
        std::vector<ParamDef> params;
        Handler               handler;
    };
    std::unordered_map<std::string, Entry> entries;
};