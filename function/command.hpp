#ifndef __FUNCTION_COMMAND_HPP
#define __FUNCTION_COMMAND_HPP

#include <MiraiCP.hpp>              //	Core header
#include <bits/chrono.h>
#include <chrono>
#include <exception>
#include <ranges>                   //	std::views
#include <vector>                   //	std::vector
#include <json.hpp>                 //	json
#include <fstream>                  //  std::ifstream
#include <string>                   //  std::string
#include <random>                   //	std::mt19937_64

#include "get_server_status.hpp"    //	inline std::string get_server_status(const string& full_address)


using namespace MiraiCP;
using json = nlohmann::json;

namespace function
{

    static const std::string config_path{ ".\\data\\tech.eritquearcus.miraicp\\Poly\\config.json" };
    static json config_json = json::parse( std::ifstream{ config_path } );

	static json poly_json = json::parse(std::ifstream{config_json["poly_json"].get<std::string>()});
	static json group_message_json = json::parse( std::ifstream{ config_json["group_message_json"].get<std::string>()});
	static std::vector<std::string> poly_buffer{};

	//	静态随机数生成
	static std::random_device rd{};
	static std::mt19937_64 engine{rd()};

	inline void initial_poly()
	{
		//	poly buffer 是 static 的, 有可能其中有数据
		poly_buffer.clear();

		//	从尾部开始逐个插入数据
		if (!poly_json["phrases"].empty())
		{
            //  由于 Pull Request https://github.com/Fluffy-Frontier/FluffySTG/pull/3326
            //  更改了 Poly 的文字存储格式, 所以读入方式也需要更改, 使用迭代器访问对象的 Key
            //  不知道出于什么原因, 对象的读取顺序是逆序的, 所以这里选择 reverse_iterator
            for (auto iter = poly_json.at("phrases").rbegin(); iter != poly_json.at("phrases").rend(); iter++)
                poly_buffer.push_back(iter.key());      //  向缓冲区存入当前 Key, Key 的类型已知均为字符串
        }

		if (!group_message_json["phrases"].empty())
			poly_buffer.insert(
				std::end(poly_buffer),
				std::begin(group_message_json["phrases"]),
				std::end(group_message_json["phrases"])
			);
	}

	inline void reload_poly_json()
	{
		//	重新读入 json 文件
		config_json = json::parse( std::ifstream{ config_path } );
		poly_json = json::parse(std::ifstream{config_json["poly_json"].get<std::string>()});

		//	将当前内存中 json 对象保存的内容写入 json 文件中
		std::ofstream{ config_json["group_message_json"] } << group_message_json;

		//	读取 json 文件内的所有内容(原有 + 更新的)
		group_message_json = json::parse( std::ifstream{ config_json["group_message_json"].get<std::string>()});

		initial_poly();
	}

	inline std::string get_format_time(const std::string seconds_string)
	{
		try
		{
			std::chrono::seconds seconds{ std::stoull(seconds_string) };
			std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
			std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(seconds);

			return std::format
					(
					"{:0>2}:{:0>2}:{:0>2}",
						hours.count(),
						hours.count() == 0 ? minutes.count() : std::chrono::duration_cast<std::chrono::minutes>(minutes - hours).count(),
						minutes.count() == 0 ? seconds.count() : std::chrono::duration_cast<std::chrono::seconds>(seconds - minutes).count()
					);
		}
		catch (std::exception& e)
		{
			Logger::logger.error(e.what());
			return "Plugin Error";
		}
	}

	inline std::string get_shuttle_time(std::string shuttle_time)
	{
		auto result_string = get_format_time(shuttle_time);
		return result_string.substr(result_string.find(':') + 1);
	}

	//	解析穿梭机状态的字符串
	inline std::string get_shuttle_data_string(std::map<std::string, std::string> server_data_map)
	{
		std::string shuttle_data_string{};
		std::string shuttle_time{};
		std::string shuttle_status = server_data_map["shuttle_mode"];

		if (server_data_map["shuttle_timer"] == "NON" || server_data_map["shuttle_timer"] == "")
			return "FIN 00:00";

		if (shuttle_status == "idle")
		{
			auto modified_time_string = std::to_string(std::stoull(server_data_map["shuttle_timer"]) * 60);
			return "IDL " + get_shuttle_time(modified_time_string);
		}

		shuttle_time = get_shuttle_time(server_data_map["shuttle_timer"]);
		
		if (shuttle_status == "igniting")
		{
			shuttle_data_string += "IGN ";
		}
		else if (shuttle_status == "recalled")
		{
			shuttle_data_string += "RCL ";
		}
		else if (shuttle_status == "called")
		{
			shuttle_data_string += "ETA ";
		}
		else if (shuttle_status == "docked")
		{
			shuttle_data_string += "ETD ";
		}
		else if (shuttle_status == "escape")
		{
			shuttle_status += "ESC ";
		}
		else if (shuttle_status == "stranded")
		{
			return "ERR --:--";
		}
		else if (shuttle_status == "endgame%3a+game+over")
		{	
			return "FIN 00:00";
		}
		else
		{
			return "NON";
		}

		return shuttle_data_string + shuttle_time;
		
	}

    inline void command(GroupMessageEvent event)
    {
        //  获得产生事件的消息
        auto event_message = event.message.toMiraiCode();

        //  若消息开头为 '.'
        if (event_message.front() == '.')
        {
            //  则该条信息是指令，抛弃该符号便于分割字符串
            event_message.erase(0, 1);

            //  如果这条消息不是一条单纯的 '.'
            if (!event_message.empty())
            {
                //  存储分割后的指令
                std::vector<std::string> command_buffer{};

                //  打印日志
                Logger::logger.info(std::format("Split command : {}", event_message));

                for (auto&& word : event_message | std::views::split(' '))
                    command_buffer.push_back( {word.begin(), word.end()} );

				if (command_buffer[0] == "help")
                {
					if (command_buffer[1] == "?")
                    {
                        event.group.sendMessage
                        (
                            MessageChain{ "====== .help 的指令参数 ======\n" } +
                            PlainText{ "- 如何加入游戏\n" } +
                            PlainText{ "- 寻路指南\n" }
                        );
                    }
					else if (command_buffer.size() == 2)
                    {
						if (command_buffer[1] == "如何加入游戏")
						{
							event.group.sendMessage(MiraiCode(event.group.uploadImg(config_json["Join_game_image"].get<std::string>()).toMiraiCode()));
						}
						else if (command_buffer[1] == "寻路指南")
						{
							event.group.sendMessage(MiraiCode(event.group.uploadImg(config_json["Find_way_image"].get<std::string>()).toMiraiCode()));
						}
					}
                }
				else if (command_buffer[0] == "poly")
				{
					if (command_buffer[1] == "reload")
					{
						reload_poly_json();
						event.group.sendMessage(std::format("Poly 缓存刷新成功! 已读取消息数: {}", poly_buffer.size()));
						return;
					}

					//	产生随机数 并通过下标返回内存中poly的语句
					std::uniform_int_distribution range {std::size_t{}, poly_buffer.size() - 1};

                    if (!poly_buffer.empty())
					    event.group.sendMessage(poly_buffer[range(engine)]);
                    else
                        event.group.sendMessage("Poly 存储的消息数量为 0 !");
				}
				else if (command_buffer[0] == "server")
				{
					if (command_buffer[1] == "main" || command_buffer[1] == "erp")
					{
						//	服务器信息格式
						struct
						{
							std::string status{ "N/A" };
							std::string round_id{ "N/A" };
							std::string time{ "N/A" };
							std::string map{ "N/A" };
							std::string security_level{ "N/A" };
							std::string online_players{ "N/A" };
							std::string online_admins{ "N/A" };
							std::string shuttle_status{ "N/A" };
						} server;
						
						std::string server_data_string{};

						if (command_buffer[1] == "main")
							server_data_string = get_server_status(config_json["IP address"].get<std::string>());
						else if (command_buffer[1] == "erp")
							server_data_string = get_server_status(config_json["ERP address"].get<std::string>());
						else
						{
							event.group.sendMessage("Error, 查询格式错误, 请检查输入格式");
							return;
						}
							
						if (server_data_string == "NO DATA")
							event.group.sendMessage("Error: 服务器未发送任何数据!");
						else if (server_data_string == "ERROR")
							event.group.sendMessage("Error: 插件内部运行错误, 请检查后台日志!");

						//	将存有服务器运行信息的字符串一一对应解析成键值对存入 map
						auto server_data_map = parse_server_string(server_data_string);

						//	服务器回合 ID
						server.round_id = server_data_map["round_id"];

						//	回合持续时间的字符串
						server.time = get_format_time(server_data_map["round_duration"]);

						//	穿梭机信息的字符串
						server.shuttle_status = get_shuttle_data_string(server_data_map);

						//	警报等级字符串，使首字母大写
						server.security_level = server_data_map["security_level"];
						server.security_level.front() = std::toupper(server.security_level.front());

						//	移出地图名字符串中的所有 '+'
						server.map = server_data_map["map_name"];
						while (server.map.find('+') != std::string::npos)
							server.map.replace(server.map.find('+'), 1, " ");

						//	解析服务器运行状态字符串
						if (server_data_map["gamestate"] == "0")
							server.status = "Initializing";
						else if (server_data_map["gamestate"] == "1")
							server.status = "Preparing";
						else if (server_data_map["gamestate"] == "3")
							server.status = "Running";
						else if (server_data_map["gamestate"] == "4")
							server.status = "Ending";
						else
							server.status = "Unknown";

						server.online_players = server_data_map["players"];
						server.online_admins = server_data_map["admins"];
						
						MessageChain server_status = MessageChain("====== 服务器状态 ======\n") +
													 PlainText("运行状态: " + server.status + "\n") +
													 PlainText("回合ID: " + server.round_id + "\n") +
													 PlainText("回合时长: " + server.time + "\n") +
													 PlainText("当前地图: " + server.map + "\n") +
													 PlainText("警报等级: " + server.security_level + "\n") +
													 PlainText("在线人数: " + server.online_players + "\n") + 
													 PlainText("在线管理: " + server.online_admins + "\n") +
													 PlainText("撤离穿梭机状态: " + server.shuttle_status);
													 
						event.group.sendMessage( server_status );
					}
					else if (command_buffer[1] == "IP" || command_buffer[1] == "ip")
					{
						event.group.sendMessage
						(
							MessageChain("服务器地址: ") +
							PlainText(config_json["IP address"].get<std::string>())
						);
					}
					else if (command_buffer[1] == "log")
					{
						event.group.sendMessage
						(
							MessageChain("更新日志列表：") + 
                            PlainText("https://spacestation13cn.miraheze.org/wiki/%E6%9C%8D%E5%8A%A1%E5%99%A8Log%E5%88%97%E8%A1%A8")
						);
					}
            	}
				else if (command_buffer[0] == "plugin")
				{
					if (command_buffer[1] == "version")
						event.group.sendMessage(std::string{"插件版本: "} + CPPPlugin::config.version);
				}
			}
        }
    }   //  void command() end
}   //  namespace function

#endif //	ifndef macro end