#include <detailed.h>

//  标准库
#include <string>
#include <chrono>
#include <ranges>
#include <map>

//  Boost 库
#include <boost/asio.hpp>

//  MiraiCP
#include <MiraiCP.hpp>

namespace function {

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

    //  获取时间字符串代表的小时，分钟，和秒
    std::chrono::seconds seconds{ std::stoull(time_string) };
    std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(seconds);

    //  解析成可读模式，例：xx:xx:xx
    return std::format("{:0>2}:{:0>2}:{:0>2}",
            hours.count(),
            hours.count() == 0 ? minutes.count() : std::chrono::duration_cast<std::chrono::minutes>(minutes - hours).count(),
            minutes.count() == 0 ? seconds.count() : std::chrono::duration_cast<std::chrono::seconds>(seconds - minutes).count()
           );
}

/**
 * @brief 解析服务器返回的带有服务器运行信息的数据字符串
 * @return 返回 server_status 的装有信息的结构体
 */
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
    server_status data_struct{};
    data_struct.status = get_server_running_status(server_data_map.at("gamestate"));
    data_struct.round_id = server_data_map.at("round_id");
    data_struct.time = calculate_time(server_data_map.at("round_duration"));
    return data_struct;
}

/**
 * @brief 获取当前服务器的信息，可能失败
 * @return 成功返回服务器的状态，失败则返回错误信息
 */
tl::expected<server_status, error_info> get_server_status() {

    using namespace MiraiCP;

    //  初始化 io_context 和 socket
    boost::asio::io_context context{};
    boost::asio::ip::tcp::socket socket{ context };

    //  设置 endpoint 为服务器的 ip 地址
    boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address::from_string("43.248.187.124"), 43319};

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

    Logger::logger.info("服务器数据请求已发送, 数据包字节数: {}", bytes_sent);

    //  接收服务器发回的服务器数据
    constexpr std::size_t buffer_size = 4096;
    std::array<char, buffer_size> data_buffer{};

    auto bytes_read = socket.read_some(boost::asio::buffer(data_buffer), transform_error_code);

    if (transform_error_code && transform_error_code != boost::asio::error::eof) {
        socket.close();
		return tl::unexpected(error_info{transform_error_code, "数据接收失败!"});
    }

    Logger::logger.info("已接收服务器发送的数据包, 接收字节数: {}", bytes_read);
    socket.close();

    //  如果数据包的头两个 byte 与约定的相同
    if (data_buffer.at(0) == '\x00' && data_buffer.at(1) == '\x83')
    {
        Logger::logger.info("数据包已验证通过, 正在解析数据");

        //  服务器数据的起始位置
        constexpr std::size_t data_index_begin = 5;

        //  将缓冲区的数据复制到字符串内
        std::string server_data_string{};
        std::copy(std::next(std::begin(data_buffer), data_index_begin),
                        std::end(data_buffer),
                        std::back_inserter(server_data_string)
        );

        Logger::logger.info("解析的服务器数据字符串为: {}", server_data_string);
        Logger::logger.info("数据长度: {}", server_data_string.size());
        
        //  解析字符串并保存至结构体
        auto server_status_struct = parse_data_string(server_data_string);
        return server_status_struct;
    } else {
		auto error_code = boost::system::errc::make_error_code(boost::system::errc::protocol_error);
        return tl::unexpected(error_info{error_code, "数据包验证失败!"});
    }
}

}

}