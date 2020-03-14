#ifndef REMOTE_PLAYER_H
#define REMOTE_PLAYER_H
#include <Windows.h>
#include "ch_chess.h"
#include "net_trans.h"

#define MAX_STR_LEN 128
#define MAX_PK_PENDING 6
#define ERROR_PK_PENDING 4000
enum net_init_state{
    NOT_CALLED,
    WAITING,
    READY,
    FAILED
};
enum package_type{
    CHESS_STEP,
    REQUEST_DRAWN,
    REQUEST_SWITCH,
    REQUEST_GIVE,
    SET_REMOTE_PLAYER,
    NETCMD_START_BUTTON,
    SET_TIMEOUT,
    ACK,
    APP_QUIT,
    TIMER_PAUSE,
    STRING
};

struct trans_package{
    package_type p_type;
    int pk_id;
    struct pck_data{
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
        remote_player();
        virtual bool init(const char*ip, u_short port)=0;
        virtual trans_package* get_trans_pack_buf()=0;
        virtual bool is_ready()=0;
        virtual trans_package* get_recved_ok()=0;
        virtual bool send_package(trans_package*)=0;
        virtual bool send_cmd(package_type)=0;
        virtual bool send_ack(int id)=0;
        virtual void deinit()=0;
        net_init_state get_init_state()
        {
            return init_state;
        }
        int get_error_status(){
            bool ret = error_status;
            error_status = 0;
            return ret;
        }

    protected:
        int error_status;
        bool connec_is_rdy;
        trans_package tpg;
        trans_package tpg_bak;
        trans_package ack_tpg;
        net_init_state init_state;
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
        void deinit();
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
        bool send_ack(int id);
        void deinit();
        ~net_remote_player();
    private:
        const char*mip;
        u_short mport;
        net_trans mynt;
        int data_left_len;
        char* data_left_buf;
        int current_pk_id;
        int pending_pk_id;
        int pk_pending_last;
        static DWORD WINAPI init_thread_func(LPVOID lpThreadParameter);

};

#endif