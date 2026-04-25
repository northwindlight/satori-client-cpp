#pragma once
#include <string>
#include <optional>
#include "../Satori/Satori.h"

class Bot;

class ChannelApi
{
private:
    Bot* bot;

public:
    ChannelApi(Bot* b);
    satori::event::Channel get(const std::string& channel_id);
    satori::List<satori::event::Channel> list(const std::string& guild_id, const std::optional<std::string>& next = std::nullopt);
    satori::event::Channel create(const std::string& guild_id, const satori::event::Channel& data);
    void update(const std::string& channel_id, const satori::event::Channel& data);
    void delete_(const std::string& channel_id);
    void mute(const std::string& channel_id, int duration);
};
