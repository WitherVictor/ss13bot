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

        //  如果消息类型不是纯文本则返回
        auto message_json = json::parse(event.message.toString());
        if (message_json.at(0).at("type") != "PlainText")
            return;

        //  如果随机数 >= 概率，则返回
        if (range(function::engine) >= possibility)
            return;

        auto message = message_json.at(0).at("content").get<std::string>();
        boost::regex regex("^((http[s]?|ftp):\\/)?\\/?([^:\\/\\s]+)((\\/\\w+)*\\/)([\\w\\-\\.]+[^#?\\s]+)(.*)?(#[\\w\\-]+)?$");

        if (!message.empty() && !boost::regex_match(message, regex))
            function::group_message_json["phrases"] += message;
    }
}

#endif