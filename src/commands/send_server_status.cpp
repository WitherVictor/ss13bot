//  标准库组件
#include <format>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

//	项目内头文件
#include <commands/send_server_status.h>
#include <detailed.h>

namespace function {

using namespace MiraiCP;

void send_server_status(GroupMessageEvent event) {

    //  获取服务器的当前信息
    auto server_status_expected = detailed::get_server_status();

    //  如果连接失败
    if (!server_status_expected) {
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
    auto server_info = server_status_expected.value();
}

}   //  end of namespace function