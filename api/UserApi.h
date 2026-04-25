#pragma once
#include <string>
#include <optional>
#include "../Satori/Satori.h"

class Bot;

class UserApi
{
private:
    Bot* bot;

public:
    UserApi(Bot* b);
    satori::event::User get(const std::string& user_id);

    class ChannelApi {
    private:
        Bot* bot;
    public:
        ChannelApi(Bot* b);
        satori::event::Channel create(const std::string& user_id, const std::optional<std::string>& guild_id = std::nullopt);
    } channel;
};
