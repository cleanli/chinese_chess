#ifndef REMOTE_PLAYER_H
#define REMOTE_PLAYER_H
#include <Windows.h>
#include "ch_chess.h"

#define MAX_STR_LEN 128
enum package_type{
    CHESS_STEP,
    REQUEST_DRAWN,
    REQUEST_SWITCH,
    STRING
};

struct trans_package{
    package_type p_type;
    union pck_data{
        move_step ch_move_step;
        package_type request;
        char str_message[MAX_STR_LEN];
    } pd;
};
class remote_player
{
    public:
        remote_player();
        bool init(const char*ip, u_short port);
        trans_package* get_trans_pack_buf();
        bool is_ready();
        trans_package* get_recved_ok();
        bool send_package(trans_package*);

    private:
        bool connec_is_rdy;
        trans_package tpg;
};

#endif
