#ifndef __FUNCTION_GET_SERVER_STATUS_HPP
#define __FUNCTION_GET_SERVER_STATUS_HPP

#include <MiraiCP.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio.hpp>

#include <boost/asio/ip/basic_endpoint.hpp>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <map>
#include <ranges>

using namespace MiraiCP;

namespace function
{
    constexpr std::size_t array_buffer_size{ 1024 };

    inline std::string get_server_status(const std::string& full_address) {
        try {
            std::string addr = full_address.substr(0, full_address.find(':'));
            const unsigned short port = std::stoull(full_address.substr(full_address.find(':') + 1));
            
            Logger::logger.info(std::format("Parsed addr: {}", addr));
            Logger::logger.info(std::format("Parsed port: {}", port));

            boost::asio::io_context context;
            boost::asio::ip::tcp::socket socket{ context };

            boost::asio::ip::tcp::endpoint endpoint{ boost::asio::ip::address::from_string(addr), port };
            socket.connect(endpoint);

            if (socket.is_open())
                Logger::logger.info("Socket Connect Success");
            else
                Logger::logger.warning("Socket Connect Failed !");

            // Prepare the packet to send to the server (based on a reverse-engineered packet structure)
            std::string packet{ "\x00\x83\x00\x0d\x00\x00\x00\x00\x00?status\x00", sizeof("\x00\x83\x00\x0d\x00\x00\x00\x00\x00?status\x00") - 1};

            auto packet_length = sizeof("\x00\x83\x00\x0d\x00\x00\x00\x00\x00?status\x00") - 1;
            std::size_t data_sent = 0;
            
            try
            {
                while (data_sent < packet_length)
                {
                    auto bytes_sent = socket.write_some(boost::asio::buffer(packet.substr(data_sent), packet_length - data_sent));

                    Logger::logger.info(std::format("Bytes sent: {}", bytes_sent));
                    data_sent += bytes_sent;
                }
            }
            catch (std::exception& e)
            {
                Logger::logger.error(e.what());
                return "ERROR";
            }

            Logger::logger.info("Packet have been sent");

            std::array<char, array_buffer_size> buff{};

            auto bytes_read = socket.read_some(boost::asio::buffer(buff));
            Logger::logger.info(std::format("Bytes read: {}", bytes_read));
            Logger::logger.info("Packet received");

            socket.close();
            Logger::logger.info(std::format("Socket closed"));

            //  如果数据包的头两个 byte 与约定的相同
            if (buff[0] == '\x00' && buff[1] == '\x83')
            {
                Logger::logger.info("Packet verified, reading data");

                std::size_t size = buff.size() - 5; 
                std::string result_string{""};
                result_string.reserve(size);

                for (int index{5}; size > 0; index++, size--)
                    result_string += buff[index];

                Logger::logger.info("Parsed server data string is: {}", result_string);
                return result_string;

            }
            else
                return "NO DATA";
        }       
        catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return "ERROR";
        }

        return "ERROR";
    }   //  inline void get_server_status(const std::string&) end

    inline std::map<std::string, std::string> parse_server_string(const std::string& server_data_string)
    {
        auto server_data_view = server_data_string
							  | std::views::split('&')
							  | std::views::transform( [](auto word){ return std::string(word.begin(), word.end()); } );

		std::vector<std::string> server_data_vector(server_data_view.begin(), server_data_view.end());

		std::map<std::string, std::string> server_data_map{};

		for (auto single_data : server_data_vector)
		{
            if (single_data.find('=') == std::string::npos)
                continue;
			auto data_key = single_data.substr(0, single_data.find('='));
            std::string data_value{};

            //  如果最后一个字符不是 '=', 即存在 data_value
            if (single_data.find('=') + 1 != single_data.size())
                data_value = single_data.substr(single_data.find('=') + 1);
			server_data_map[data_key] = data_value;
		}

        return server_data_map;
    }
}   //  namespace function end

#endif