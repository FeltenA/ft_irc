#ifndef USER_HPP
#define USER_HPP


#include <string>


class User
{
    public:
        User();
        ~User();
        void            set_nickname(std::string nickname);
        std::string     get_nickname() const;
        void            set_username(std::string username);
        std::string     get_username() const;
        void            set_id(int client_fd);
        int             get_id()const;
        void            set_password_validation(bool is_valid_or_not);
        bool            get_password_validation() const;

    private:
        std::string     nickname;
        std::string     username;
        int             id;
        bool            password_validation_passed;

};

#endif
