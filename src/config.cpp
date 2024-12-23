#include <config.h>

//  标准库
#include <exception>
#include <filesystem>
#include <fstream>

//  MiraiCP
#include <MiraiCP.hpp>
#include <json.hpp>

namespace config {

namespace fs = std::filesystem;
using namespace MiraiCP;

void initialize() {
    //  读取配置文件
    auto config_json = read();
}

//  若未提供参数，那么默认为 fs::current_path()
nlohmann::json read(fs::path current_path) {
    current_path += "/config.json";

    //  对于找不到配置文件的情况
    //  我们无法预期使用者将在哪个服务器上进行查询
    //  因此除了直接终止程序, 我们没有更好的选择
    if (!fs::exists(current_path)) {
        Logger::logger.error("配置文件不存在!");
        std::terminate();
    }

    return nlohmann::json::parse( std::ifstream(current_path) );
}

}   //  end of namespace config