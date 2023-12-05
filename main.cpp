#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Please use the following input: \"./ircserv <port_num> <password>\"" << std::endl;
        return 1;
    }

    Server server;
    server.set_port(argv[1]);
    server.set_password(argv[2]);

    if (server.configure_server() == -1)
    {
        std::cerr << RED << "Server: Error : server creation" << COLOR_END << std::endl;
        return 1;
    }
    return 0;
}
