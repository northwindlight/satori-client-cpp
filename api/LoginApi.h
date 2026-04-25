#pragma once
#include "../Satori/Satori.h"

class Bot;

class LoginApi
{
private:
    Bot* bot;

public:
    LoginApi(Bot* b);
    satori::event::Login get();
};
