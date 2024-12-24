#pragma once

//  标准库组件
#include <functional>
#include <string>
#include <unordered_map>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

namespace command {

using namespace MiraiCP;

extern const std::unordered_map<std::string, std::function<void(GroupMessageEvent)>> commands_list;

void main(GroupMessageEvent event);

}   //  end of namespace plugin