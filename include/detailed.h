#pragma once

//  标准库
#include <string>

//  Boost 库
#include <boost/system/error_code.hpp>

//  三方库
#include <tl/expected.hpp>

namespace function {

//  表示服务器信息的结构体
struct server_status {
    std::string status{};
    std::string round_id{};
    std::string time{};
    std::string time_dilation{};
    std::string map{};
    std::string security_level{};
    std::string online_players{};
    std::string shuttle_status{};
};

//	表示错误的结构体
struct error_info {
	boost::system::error_code error_code{};
	std::string error_message{};	
};

namespace detailed {

/**
 * @brief 将字符串的值转换为指代服务器运行状态的字符串
 * 
 * @param current_status 
 * @return std::string 
 */
std::string get_server_running_status(const std::string& current_status);

/**
 * @brief 将字符串的值转为 {}:{}:{} 格式的时间
 * 
 * @param time_string 
 * @return std::string 
 */
std::string calculate_time(const std::string& time_string);

/**
 * @brief 解析服务器返回的带有服务器运行信息的数据字符串
 *
 * @param server_data_string
 * @return server_status
 */
server_status parse_data_string(const std::string& server_data_string);

/**
 * @brief 获取当前服务器的信息，可能失败
 *
 * @return tl::expected<server_status, error_info>
 */
tl::expected<server_status, error_info> get_server_status();

}   //  end of namespace detailed

}   //  end of namespace function