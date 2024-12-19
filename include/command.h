#pragma once

//  标准库组件
#include <functional>
#include <string>
#include <unordered_map>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

namespace function {

using namespace MiraiCP;

extern const std::unordered_map<std::string, std::function<void(GroupMessageEvent)>> commands_list;

void command(GroupMessageEvent event);

}   //  end of namespace function