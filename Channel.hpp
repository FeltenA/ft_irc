#ifndef CHANNEL_HPP
#define CHANNEL_HPP


#include "Server.hpp"
#include "User.hpp"

class Channel
{
    public:
        Channel();
        Channel(std::string channel_name);
        ~Channel();
        void                    set_channel_name(std::string name);
        std::string             get_channel_name() const;
        int                     check_if_user_invited(std::string nickname);
        void                    set_topic(std::string new_topic, std::string nickname);
        void                    set_channel_password(std::string new_channel_password, std::string nickname);
        int                     check_password_is_matching(std::string user_input);
        void                    user_become_operator(std::string nickname);
        void                    operator_become_user(std::string nickname);
        void                    set_user_limit(int new_user_limit, std::string nickname);
        int                     get_user_limit() const;
        bool                    user_is_operator(std::string nickname);
        void                    remove_user(std::string nickname);
        void                    add_user(char type, User *user);
        bool                    is_user_in_channel(User *user);
        void                    insert_message(std::string message, std::string nickname, std::multimap<std::string, int> &messages);

        bool                    getModeI(void) const;
        bool                    getModeT(void) const;
        bool                    getModeK(void) const;
        bool                    getModeO(void) const;
        bool                    getModeL(void) const;
        void                    setModeI(bool i);
        void                    setModeT(bool t);
        void                    setModeK(bool k);
        void                    setModeO(bool o);
        void                    setModeL(bool l);

        //for tests
        void                    print_channel_users();
        void                    print_all_channel_users();
    
    private:
        std::string                     name;
        std::string                     topic;
        std::string                     channel_password;
        int                             user_limit;
        std::multimap<char, User *>     channel_users; //char: 'o' = is_operator // 'u' = is_user
        std::vector<std::string>        invitation_list;

        bool                            i;
        bool                            t;
        bool                            k;
        bool                            o;
        bool                            l;
};

#endif
