#include "User.hpp"

User::User()
{
    this->nickname = "";
    this->username = "";
    this->id = -1;
    this->password_validation_passed = false;
}


User::~User()
{

}


void    User::set_nickname(std::string nickname)
{
    this->nickname = nickname;
}


std::string User::get_nickname() const
{
    return this->nickname;
}


void    User::set_username(std::string username)
{
    this->username = username;
}


std::string User::get_username() const
{
    return this->username;
}


void    User::set_id(int client_fd)
{
    this->id = client_fd;
}


int     User::get_id() const
{
    return this->id;
}


void    User::set_password_validation(bool is_valid_or_not)
{
    this->password_validation_passed = is_valid_or_not;
}


bool    User::get_password_validation() const
{
    return this->password_validation_passed;
}
