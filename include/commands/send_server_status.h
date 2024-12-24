#pragma once

//  MiraiCP 依赖
#include <MiraiCP.hpp>

namespace command {

using namespace MiraiCP;

/**
 * @brief 向群内发送所查询的服务器信息
 * 
 */
void send_server_status(GroupMessageEvent, const std::string&);

}   //  end of namespace plugin