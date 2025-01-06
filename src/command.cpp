//  标准库
#include <functional>

//  boost 库组件
#include <boost/asio.hpp>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

//  项目内头文件
#include <command.h>
#include <commands/send_server_status.h>
#include <commands/ip.h>
#include <commands/poly.h>
#include <detailed.h>

using namespace MiraiCP;

namespace command {

//  返回 data_map 单例对象
const data_map& list() {
    static data_map cmd_list = config::get();
    return cmd_list;
}

//  class data_map definition begin
data_map::data_map(const config::data& config_data) {
    using namespace std::placeholders;

    for (const auto& [server_name, server_ip] : config::get().server_map) {
        //  将查询服务器信息的指令注册到指令列表内
        auto server_info = std::format(".server {}", server_name);
        auto server_info_function = std::bind(send_server_status, _1, server_ip);
        command_list[server_info] = server_info_function;
        Logger::logger.info("已注册指令: ", server_info);

        //  将查询服务器 IP 的指令注册到指令列表内
        auto server_address = std::format(".ip {}", server_name);
        auto server_address_function = std::bind(ip, _1, server_ip);
        command_list[server_address] = server_address_function;
        Logger::logger.info("已注册指令: ", server_address);

        //  注册 .poly 指令
        command_list[".poly"] = command::send_poly_message;
    }
}
//  class data_map definition end

//  群消息的回调函数, 用于监听指令
void main_logic(GroupMessageEvent event) {

    constexpr auto QQGroup_number = QQID{884013901};

    //  如果加入事件并非来源于指定群，那么直接返回，不做任何事。
    if (event.group.groupId() != QQGroup_number) {
        Logger::logger.info("群事件不来自于指定群: ", QQGroup_number);
        return;
    }

    //  如果存在该指令, 则调用对应函数
    auto raw_command = event.message.toMiraiCode();
    for (const auto& [command, function] : command::list().command_list) {
        if (raw_command.starts_with(command)) {
            Logger::logger.info("检测到有效指令: ", command);
            function(event);
        }
    }
}

}   //  end of namespace plugin