#pragma once

//  标准库
#include <string>
#include <map>
#include <random>
#include <algorithm>

//  Boost 库
#include <boost/system/error_code.hpp>

//  三方库
#include <tl/expected.hpp>
#include <type_traits>

namespace command {

//  表示服务器信息的结构体
struct server_status {
    std::string gamestate{};
    std::string round_id{};
    std::string round_duration{};
    std::string time_dilation{};
    std::string map_name{};
    std::string security_level{};
    std::string players{};
    std::string shuttle_status{};
};

//	表示错误的结构体
struct error_info {
	boost::system::error_code error_code{};
	std::string error_message{};	
};

namespace detailed {

template <std::integral Tp>
class rng {
public:
    rng(Tp begin, Tp end) 
        : range{begin, end}
    {
        if (begin >= end) {
            throw std::invalid_argument{"Range begin should smaller than end in the constructor of class rng!"};
        }

        //  使初始种子更加随机化
        std::random_device rd{};
        std::array<Tp, std::mt19937_64::state_size> seed_data{};
        std::ranges::generate(seed_data, std::ref(rd));
        std::seed_seq seed_sequence(std::begin(seed_data), std::end(seed_data));

        engine = std::mt19937_64{seed_sequence};
    }

    Tp yield() { return range(engine); }
private:
    std::uniform_int_distribution<Tp> range;
    std::mt19937_64 engine;
};

template <typename Tp, typename Up>
rng(Tp, Up) -> rng<std::common_type_t<Tp, Up>>;

/**
 * @brief 将字符串的值转换为指代服务器运行状态的字符串
 * 
 * @param current_status 
 * @return std::string 
 */
std::string get_server_running_status(const std::string&);

/**
 * @brief 将字符串的值转为 {}:{}:{} 格式的时间
 * 
 * @param time_string 
 * @return std::string 
 */
std::string calculate_time(const std::string&);

/**
 * @brief 解析服务器返回的带有服务器运行信息的数据字符串
 *
 * @param server_data_string
 * @return server_status
 */
server_status parse_data_string(const std::string&);

/**
 * @brief 获取当前服务器的信息，可能失败
 *
 * @return tl::expected<server_status, error_info>
 */
tl::expected<server_status, error_info> query_server_status(const std::string&);

/**
 * @brief 将分割的数据提取到结构体内
 * 
 * @param server_info_map
 * @return server_status
 */
server_status extract_server_info(const std::map<std::string, std::string>& server_info_map);

/**
 * @brief 将多个 Time Dilation 数据合并成一个字符串
 * 
 * @param server_info_map 
 * @return std::string 
 */
std::string parse_server_dilation(const std::map<std::string, std::string>& server_info_map);

/**
 * @brief 将 Time Dilation 的值类型从字符串转换为浮点类型
 * 
 * @param time_dilation_string 
 * @return std::optional<double> 
 */
double td_string_to_double(const std::string& time_dilation_string);

/**
 * @brief 替换字符串内的连接符使其更可读
 * 
 * @param server_map 
 * @return std::string 
 */
std::string parse_server_map(std::string server_map);

/**
 * @brief 替换字符串内的子串为预期子串
 * 
 * @param str 
 * @param old_substr 
 * @param new_substr 
 */
void replace_substring(std::string &str, const std::string &old_substr, const std::string &new_substr);

/**
 * @brief 压缩紧急穿梭机状态为简化表示
 * 
 * @param server_info_map 
 * @return std::string 
 */
std::string get_shuttle_status(const std::map<std::string, std::string>& server_info_map);

/**
 * @brief 获取紧急穿梭机状态字符串的缩写
 * 
 * @param shuttle_status 
 * @return std::string 
 */
std::string parse_shuttle_status(const std::string& shuttle_status);

/**
 * @brief 获取紧急穿梭机穿梭时间的值
 * 
 * @param shuttle_time 
 * @return std::string 
 */
std::string get_shuttle_time(const std::string& shuttle_time);

}   //  end of namespace detailed

}   //  end of namespace plugin