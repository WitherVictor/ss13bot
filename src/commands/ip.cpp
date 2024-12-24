#include <commands/ip.h>

//  MiraiCP
#include <MiraiCP.hpp>

namespace command {

using namespace MiraiCP;

void ip(GroupMessageEvent event, const std::string& full_server_ip) {
    event.group.sendMessage(full_server_ip);
}

}