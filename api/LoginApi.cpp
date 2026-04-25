#include "LoginApi.h"
#include "../bot.h"
#include "../nlohmann/json.hpp"

LoginApi::LoginApi(Bot* b) : bot(b) {}

satori::event::Login LoginApi::get()
{
    try
    {
        std::string response = bot->httpPost(bot->baseUrl() + "/login.get", "{}");
        return nlohmann::json::parse(response).get<satori::event::Login>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("LoginApi::get error: ") + e.what());
    }
}
