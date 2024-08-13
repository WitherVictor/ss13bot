#ifndef __FUNCTION_READ_GROUP_MESSAGE_HPP
#define __FUNCTION_READ_GROUP_MESSAGE_HPP

#include "command.hpp"
#include <MiraiCP.hpp>
#include <boost/regex.hpp>
#include <boost/regex/v5/regex_match.hpp>
#include <random>

namespace function
{
    inline void read_group_message(MiraiCP::GroupMessageEvent event)
    {
        //  按概率记录信息
        static std::uniform_int_distribution<std::size_t> range {0, 100};

        //  假设概率为 10%
        static constexpr std::size_t possibility = 10;

        //  如果不是主群的消息，则不记录
        if (event.group.groupId() != static_cast<QQID>(884013901ULL))
            return;

        //  手动解析 MiraiCode, 如果消息类型不是纯文本则返回
        auto message_json = json::parse(event.message.toString());
        if (message_json.at(0).at("type") != "PlainText")
            return;

        //  获得信息的字符串
        auto message = message_json.at(0).at("content").get<std::string>();

        //  如果消息为空, 则不记录这条消息
        if (message.empty())
            return;

        //  如果消息内容开头是 '.' 则返回, 我们不希望 Poly 读入指令类型的信息
        if (message.front() == '.')
            return;

        //  定义表示 URL 的 regex, 如果这条消息是一条 URL, 则直接返回
        boost::regex regex("^((http[s]?|ftp):\\/)?\\/?([^:\\/\\s]+)((\\/\\w+)*\\/)([\\w\\-\\.]+[^#?\\s]+)(.*)?(#[\\w\\-]+)?$");
        if (boost::regex_match(message, regex))
            return;       

        //  如果随机数在概率区间内, 则存入这条消息
        if (range(function::engine) < possibility)
            function::group_message_json["phrases"] += message;
    }
}

#endif