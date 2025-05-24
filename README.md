# SS13bot

本项目是专用于[国服 Space Station 13](https://qm.qq.com/q/NdFaMsp0oU) 的查询用 QQ 机器人，附带其他的一些功能。

本项目使用 [MiraiCP](https://github.com/Nambers/MiraiCP), 即 [mirai](https://github.com/mamoe/mirai) 的 C++ SDK 开发, 在 mirai - [overflow](https://github.com/MrXiaoM/overflow) - [LLOneBot](https://llonebot.github.io/zh-CN/) 框架下运行。

## 使用须知

### 前置要求

本项目基于 Mirai 和 MiraiCP 开发, 在使用机器人之前须成功配置 Mirai 和 MiraiCP 后, 由 MiraiCP 的 `libloader.dll` 启动插件。

**注意**：插件支持向服务器传递报错信息，但 `json` 要求文本信息是合法的 UTF-8 文本，若本地报错信息存在不合法的 UTF-8 字符会导致崩溃。

### 项目配置

本机器人的运行依赖于`config.json`配置文件，该文件必须与编译的动态库`libMiraiCPPlugin.dll`处于同一目录下, 其使程序可运行的最小格式为
```json
{
    "poly_json" : "...",
    "group_message_json" : "...",
    "IP address" : "..."
}
```
- `poly_json`: 游戏内 Poly 所保存的消息文件路径
- `group_message_json`: 保存群内消息数据的文件路径
- `IP address`: 服务器的 IP 地址

缺少任意一项都会导致运行时库出错, 进而导致机器人启动失败
