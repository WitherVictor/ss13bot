// MiraiCP依赖文件(只需要引入这一个)
#include <MiraiCP.hpp>

//  机器人项目引入的头文件
#include "../../function/command.hpp"
#include "../../function/member_join.hpp"
#include "../../function/read_group_message.hpp"
//  #include "../../function/member_leave.hpp"

using namespace MiraiCP;

const PluginConfig CPPPlugin::config {
        "19990507",             // 插件id，如果和其他插件id重复将会被拒绝加载！
        "SS13Bot",        // 插件名称
        "2.0.0",            // 插件版本
        "WitherVictor",        // 插件作者
        "SS13国服查询用机器人"  // 可选：插件描述
        //  "Publish time"        // 可选：日期
};

// 插件实例
class PluginMain : public CPPPlugin {
public:
  // 配置插件信息
  PluginMain() : CPPPlugin() {}
  ~PluginMain() override = default; // override关键字是为了防止内存泄漏

  // 入口函数。插件初始化时会被调用一次，请在此处注册监听
  void onEnable() override {
    /*插件启动时执行一次*/
    /*插件启动时执行一次*/

		//	对 poly 的文件进行初始化
        function::initial_poly();

        //	注册指令函数
        Event::registerEvent<GroupMessageEvent>(function::command);

        //	注册成员进入函数
        Event::registerEvent<MemberJoinEvent>(function::member_join);

        //  注册成员消息读取函数
        Event::registerEvent<GroupMessageEvent>(function::read_group_message);

        //	注册成员退出函数
        //	Event::registerEvent<MemberLeaveEvent>(function::member_leave);
    }

  // 退出函数。请在这里结束掉所有子线程，否则可能会导致程序崩溃
  void onDisable() override {
    /*插件结束前执行*/
    
    std::ofstream{ function::config_json["group_message_json"] } << function::group_message_json;
  }
};

// 创建当前插件实例。请不要进行其他操作，
// 初始化请在onEnable中进行
void MiraiCP::enrollPlugin() {
  MiraiCP::enrollPlugin<PluginMain>();
}