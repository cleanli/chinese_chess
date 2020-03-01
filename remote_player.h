#ifndef REMOTE_PLAYER_H
#define REMOTE_PLAYER_H
#include <Windows.h>
#include "ch_chess.h"
#include "net_trans.h"

#define MAX_STR_LEN 128
enum package_type{
    CHESS_STEP,
    REQUEST_DRAWN,
    REQUEST_SWITCH,
    REQUEST_GIVE,
    SET_REMOTE_PLAYER,
    NETCMD_START_BUTTON,
    SET_TIMEOUT,
    SEND_CURRENT_TIMEOUT,
    APP_QUIT,
    STRING
};

struct trans_package{
    package_type p_type;
    union pck_data{
        move_step ch_move_step;
        package_type request;
        char str_message[MAX_STR_LEN];
        PLAYING_SIDE remote_side;
        int timeout;
    } pd;
};
class remote_player
{
    public:
        virtual bool init(const char*ip, u_short port)=0;
        virtual trans_package* get_trans_pack_buf()=0;
        virtual bool is_ready()=0;
        virtual trans_package* get_recved_ok()=0;
        virtual bool send_package(trans_package*)=0;
        virtual bool send_cmd(package_type)=0;
        virtual bool send_cur_timeout(int timeout)=0;

    protected:
        bool connec_is_rdy;
        trans_package tpg;
};
class dummy_remote_player:public remote_player
{
    public:
        dummy_remote_player();
        bool init(const char*ip, u_short port);
        trans_package* get_trans_pack_buf();
        bool is_ready();
        trans_package* get_recved_ok();
        bool send_package(trans_package*);
};
class net_remote_player:public remote_player
{
    public:
        net_remote_player();
        bool init(const char*ip, u_short port);
        trans_package* get_trans_pack_buf();
        bool is_ready();
        trans_package* get_recved_ok();
        bool send_package(trans_package*);
        bool send_cmd(package_type);
        bool send_cur_timeout(int timeout);
        ~net_remote_player();
    private:
        net_trans mynt;

};

#endif
