#pragma once
#include <stdexcept>
#include <string>

namespace api {

// 统一的 API 调用错误处理包装器。
// 用法:
//   return api::call("ClassName::method", [&] {
//       return nlohmann::json::parse(bot->httpPost(...)).get<T>();
//   });
template<typename Func>
auto call(const std::string& name, Func&& f) -> decltype(f())
{
    try {
        return f();
    } catch (const std::exception& e) {
        throw std::runtime_error(name + " error: " + e.what());
    }
}

} // namespace api
