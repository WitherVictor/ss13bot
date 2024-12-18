#pragma once

#include <MiraiCP.hpp>

namespace function {

using namespace MiraiCP;

/**
 * @brief 新成员加入时向群内发送欢迎消息
 * 
 * @param event 
 */
void welcome(MemberJoinEvent event);

}   //  end of namespace function