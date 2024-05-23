#ifndef __FUNCTION_MEMBER_LEAVE_HPP__
#define __FUNCTION_MEMBER_LEAVE_HPP__

#include <MiraiCP.hpp>

using namespace MiraiCP;

namespace function
{
    inline void member_leave(MemberLeaveEvent event)
    {
        MessageChain buffer{};

        buffer.add( At(event.member.id()) );
        buffer.add( PlainText{std::format("({}) 从纳米传讯卸职, 离开了太空站", event.member.nickOrNameCard())} );

        event.group.sendMessage(buffer);
    }
}

#endif