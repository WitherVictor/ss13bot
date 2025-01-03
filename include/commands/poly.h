#pragma once

#include <MiraiCP.hpp>

#include <config.h>

#include <filesystem>

using namespace MiraiCP;

class poly {
public:
    poly(std::filesystem::path);

    static const std::vector<std::string> get_message();
private:
    std::vector<std::string> m_message_vector;
};

namespace command {

void send_poly_message(GroupMessageEvent);

}