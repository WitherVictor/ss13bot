#pragma once

//  标准库
#include <filesystem>

//  json
#include <json.hpp>

namespace config {

namespace fs = std::filesystem;

/**
 * @brief 插件启动之前执行初始化
 * 
 */
void initialize();

/**
 * @brief 读取当前目录下的配置文件
 * 
 * @param current_path 
 * @return nlohmann::json
 */
nlohmann::json read(fs::path current_path = fs::current_path());

}   //  end of namespace config