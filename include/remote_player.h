#ifndef REMOTE_PLAYER_H
#define REMOTE_PLAYER_H
#include<list>
#include <Windows.h>
#include "ch_chess.h"
#include "net_trans.h"

#define MAX_STR_LEN 64
#define MAX_PK_PENDING 6
#define MAX_HK_PK_PENDING 10
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
    HANDSHAKE,
    STRING
};

struct trans_package{
    package_type p_type;
    int pk_id;
    int pk_pending_last;
    struct pck_data{
        move_step ch_move_step;
        package_type request;
        char str_message[MAX_STR_LEN];
        PLAYING_SIDE remote_side;
        int timeout;
    } pd;
};

class tpg_manager{
    public:
    tpg_manager();
    ~tpg_manager();
    bool check_pending();
    trans_package* get_tpg();
    void set_pending(trans_package*);
    trans_package* get_next_pending_tpg();
    bool put_tpg(int pk_id);
    trans_package* get_pending_tpg(int id);

    private:
    list<trans_package*> free_tpg_list;
    list<trans_package*> pending_tpg_list;
    typedef list<trans_package*>::iterator tpg_iter;
    tpg_iter g_tpgit;
    CRITICAL_SECTION g_tpgm_critc_sctn;
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
        virtual void set_quit(){};
        virtual void reset_connect(){};
        net_init_state get_init_state()
        {
            return init_state;
        }
        int get_error_status(){
            return error_status;
        }

    protected:
        tpg_manager tpgm;
        tpg_manager tpgm_recv;
        int error_status;
        bool connec_is_rdy;
        trans_package tpg;
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
        void set_quit();
        void deinit();
        void reset_connect();
        ~net_remote_player();
    private:
        char* mip;
        char store_ip[64];
        u_short mport;
        net_trans mynt;
        int data_left_len;
        char* data_left_buf;
        int current_pk_id;
        int current_recv_pk_id;
        trans_package tpg_bak;
        trans_package ack_tpg;
        int pending_pk_id;
        int pk_pending_last;
        trans_package handshake_tpg;
        int handshake_pending_pk_id;
        int handshake_pk_pending_last;
        static DWORD WINAPI init_thread_func(LPVOID lpThreadParameter);
        LONG volatile send_package_guard;
        LONG volatile reset_connect_guard;

};

#endif
