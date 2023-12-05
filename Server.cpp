#include "Server.hpp"


Server::Server()
{
    this->connected_fds = 0;
    this->port = -1;
    this->server_name = "Server";
    this->password = "";
    this->server_socket_fd = -1;
    this->fds_poll = new std::vector<struct pollfd>(10);
}


Server::~Server()
{
    delete this->fds_poll;
    close(this->server_socket_fd);
}


void Server::set_port(std::string input_port)
{
    std::istringstream  iss(input_port);
    iss >> port;
    std::cout << GREEN << "Server: port set: " << port << COLOR_END << std::endl;
}


void Server::set_password(std::string input_password)
{
    password = input_password;
    std::cout << GREEN << "Server: password set" << COLOR_END << std::endl;
}


void Server::remove_user_from_channel(User *user, Channel *channel)
{
    channel->remove_user(user->get_nickname());
}


int Server::configure_server()
{
    int res = 0;

    struct sockaddr_in sockaddr_struct = {};                    // Init struct set to zero
    sockaddr_struct.sin_family = AF_INET;                       // IPv4
    sockaddr_struct.sin_port = htons(port);                     // input port
    sockaddr_struct.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // 127.0.0.1

    this->server_socket_fd = socket(sockaddr_struct.sin_family, SOCK_STREAM, 0); // SOCK_STREAM = TCP, 0 = default protocol
    if (this->server_socket_fd == -1)
    {
        std::cerr << RED << "Server: Error: socket function. errno = " << errno << COLOR_END << std::endl;
        return -1;
    }// std::cout << "Server: socket created. fd: " << this->server_socket_fd << std::endl;

    
    // !!! a revoir si correct, set socket options
    int i = 1;
    res = setsockopt(this->server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &i, sizeof(i));
    if (res < 0)
    {
        std::cerr << RED << "Server: Error: setsockopt function. errno = " << errno << COLOR_END << std::endl;
        return -1;
    }

    int flags = fcntl(this->server_socket_fd, F_GETFL, 0);
	fcntl(this->server_socket_fd, F_SETFL, flags | O_NONBLOCK);


    res = bind(this->server_socket_fd, reinterpret_cast<struct sockaddr *>(&sockaddr_struct), sizeof sockaddr_struct);
    if (res != 0)
    {
        std::cerr << RED << "Server: Error: bind function. errno = " << errno << COLOR_END << std::endl;
        return -1;
    }// std::cout << "Server: bound socket to port " << this->port << std::endl;
    

    res = listen(this->server_socket_fd, 10); //!!! need more than 10
    if (res != 0)
    {
        std::cerr << RED << "Server: Error: listen function. errno " << errno << COLOR_END << std::endl;
        return -1;
    }// std::cout << "Server: listening" << std::endl;



    this->fds_poll->at(0).fd = this->server_socket_fd;
    this->fds_poll->at(0).events = POLLIN;
    this->connected_fds = 1;
    if (poll_function() == -1)
        return -1;

    return 0;
}



int Server::poll_function()
{
    int res = -1;

    while (1)
    {
        res = poll(this->fds_poll->data(), this->connected_fds, 2000);
        if (res == -1)
        {
            std::cerr << RED << "Server: Error: poll function. errno = " << errno << COLOR_END << std::endl;
            return -1;
        }
        else if (res == 0)
        {
            // std::cout << "Server: poll while loop functionning" << std::endl;
            continue;
        }

        for (int i = 0; i < this->connected_fds; i++)
        {
            //POLLIN = if there is data to read 
            //revents = returned events after poll() work
            // != 1 means the POLLIN flag is not set for this fd
            //so there is no data to read
            if ((this->fds_poll->at(i).revents & POLLIN) != 1)
            {
                continue;
            }
            if (this->fds_poll->at(i).fd == this->server_socket_fd) //socket of the server
            {
                accept_data();
            }
            else //clients sockets
            {
                receive_data(i);
            }
        }
    }
}


//verify if name > 1 && < 9
//only [0-9] [A-Z] [a-z] allowed
bool    name_format_is_valid(const std::string input)
{
    if (!input.size() || input.size() > 9 || input.size() < 1)
        return false;
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
    {
        if (!::isalnum(*it))
        {
            return false;
        }
    }
    return true;
}


//true = nickname does not exist in the Users nicknames stored in the server
bool    Server::nickname_is_available(std::string nickname)
{
    for (size_t i = 0; i < this->users.size(); ++i)
    {
        if (users[i]->get_nickname() == nickname)
        {
            return false;
        }
    }
    return true;
}


//true = username does not exist in the Users usernames stored in the server
bool    Server::username_is_available(std::string username)
{
    for (size_t i = 0; i < this->users.size(); ++i)
    {
        if (users[i]->get_username() == username)
        {
            return false;
        }
    }
    return true;
}


//creation of valid nickname and username
void    Server::nickname_and_username_creation(int user_fd)
{

    User *new_user = new User();
    new_user->set_id(user_fd);
    add_user_to_server(new_user);
    //print all users for test
    print_all_users();
}


void    Server::accept_data()
{
    int                 user_fd = -1;
    struct sockaddr_in  sockaddr_struct_client = {};  // !!! need a struct ???
    socklen_t           client_addr_size = sizeof(sockaddr_struct_client);


    //accept incoming client connexions
    user_fd = accept(this->server_socket_fd, reinterpret_cast<struct sockaddr *>(&sockaddr_struct_client), &client_addr_size);
    if (user_fd == -1)
    {
        // no pending connections when using a non-blocking socket
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        else
        {
            std::cerr << RED << "Server: Error: accept function. errno = " << errno << COLOR_END << std::endl;
            return;
        }
    }
    std::cout << GREEN << "Server: New connection accepted. User fd = " << user_fd << COLOR_END << std::endl;

    int flags = fcntl(user_fd, F_GETFL, 0);
	fcntl(user_fd, F_SETFL, flags | O_NONBLOCK);

    send_message("001 :Welcome to the IRC server\r\n", user_fd);

    this->fds_poll->at(this->connected_fds).fd = user_fd;
    this->fds_poll->at(this->connected_fds).events = POLLIN | POLLOUT; // !!! POULLOUT
    this->connected_fds++;

    nickname_and_username_creation(user_fd);
}


void    Server::send_message(std::string message_to_send, int user_fd)
{
    int res = -1;

    res = send(user_fd, message_to_send.c_str(), message_to_send.size(), 0);
    if (res == -1)
    {
        std::cerr << RED << "Server: Error: send function message: '" + message_to_send + "' to " << user_fd << ". errno = " << errno << COLOR_END << std::endl;
    }
    else
    {
        std::cout << GREEN << "Server sent message '" + message_to_send + "' to " << user_fd << COLOR_END << std::endl;
    }
}


void    Server::receive_data(int i)
{
    std::vector<char>   buffer(BUFSIZ);
    int                 recv_res = -1;
    int                 sender_fd = this->fds_poll->at(i).fd;

    std::cout << GREEN << "test" << COLOR_END << std::endl;

    // init buffer
    std::fill(buffer.begin(), buffer.end(), '\0');

    // receive all messages from user (sender_fd)
    recv_res = recv(sender_fd, &buffer[0], BUFSIZ, 0);
    if (recv_res <= 0)
    {
        if (recv_res == 0)
        {
            std::cout << GREEN << "User fd " << sender_fd << " disconnected" << COLOR_END << std::endl;
        }
        else
        {
            std::cerr << RED << "Server: Error: recv function input from user_fd " << sender_fd << ". errno = " << errno << COLOR_END << std::endl;
        }
        close(sender_fd);
        this->fds_poll->at(i) = this->fds_poll->at(this->connected_fds - 1);
        this->connected_fds--;
    }

    // convert vector to string
    std::string buffer_str;
    std::vector<char>::iterator itvect = std::find(buffer.begin(), buffer.end(), '\0');
    if (itvect == buffer.end())
        buffer_str.append(buffer.begin(), buffer.end());
    else
        buffer_str.append(buffer.begin(), itvect);

    // save command in buffer
    std::map<int, std::string>::iterator itmap = this->buffers.find(sender_fd);
    if (itmap == this->buffers.end())
        this->buffers.insert(std::make_pair(sender_fd, buffer_str));
    else
        itmap->second = itmap->second + buffer_str;

    // check if command is completed
    itmap = this->buffers.find(sender_fd);
    if (itmap->second.find('\n') != std::string::npos) {

        buffer_str = itmap->second;
        this->buffers.erase(itmap);

        // identify user to know later who sent the CMD to server
        User *current_user = get_user_ptr_from_fd(sender_fd);
        identify_command(itmap->second, current_user);
    }

    std::string received_message(buffer.begin(), buffer.begin() + recv_res);

    // !!! function a ameliorer ou remplacer 
    send_command_from_multimap(received_message, sender_fd);
}


// !!! function a ameliorer ou remplacer 
void    Server::send_command_from_multimap(std::string received_message, int sender_fd)
{
    int res = -1;

    // test printing buffer
    std::cout << BLUE << "Buffer = " << received_message << COLOR_END << std::endl;
    for (std::multimap<std::string, int>::iterator it = this->messages.begin(); it != this->messages.end(); ++it)
        std::cout << BLUE << "Messages = " << it->first << COLOR_END << std::endl;


    for (std::multimap<std::string, int>::iterator it = this->messages.begin(); it != this->messages.end(); ++it)
    {
        std::string message_to_send = it->first;

        res = send(it->second, message_to_send.c_str(), message_to_send.size(), 0);
        if (res == -1)
        {
            std::cerr << RED << "Server: Error: Password: send function: welcome_message string for user_fd " << sender_fd << ". errno = " << errno << COLOR_END << std::endl;
        }
        else
        {
            //send message to server terminal
            std::cout << GREEN << "User " << sender_fd << " to " << it->second << ". Message: " << it->first;
            std::ostringstream  message_to_server;
            message_to_server << sender_fd << " said: " << it->first;
            std::cout << COLOR_END;
        }
    }
    this->messages.clear();
}


void    Server::add_new_channel(std::string channel_name)
{
    Channel *new_channel = new Channel(channel_name);
    this->channels.push_back(new_channel);
}


// !!! to finish
std::string extract_password(std::string input)
{
    std::string pass_command = "PASS";
    std::size_t pass_pos = input.find(pass_command);

    if (pass_pos != std::string::npos)
    {
        pass_pos += pass_command.length();
        std::string password = input.substr(pass_pos);
        return password;
    }
    else
        return "";
}


// !!! check why adding weird channel name sometimes (when /JOIN #channel used to change channel)
void    Server::join_cmd(std::string channel_name, std::istringstream &iss, User *current_user)
{
    std::string temp = "";

    //check if password validated !!! HERE


    //!!! temp while. need to forbid to write after the name of the channel
    //!!! + verify if channel name input is correct + not empty
    //!!! + message if already joined  
    while (iss >> temp)
    {
        //remove '#'
        if (temp[0] == '#')
        {
            channel_name = temp.substr(1);
            break;
        }
    }


    //si n'existe pas dans vector channels
        //creer new channel
        //ajouter au vector du serveur
        //user devient un operateur dedans
    //si existe
        //si user n'est pas dans le chan
            //user rejoin le chan en mode user
    Channel *channel_ptr = get_channel_ptr(channel_name);
    if (channel_ptr == NULL)
    {
        // std::cout << "Channel does not exist" << std::endl;
        channel_ptr = new Channel(channel_name);
        this->channels.push_back(channel_ptr);
        channel_ptr->set_channel_name(channel_name);
        channel_ptr->add_user('o', current_user);
    }
    else
    {
        // std::cout << "\033[36mChannel does exist\033[0m" << std::endl; //test
        if (user_not_is_the_channel(current_user, channel_ptr))
        {
            channel_ptr->add_user('u', current_user);
        }
        else
        {
        }
    }
    print_all_channels_with_all_users(); //test
}


// !!! en cours : privmsg to user 
void    Server::privmsg_cmd(std::string buffer_content, std::istringstream &iss, User *current_user)
{
    //check if password validated !!!


    std::string receiver;
    std::string massage_content;
    iss >> receiver;
    std::getline(iss, massage_content);


    // Remove leading whitespace from massage_content
    massage_content = massage_content.substr(massage_content.find_first_not_of(" \t"));

    if (!receiver.empty() && !massage_content.empty())
    {
        // Check if receiver is a channel or user (starts with '#')
        if (receiver[0] == '#')
        {
            // Find the channel pointer
            Channel *channel_ptr = get_channel_ptr(receiver.substr(1)); // Remove '#' from receiver

            if (channel_ptr != NULL)
            {
                std::multimap<std::string, int>& multimap_copy = this->messages;
                std::cout << BLUE << "Reciever = " << receiver + " from ptr = " + channel_ptr->get_channel_name() << COLOR_END << std::endl;

                // Forward the message to all users in the channel except the sender
                channel_ptr->insert_message(buffer_content, current_user->get_nickname(), multimap_copy);   
            }
            else
            {
                //case when channel does not exist    
            }
        }
    // else
    // {
    //     //private messages to individual users
    // }
    // else
    // {
    //     //case when receiver or message is empty
    // }
    }
}


//respond with PONG + servername which is localhost ??? and why we don't have a normal server name now
void    Server::pong_cmd(std::istringstream &iss, User *current_user)
{
    std::string tmp;
    iss >> tmp;
    std::string message_to_send = "PONG " + tmp + "\r\n";
    std::cout << message_to_send;

    send_message(message_to_send, current_user->get_id());
}

//set nickname
void    Server::nick_cmd(std::istringstream &iss, User *current_user)
{
    std::string nickname;
    iss >> nickname;
    current_user->set_nickname(nickname);
    std::cout << GREEN << "User fd " << current_user->get_id() << " new nickname = " << current_user->get_nickname() << COLOR_END << std::endl;
    print_all_users(); // test
}


//set username
void    Server::user_cmd(std::istringstream &iss, User *current_user)
{
    std::string username;
    iss >> username;
    current_user->set_username(username);
    std::cout << GREEN << "User fd " << current_user->get_id() << " new username = " << current_user->get_username() << COLOR_END << std::endl;
    print_all_users(); // test
}


// !!! a finir
// commande PASS a extraire d'une longue string parmi d'autres commandes 
void    Server::pass_cmd(std::string buffer_content, User *current_user)
{
    std::string pass = extract_password(buffer_content);

    if (pass != "")
    {
        if (pass == this->password)
        {
            std::cout << GREEN << "User fd " << current_user->get_id() << " entered the correct password !" << COLOR_END << std::endl;
            current_user->set_password_validation(true);

        }
        else
        {
            std::cout << RED << "User fd " << current_user->get_id() << " entered the wrong password ! '" << pass << "'" << COLOR_END << std::endl;
            current_user->set_password_validation(true);
        }
    }
    else
    {
        std::cout << BLUE << "User fd " << " already entered the correct password." << COLOR_END << std::endl;
    }
}


int Server::identify_command(std::string buffer, User *current_user)
{
    std::string         buffer_content(buffer);
    std::string         channel_name = "";
    std::istringstream  iss(buffer_content);
    std::string         command = "";

    //!!! parsing a refaire
    iss >> command;

    if (command == "JOIN")
    {
        join_cmd(channel_name, iss, current_user);
    }
    else if (command == "PRIVMSG")
    {
        privmsg_cmd(buffer_content, iss, current_user);
    }
    else if (command == "PING")
    {
        pong_cmd(iss, current_user);
    }
    else if (command == "NICK")
    {
        nick_cmd(iss, current_user);
    }
    else if (command == "userhost" || command == "USER") //"userhost" for irssi, "USER" for netcat
    {
        user_cmd(iss, current_user);
    }
    else if (command == "QUIT")
    {
        // !!! en cours
    }
    else
    {
        pass_cmd(buffer_content, current_user);
    }
    return 0;
}


Channel *Server::get_channel_ptr(std::string channel_name)
{
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i]->get_channel_name() == channel_name)
        {
            return channels[i];
        }
    }
    return NULL;
}


bool    Server::user_not_is_the_channel(User *user, Channel *channel)
{
    // //test
    // if (!channel->is_user_in_channel(user) == true)
    // {
    //     std::cout << "\033[36mUser " << user->get_nickname() << " not in the channel \033[0m"<< std::endl;
    // }
    // else
    // {
    //     std::cout << "\033[36mUser " << user->get_nickname() << " is in the channel \033[0m"<< std::endl;
    // }
    // //
    return !channel->is_user_in_channel(user);
}


User    *Server::get_user_ptr_from_fd(int id)
{
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i]->get_id() == id)
        {
            return users[i];
        }
    }
    // std::cout << "User with ID " << id << " not found!" << std::endl;
    return NULL;
}


void    Server::add_user_to_server(User *user)
{
    //check if the user is already in the server
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i] == user)
        {
            // std::cout << "User already exists in the server." << std::endl;
            return;
        }
    }

    //add the user to the server
    users.push_back(user);
    // std::cout << "User added to the server." << std::endl;
}


//for tests
void    Server::print_all_channels_with_all_users()
{
    std::cout << BLUE;
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::cout << "Channel: " << channels[i]->get_channel_name() << std::endl;
        channels[i]->print_channel_users();
    }
    std::cout << COLOR_END << std::endl;
}


//for tests
void Server::print_all_users()
{
    std::cout << "\033[34m";
    std::cout << "----------------------------------\n";
    std::cout << "Printing all Users:\n";
    for (size_t i = 0; i < users.size(); ++i)
    {
        User *currentUser = users[i];
        std::cout << "User Address: " << currentUser << std::endl;
        std::cout << "Client FD: " << currentUser->get_id() << std::endl;
        std::cout << "User ID: " << currentUser->get_id() << std::endl;
        std::cout << "Nickname: " << currentUser->get_nickname() << std::endl;
        std::cout << "Username: " << currentUser->get_username() << std::endl;
        std::cout << "----------------------------------\n";
    }
    std::cout << "\033[0m";
}
