#pragma once

//  MiraiCP
#include <MiraiCP.hpp>

//  标准库
#include <string>

namespace command {

using namespace MiraiCP;

/**
 * @brief 查询服务器的 IP 地址
 * 
 */
void ip(GroupMessageEvent, const std::string&);

}   //  end of namespace command