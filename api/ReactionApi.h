#pragma once
#include <string>
#include <optional>
#include "../Satori/Satori.h"

class Bot;

class ReactionApi
{
private:
    Bot* bot;

public:
    ReactionApi(Bot* b);
    void create(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id);
    void delete_(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id, const std::optional<std::string>& user_id = std::nullopt);
    void clear(const std::string& channel_id, const std::string& message_id, const std::optional<std::string>& emoji_id = std::nullopt);
    satori::List<satori::event::User> list(const std::string& channel_id, const std::string& message_id, const std::string& emoji_id, const std::optional<std::string>& next = std::nullopt);
};
