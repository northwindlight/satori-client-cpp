#pragma once
#include <string>
#include <optional>
#include "../Satori/Satori.h"

class Bot;

class FriendApi
{
private:
    Bot* bot;

public:
    FriendApi(Bot* b);
    satori::List<satori::event::Friend> list(const std::optional<std::string>& next = std::nullopt);
    void delete_(const std::string& user_id);
    void approve(const std::string& message_id, bool approve_, const std::optional<std::string>& comment = std::nullopt);
};
