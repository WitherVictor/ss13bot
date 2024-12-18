#pragma once

//  标准库组件
#include <iterator>
#include <functional>
#include <string>
#include <unordered_map>
#include <format>

//  boost 库组件
#include <boost/asio.hpp>
//  MiraiCP 依赖
#include <MiraiCP.hpp>

//	项目内头文件
#include <detailed.h>

using namespace MiraiCP;

namespace function {

void send_server_status(GroupMessageEvent event) {

    //  获取服务器的当前信息
    auto server_status_expected = detailed::get_server_status();

    //  如果连接失败
    if (server_status_expected) {
        //  组合含有错误信息的字符串
		auto error_struct = server_status_expected.error();
        auto error_message = std::format("服务器连接失败! 错误类型: {}, 错误码: {}, 错误码信息: {}", 
                                error_struct.error_message, error_struct.error_code.value(), error_struct.error_code.message());

        //  将错误消息发送至 QQ 群并终止函数执行
        event.group.sendMessage(error_message);
		Logger::logger.error(error_message);
        return;
    }

    //  TODO：将解析后的消息发送到群组内
}

//  目前支持的指令列表
const std::unordered_map<std::string, std::function<void(GroupMessageEvent)>> commands_list{
    {".server main", send_server_status}
};

void command(GroupMessageEvent event) {

    constexpr auto QQGroup_number = QQID{884013901};

    //  如果加入事件并非来源于指定群，那么直接返回，不做任何事。
    if (event.group.groupId() != QQGroup_number) {
        Logger::logger.info(std::format("群事件不来自于指定群: {}", QQGroup_number));
        return;
    }

    auto command = event.message.toMiraiCode();

    auto called_command = commands_list.find(command);
    if (called_command != std::end(commands_list)) {
        called_command->second(event);
    } else {
        Logger::logger.info("未检测到有效指令");
    }
}

}   //  end of namespace function