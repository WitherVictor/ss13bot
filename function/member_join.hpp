#ifndef __FUNCTION_MEMBER_JOIN_HPP__
#define __FUNCTION_MEMBER_JOIN_HPP__

#include <MiraiCP.hpp>
#include <thread>

using namespace MiraiCP;
using namespace std::chrono_literals;

namespace function
{
    inline void member_join(MemberJoinEvent event)
    {
        std::this_thread::sleep_for(5s);

        Logger::logger.info(std::format("member join event type: {}", static_cast<std::size_t>(event.type)));

        MessageChain buffer{};

        //  若用户是被邀请进来的
        if (event.type == MemberJoinEvent::joinType::invited)
        {
            buffer.add(PlainText{"欢迎由 "});
            buffer.add(At(event.inviter->id()));
            buffer.add(PlainText{std::format(" ({}) 邀请的新员工 ", event.inviter->nickOrNameCard())} );
            buffer.add(At(event.member.id()));
            buffer.add(PlainText{std::format(" ({})", event.member.nickOrNameCard())} );
        }
        //  若用户为自己加入的 此处有 MiraiCP 的 bug, 必须以此形式发送
        else
        {
            buffer.add(PlainText{"欢迎新员工 "});
            buffer.add(At(event.member.id()));
            buffer.add( PlainText{ std::format(" ({})", event.member.nickOrNameCard())} );
        }

        event.group.sendMessage
        (
            buffer +
            PlainText{ " !\n" } +
            PlainText{ "员工手册, 空间站地址与服务器规则在公告中\n" } +
            PlainText{ "工作必须的 Byond 可在群文件中下载\n" } +
            PlainText{ "请尽情 \"享受\" 你在空间站里的工作吧, 尽你所能努力活下去" }
        );
    }
};

#endif