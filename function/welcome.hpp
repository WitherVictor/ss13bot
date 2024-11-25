#pragma once

#include <MiraiCP.hpp>
#include <thread>
#include <format>

using namespace MiraiCP;
using namespace std::chrono_literals;

namespace function {

inline void welcome(MemberJoinEvent event) {

    constexpr auto delay_time = 3s;
    std::this_thread::sleep_for(delay_time);

    //  如果加入事件并非来源于指定群，那么直接返回，不做任何事。
    if (event.group.groupId() != QQID{884013901}) {
        Logger::logger.info(std::format("群事件不来自于指定群: {}", 884013901));
        return;
    }

    //  要发送的消息，因为包含 @xxx 消息因此不能直接操作字符串
    MessageChain welcome_message{};

    //  根据邀请与自行加入发送不同的欢迎消息
    if (event.type == MemberJoinEvent::joinType::invited) {
        welcome_message.add(PlainText{"欢迎由 "});
        welcome_message.add(At(event.inviter->id()));
        welcome_message.add(PlainText{std::format(" ({}) 邀请的新员工 ", event.inviter->nickOrNameCard())} );
        welcome_message.add(At(event.member.id()));
        welcome_message.add(PlainText{std::format(" ({})", event.member.nickOrNameCard())} );

        //  打印邀请人和新成员的信息日志
        Logger::logger.info(std::format("邀请人的用户名: {}, QQID: {}", event.inviter->nickOrNameCard(), event.inviter->id()));
        Logger::logger.info(std::format("新成员的用户名: {}, QQID: {}", event.member.nickOrNameCard(), event.member.id()));
    } else {
        welcome_message.add(PlainText{"欢迎新员工 "});
        welcome_message.add(At(event.member.id()));
        welcome_message.add(PlainText{std::format("({})", event.member.id())});

        //  打印新成员的信息日志
        Logger::logger.info(std::format("新成员的用户名: {}, QQID: {}", event.member.nickOrNameCard(), event.member.id()));
    }

    //  将构造好的消息发送到群组中
    event.group.sendMessage(
        welcome_message +
        PlainText{ " !\n" } +
        PlainText{ "员工手册, 空间站地址与服务器规则在公告中.\n" } +
        PlainText{ "工作必须的 Byond 可在群文件中下载.\n" } +
        PlainText{ "请尽情 \"享受\" 你在空间站里的工作吧, 尽你所能努力活下去." }
    );
}
}   //  end of namespace function