#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>

//  标准库
#include <format>
#include <iterator>
#include <string>
#include <chrono>
#include <ranges>
#include <map>

//  Boost 库
#include <boost/asio.hpp>

//  MiraiCP
#include <MiraiCP.hpp>
#include <detailed.h>

namespace command {

namespace detailed {

std::string get_server_running_status(const std::string& current_status) {

    //	解析服务器运行状态字符串
    if (current_status == "0")
        return "Initializing";
    else if (current_status == "1")
        return "Preparing";
    else if (current_status == "3")
        return "Running";
    else if (current_status == "4")
        return "Ending";
    else
        return "Unknown";
}



std::string calculate_time(const std::string& time_string) {

    std::size_t seconds_value{};

    std::from_chars(time_string.data(), time_string.data() + time_string.size(), seconds_value);

    //  获取时间字符串代表的小时，分钟，和秒
    std::chrono::seconds seconds{ seconds_value };
    std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(seconds);

    //  解析成可读模式，例：xx:xx:xx
    return std::format("{:0>2}:{:0>2}:{:0>2}",
            hours.count(),
            hours.count() == 0 ? minutes.count() : std::chrono::duration_cast<std::chrono::minutes>(minutes - hours).count(),
            minutes.count() == 0 ? seconds.count() : std::chrono::duration_cast<std::chrono::seconds>(seconds - minutes).count()
           );
}



server_status parse_data_string(const std::string& server_data_string) {

    //  将数据字符串分段
    auto server_data_view = server_data_string
							  | std::views::split('&')
							  | std::views::transform( [](auto word){ return std::string(word.begin(), word.end()); } );

    //  可用 C++23 的 std::ranges::to
    //  将每一段数据转换成字符串
    std::vector<std::string> server_data_vector(server_data_view.begin(), server_data_view.end());

    //  用来存储键值映射关系的 map
    std::map<std::string, std::string> server_data_map{};
    for (auto single_data : server_data_vector) {

        //  如果找不到等号，则不是需要的信息，直接丢弃
        if (single_data.find('=') == std::string::npos) {
            continue;
        }

        auto data_key = single_data.substr(0, single_data.find('='));
        std::string data_value{};

        //  如果最后一个字符不是 '=', 即存在 data_value
        if (single_data.find('=') + 1 != single_data.size())
            data_value = single_data.substr(single_data.find('=') + 1);

        //  将当前键值对保存到 map 内
        server_data_map[data_key] = data_value;
    }

    //  将服务器信息放入结构体内
    return extract_server_info(server_data_map);
}



tl::expected<server_status, error_info> query_server_status() {

    using namespace MiraiCP;

    //  初始化 io_context 和 socket
    boost::asio::io_context context{};
    boost::asio::ip::tcp::socket socket{ context };

    //  设置 endpoint 为服务器的 ip 地址
    boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::make_address("43.248.187.124"), 43319};

    //  连接游戏服务器
    boost::system::error_code socket_error_code{};
    auto _ = socket.connect(endpoint, socket_error_code);

    //  如果连接失败，则发出错误消息并直接返回
    if (socket_error_code) {
        socket.close();
        return tl::unexpected(error_info{socket_error_code, "Socket 连接失败!"});
    }

    //  准备向服务器发起查询请求的数据包
    const std::string packet{ "\x00\x83\x00\x0d\x00\x00\x00\x00\x00?status\x00",
                        sizeof("\x00\x83\x00\x0d\x00\x00\x00\x00\x00?status\x00") - 1};

    //  向服务器发送数据
    boost::system::error_code transform_error_code{};
    auto bytes_sent = boost::asio::write(socket, boost::asio::buffer(packet), transform_error_code);

    //  如果发送失败，则发出错误消息并直接返回
    if (transform_error_code) {
        socket.close();
		return tl::unexpected(error_info{transform_error_code, "数据发送失败!"});
    }

    Logger::logger.info("服务器数据请求已发送, 数据包字节数: ", bytes_sent);

    //  接收服务器发回的服务器数据, 默认 8M
    constexpr std::size_t buffer_size = 8192;
    std::array<char, buffer_size> data_buffer{};

    auto bytes_read = socket.read_some(boost::asio::buffer(data_buffer), transform_error_code);

    if (transform_error_code && transform_error_code != boost::asio::error::eof) {
        socket.close();
		return tl::unexpected(error_info{transform_error_code, "数据接收失败!"});
    }

    Logger::logger.info("已接收服务器发送的数据包, 接收字节数: ", bytes_read);
    socket.close();

    //  如果数据包的头两个 byte 与约定的相同
    if (data_buffer.at(0) == '\x00' && data_buffer.at(1) == '\x83')
    {
        Logger::logger.info("数据包已验证通过, 正在解析数据");

        //  服务器数据的起始位置
        constexpr std::size_t data_index_begin = 5;

        //  TODO: 复制数据可能导致额外的空字节被复制了，有待测试
        //  将缓冲区的数据复制到字符串内
        std::string server_data_string{};
        std::ranges::copy(data_buffer | std::views::drop(data_index_begin),
                        std::back_inserter(server_data_string));

        Logger::logger.info("解析的服务器数据字符串为: ", server_data_string);
        Logger::logger.info("数据长度: ", server_data_string.size());
        
        //  解析字符串并保存至结构体
        return detailed::parse_data_string(server_data_string);
    } else {
		auto error_code = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
        return tl::unexpected(error_info{error_code, "数据包验证失败!"});
    }
}



server_status extract_server_info(const std::map<std::string, std::string>& server_info_map) {
    server_status server_info_struct{
        .gamestate = detailed::get_server_running_status(server_info_map.at("gamestate")),
        .round_id = server_info_map.at("round_id"),
        .round_duration = detailed::calculate_time(server_info_map.at("round_duration")),
        .time_dilation = detailed::parse_server_dilation(server_info_map),
        .map_name = detailed::parse_server_map(server_info_map.at("map_name")),
        .security_level = server_info_map.at("security_level"),
        .players = server_info_map.at("players"),
        .shuttle_status = detailed::get_shuttle_status(server_info_map)
    };

    //  将警报等级的第一个字符设为大写
    auto& front_char = server_info_struct.security_level.front();
    front_char = std::toupper(static_cast<unsigned char>(front_char));
    return server_info_struct;
}



double td_string_to_double(const std::string& time_dilation_string) {
    auto size = static_cast<std::string::iterator::difference_type>(time_dilation_string.size());

    double result{};
    std::from_chars(time_dilation_string.data(), std::next(time_dilation_string.data(), size), result);
    return result;
}



std::string parse_server_dilation(const std::map<std::string, std::string> &server_info_map) {
    //  提取所有字符串
    auto td_current = server_info_map.at("time_dilation_current");
    auto td_average = server_info_map.at("time_dilation_avg");
    auto td_average_slow = server_info_map.at("time_dilation_avg_slow");
    auto td_average_fast = server_info_map.at("time_dilation_avg_fast");

    //  保存转换后的结果
    auto td_current_value = detailed::td_string_to_double(td_current);
    auto td_average_value = detailed::td_string_to_double(td_average);
    auto td_average_slow_value = detailed::td_string_to_double(td_average_slow);
    auto td_average_fast_value = detailed::td_string_to_double(td_average_fast);

    return std::format("{:.2f}% ({:.2f}%, {:.2f}%, {:.2f}%)", 
                            td_current_value,
                            td_average_value,
                            td_average_slow_value,
                            td_average_fast_value);
}



std::string parse_server_map(std::string server_map) {

    //  将连接符 + 替换成空格
    detailed::replace_substring(server_map, "+", " ");

    //  将 URL 编码字符转换成实际符号
    //  因为需要转换的字符较少因此直接替换
    detailed::replace_substring(server_map, "%28", "(");
    detailed::replace_substring(server_map, "%29", ")");

    return server_map;
}



void replace_substring(std::string &str, const std::string &old_substr, const std::string &new_substr) {
    size_t pos = 0;
    while ((pos = str.find(old_substr, pos)) != std::string::npos) {
        str.replace(pos, old_substr.length(), new_substr);
        pos += new_substr.length();  // 确保继续查找下一个匹配
    }
}



std::string get_shuttle_status(const std::map<std::string, std::string>& server_info_map) {
    auto shuttle_status = detailed::parse_shuttle_status(server_info_map.at("shuttle_mode"));
    
    auto shuttle_time_string = server_info_map.at("shuttle_timer");
    std::string shuttle_time{};

    if (shuttle_status == "ERR") {  //  如果穿梭机被禁用，那么与游戏内一致显示为 "--:--"
        shuttle_time = "--:--";
    } else if (shuttle_status == "FIN" || shuttle_status == "NON") {    //  如果已经结束或状态未知，那么置为 0
        shuttle_time = "00:00";
    } else if (shuttle_status == "IDL") {   //  如果穿梭机闲置，那么 shuttle_time_string 的单位是分钟，直接显示
        shuttle_time = std::format("{:0>2}:00", shuttle_time_string);
    } else if (shuttle_time_string == "") { //  如果时间字符串为空，那么直接置为 0。与上一段以 shuttle_status 为判断条件分开
        shuttle_time = "00:00";
    } else {    //  否则会返回一个秒数，那么直接计算秒数即可
        shuttle_time = detailed::get_shuttle_time(shuttle_time_string);
    }

    //  返回与游戏内格式相同的压缩后穿梭机状态字符串
    return std::format("{} {}", shuttle_status, shuttle_time);
}



std::string parse_shuttle_status(const std::string &shuttle_status) {
    if (shuttle_status == "idle") {
        return "IDL";
    } else if (shuttle_status == "igniting") {
        return "IGN";
    } else if (shuttle_status == "recalled") {
        return "RCL";
    } else if (shuttle_status == "called") {
        return "ETA";
    } else if (shuttle_status == "docked") {
        return "ETD";
    } else if (shuttle_status == "escape") {
        return "ESC";
    } else if (shuttle_status == "stranded") {
        return "ERR";
    } else if (shuttle_status == "endgame%3a+game+over") {  //  判断条件就长这样，搞不懂 - -||
        return "FIN";
    } else {
        return "NON";
    }
}



std::string get_shuttle_time(const std::string &shuttle_time) {
    //  转发调用计算时间
    auto result = detailed::calculate_time(shuttle_time);

    //  抛弃小时上的结果，只返回分钟和秒
    return result.substr(result.find(":") + 1);
}

}   // end of namespace detailed

}   // end of namespace plugin