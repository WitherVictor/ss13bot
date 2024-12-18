// MiraiCP依赖文件(只需要引入这一个)
#include <MiraiCP.hpp>

//  SS13bot begin
#include <welcome.h>

using namespace MiraiCP;

const PluginConfig CPPPlugin::config{
    "19990507",                       // 插件id，如果和其他插件id重复将会被拒绝加载！
    "SS13Bot",                      // 插件名称
    "3.0.0",                     // 插件版本
    "WitherVictor",               // 插件作者
    "SS13 国服查询用机器人",   // 可选：插件描述
    "2024.12.18"                    // 可选：日期
};

// 插件实例
class PluginMain : public CPPPlugin {
public:
    // 配置插件信息
    PluginMain() : CPPPlugin() {}
    ~PluginMain() override = default; // override关键字是为了防止内存泄漏

    // 入口函数。插件初始化时会被调用一次，请在此处注册监听
    void onEnable() override {

        //	注册成员加入函数, 在新成员加入时发送欢迎消息
        Event::registerEvent<MemberJoinEvent>(function::welcome);
    }

    // 退出函数。请在这里结束掉所有子线程，否则可能会导致程序崩溃
    void onDisable() override {}
};

// 创建当前插件实例。请不要进行其他操作，
// 初始化请在onEnable中进行
void MiraiCP::enrollPlugin() { MiraiCP::enrollPlugin<PluginMain>(); }