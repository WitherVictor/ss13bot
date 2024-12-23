#include <config.h>

//  标准库
#include <exception>
#include <filesystem>
#include <fstream>

//  MiraiCP
#include <MiraiCP.hpp>
#include <json.hpp>
#include <optional>

namespace config {

namespace fs = std::filesystem;
using namespace MiraiCP;

class server_list {
public:
    using data_type = std::map<std::string, std::string>;
public:
    server_list(data_type&& other)
        : data_list{other} {}
public:
    /**
     * @brief 查找服务器列表里面是否存在对应的名字, 否则返回空值
     * 
     * @param key 
     * @return std::optional<std::string> 
     */
    std::optional<std::string> find(const std::string& key) const {
        auto find_result_iter = data_list.find(key);

        if (find_result_iter != data_list.end()) {
            return find_result_iter->second;
        } else {
            return std::nullopt;
        }
    }
private:
    data_type data_list;
};

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