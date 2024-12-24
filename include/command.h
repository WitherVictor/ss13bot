#pragma once

//  标准库组件
#include <functional>
#include <string>

//  MiraiCP 依赖
#include <MiraiCP.hpp>

//  项目内头文件
#include <config.h>

namespace command {

using namespace MiraiCP;

struct data_map {
public:
    using command_list_type = std::map<std::string, std::function<void(GroupMessageEvent)>>;
private:
    data_map() = delete;

    data_map(const config::data&);

    data_map(const data_map&) = delete;
    data_map& operator=(const data_map&) = delete;

    data_map(data_map&&) = delete;
    data_map& operator=(data_map&&) = delete;

    ~data_map() = default;
public:
    friend const data_map& list();
public:
    command_list_type command_list;
};

void main(GroupMessageEvent event);

}   //  end of namespace plugin