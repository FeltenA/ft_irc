#ifndef SERVER_HPP
#define SERVER_HPP


#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>


#include "Channel.hpp"
#include "User.hpp"


#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define BLUE        "\033[34m"
#define WHITE       "\033[37m"
#define COLOR_END   "\033[0m"


class Channel;


class Server
{
    public:
        Server();
        ~Server();
        void            set_port(std::string port);
        void            set_password(std::string password);
        void            remove_user_from_channel(User *user, Channel *channel);
        void            add_user_to_server(User *user);
        int             configure_server();
        void            accept_data();
        void            receive_data(int i);
        int             identify_command(std::string buffer, User *current_user);
        void            add_new_channel(std::string channel_name);
        bool            nickname_is_available(std::string nickname);
        bool            username_is_available(std::string username);
        bool            check_connexion_input(int user_fd, std::string name_request, std::string &name_input, std::string invalid_name, char type);
        Channel         *get_channel_ptr(std::string channel_name);
        bool            user_not_is_the_channel(User *user, Channel *channel);
        User            *get_user_ptr_from_fd(int id);
        void            password_autentification(int user_fd);
        void            nickname_and_username_creation(int user_fd);
        void            send_message(std::string message_to_send, int user_fd);
        int             poll_function();
        void            send_command_from_multimap(std::string received_message, int sender_fd);

        //cmd
        void            join_cmd(std::string channel_name, std::istringstream &iss, User *current_user);
        void            privmsg_cmd(std::string buffer_content, std::istringstream &iss, User *current_user);
        void            pong_cmd(std::istringstream &iss, User *current_user);
        void            nick_cmd(std::istringstream &iss, User *current_user);
        void            user_cmd(std::istringstream &iss, User *current_user);
        void            pass_cmd(std::string buffer_content, User *current_user);
        
        //for tests
        void            print_all_channels_with_all_users();
        void            print_all_users();


    private:
        std::vector<User *>                 users;
        std::vector<Channel *>              channels;
        std::vector<struct pollfd>          *fds_poll;
        int                                 connected_fds;
        std::string                         server_name; //has to have one, 63 char max
        int                                 port;
        std::string                         password;
        int                                 server_socket_fd;
        std::multimap<std::string, int>     messages;
        std::map<int, std::string>          buffers;
};

#endif
