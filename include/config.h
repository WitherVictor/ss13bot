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
struct data {
public:
    //  Key 指代服务器的名字, Value 为服务器的 IP
    using server_data = std::map<std::string, std::string>;
private:
    data() = delete;

    data(const nlohmann::json& config_json);

    data(const data&) = delete;
    data& operator=(const data&) = delete;

    data(data&&) = delete;
    data& operator=(data&&) = delete;

    ~data() = default;
public:
    friend const data& get();
public:
    server_data server_map;
    std::filesystem::path poly_path;
};  //  end of class config::data

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
nlohmann::json read();

}   //  end of namespace config