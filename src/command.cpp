//  标准库组件
#include <iterator>
#include <functional>
#include <string>
#include <unordered_map>

//  boost 库组件
#include <boost/asio.hpp>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

//	项目内头文件
#include <command.h>
#include <commands/send_server_status.h>
#include <detailed.h>

using namespace MiraiCP;

namespace command {

//  目前支持的指令列表
const std::unordered_map<std::string, std::function<void(GroupMessageEvent)>> commands_list{
    {".server main", send_server_status}
};

void main(GroupMessageEvent event) {

    constexpr auto QQGroup_number = QQID{884013901};

    //  如果加入事件并非来源于指定群，那么直接返回，不做任何事。
    if (event.group.groupId() != QQGroup_number) {
        Logger::logger.info("群事件不来自于指定群: ", QQGroup_number);
        return;
    }

    auto raw_command = event.message.toMiraiCode();

    for (const auto& [command, function] : commands_list) {
        if (raw_command.starts_with(command)) {
            Logger::logger.info("检测到有效指令: ", command);
            function(event);
        }
    }
}

}   //  end of namespace plugin