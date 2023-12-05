#include "Channel.hpp"

Channel::Channel()
{
    name = "";
    topic = "";
    channel_password = "";
    user_limit = -1;
    i = false; //no invitation required
    t = false; //no restrictions on the TOPIC command
    k = false; //no password
    o = false; //no channel operator privilege given
    l = false; //unlimited users
}


Channel::Channel(std::string channel_name)
{
    name = channel_name;
    topic = "";
    channel_password = "";
    user_limit = -1;
    i = false; //no invitation required
    t = false; //no restrictions on the TOPIC command
    k = false; //no password
    o = false; //no channel operator privilege given
    l = false; //unlimited users
}


Channel::~Channel()
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        delete it->second;
    }
    channel_users.clear();
    invitation_list.clear();
}


void    Channel::set_channel_name(std::string name)
{
    this->name = name;
}


std::string Channel::get_channel_name() const
{
    return this->name;
}

bool    Channel::getModeI(void) const {
    return (this->i);
}

bool    Channel::getModeT(void) const {
    return (this->t);
}

bool    Channel::getModeK(void) const {
    return (this->k);
}

bool    Channel::getModeO(void) const {
    return (this->o);
}

bool    Channel::getModeL(void) const {
    return (this->l);
}

void    Channel::setModeI(bool i) {
    this->i = i;
}

void    Channel::setModeT(bool t) {
    this->t = t;
}

void    Channel::setModeK(bool k) {
    this->k = k;
}

void    Channel::setModeO(bool o) {
    this->o = o;
}

void    Channel::setModeL(bool l) {
    this->l = l;
}

//not used
int Channel::check_if_user_invited(std::string nickname)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (nickname == it->second->get_nickname())
            return 0;
    }
    return -1;
}

//not used
void    Channel::set_topic(std::string new_topic, std::string nickname)
{
    bool    is_operator = user_is_operator(nickname);

    if (is_operator|| (!is_operator && !this->t)) //no restrictions on the TOPIC command
        topic = new_topic;
    else
        std::cerr << "ERROR MESSAGE SET TOPIC : USER PRIVILEGES" << std::endl; //!!! need to change message (from protocol)
}

//not used
void    Channel::set_channel_password(std::string new_channel_password, std::string nickname)
{
    if (!this->k)
    {
        std::cerr << "ERROR CHANNEL DOES NOT HAVE PASSWORD" << std::endl;
        return ;
    }

    if (user_is_operator(nickname))
        channel_password = new_channel_password;
    else
        std::cerr << "ERROR SET PASSWORD : USER PRIVILEGES" << std::endl; //!!! need to change message (from protocol)
}

//not used
int Channel::check_password_is_matching(std::string user_input)
{
    if (user_input == channel_password)
        return 0;
    return -1;
}

//not used
void    Channel::user_become_operator(std::string nickname)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (nickname == it->second->get_nickname())
        {
            if (it->first == 'u')
            {
                User *user = it->second;
                channel_users.erase(it);
                channel_users.insert(std::make_pair('o', user));
                return;
            }
        }
    }
}

//not used
void    Channel::operator_become_user(std::string nickname)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (nickname == it->second->get_nickname())
        {
            if (it->first == 'o')
            {
                User *user = it->second;
                channel_users.erase(it);
                channel_users.insert(std::make_pair('u', user));
                return;
            }
        }
    }
}

//not used
void    Channel::set_user_limit(int new_user_limit, std::string nickname)
{
    if (!this->l)
    {
        std::cerr << "ERROR : USER LIMIT MODE UNSET" << std::endl;
        return ;
    }
    
    if (user_is_operator(nickname))
        user_limit = new_user_limit;
    else
        std::cerr << "ERROR SET LIMIT : USER PRIVILEGES" << std::endl;
}


int Channel::get_user_limit() const
{
    return user_limit;
}


bool    Channel::user_is_operator(std::string nickname)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (nickname == it->second->get_nickname() && it->first == 'o')
           return true;
    }
    return false;
}

//not used
void Channel::remove_user(std::string nickname)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (nickname == it->second->get_nickname())
        {
            delete it->second;
            channel_users.erase(it);
            return;
        }
    }
}


void Channel::add_user(char type, User *user)
{
    std::multimap<char, User *>::iterator result;
    
    result = channel_users.insert(std::make_pair(type, user));


    // //test
    // if (result != channel_users.end())
    // {
    //     std::cout << "\033[36mNickname of new user to add = " << user->get_nickname() << "\033[0m" << std::endl;
    //     this->print_all_channel_users();
    // }
    // else
    // {
    //     std::cerr << "Insertion failed." << std::endl;
    // }
}


bool    Channel::is_user_in_channel(User *user)
{
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        if (it->second == user)
        {
            // std::cout << "\033[36mUser " << user->get_nickname() << " found.\033[0m" << std::endl;
            return true;
        }
    }
    // std::cout << "\033[36mUser " << user->get_nickname() << " not found.\033[0m" << std::endl;
    return false;
}


void    Channel::insert_message(std::string message, std::string nickname, std::multimap<std::string, int> &messages)
{
    std::cout << BLUE << "Message = " << message << std::endl;
    std::cout << "Sender nickname = " << nickname << COLOR_END << std::endl;

    message = ":" + nickname + " " + message + "\r\n";

    //generate massages to all other members of channel
    for (std::multimap<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        messages.insert(std::make_pair(message, it->second->get_id()));
    }

    

    // for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it)
    //     std::cout << "Messages content = " << *it << std::endl;
}




//for test
void    Channel::print_channel_users()
{
    for (std::map<char, User *>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        std::cout << "    User: " << it->second->get_nickname();
        if (it->first == 'o')
        {
            std::cout << " (Operator)";
        }
         std::cout << " Address: " << it->second << std::endl;
    }
}


//for tests
void Channel::print_all_channel_users()
{
    std::cout << BLUE;
    std::cout << "----------------------------------\n";
    std::cout << "Printing all channel users for channel: " << name << std::endl;
    std::map<char, User *>::iterator it;
    for (it = channel_users.begin(); it != channel_users.end(); ++it)
    {
        char type = it->first;
        User* user = it->second;
        std::cout << "User Address: " << user << std::endl;
        std::cout << "Client FD: " << user->get_id() << std::endl;
        std::cout << "User ID: " << user->get_id() << std::endl;
        std::cout << "Nickname: " << user->get_nickname() << std::endl;
        std::cout << "Type: " << type << std::endl;
        std::cout << "----------------------------------\n";
    }
    std::cout << COLOR_END;
}
