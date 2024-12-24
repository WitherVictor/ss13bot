#pragma once

//  标准库
#include <filesystem>

//  json
#include <json.hpp>

namespace config {

namespace fs = std::filesystem;

/**
 * @brief 保存配置文件被解析后的数据
 * 
 */
struct data;

/**
 * @brief 获取解析后的配置文件数据
 * 
 * @return const data& 
 */
const data& get();

/**
 * @brief 读取当前目录下的配置文件
 * 
 * @param current_path 
 * @return nlohmann::json
 */
nlohmann::json read(fs::path current_path = fs::current_path());

}   //  end of namespace config