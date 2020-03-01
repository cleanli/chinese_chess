#include "remote_player.h"

dummy_remote_player::dummy_remote_player()
{
};

bool dummy_remote_player::init(const char*ip, u_short port)
{
    //connec_is_rdy = true;
    connec_is_rdy=false;
    memset(&tpg, 0, sizeof(trans_package));
    return true;
}

bool dummy_remote_player::is_ready()
{
    if(!connec_is_rdy){
        connec_is_rdy = true;
        return false;
    }
    return connec_is_rdy;
}

trans_package* dummy_remote_player::get_trans_pack_buf()
{
    return &tpg;
}

trans_package* dummy_remote_player::get_recved_ok()
{
    static int ct = 0;
    if(ct++ < 50){
        return NULL;
    }
    ct = 0;
    if((tpg.pd.ch_move_step.x1==0 &&
            tpg.pd.ch_move_step.y1==0 &&
            tpg.pd.ch_move_step.x2==0 &&
            tpg.pd.ch_move_step.y2==0)
        || tpg.p_type== SET_REMOTE_PLAYER){
        tpg.p_type=CHESS_STEP;
        tpg.pd.ch_move_step.x1=7;
        tpg.pd.ch_move_step.y1=9;
        tpg.pd.ch_move_step.x2=6;
        tpg.pd.ch_move_step.y2=7;
    }
    else{
        tpg.pd.ch_move_step.x1=MAX_CHS_BOARD_X -1 -tpg.pd.ch_move_step.x1;
        tpg.pd.ch_move_step.y1=MAX_CHS_BOARD_Y -1 -tpg.pd.ch_move_step.y1;
        tpg.pd.ch_move_step.x2=MAX_CHS_BOARD_X -1 -tpg.pd.ch_move_step.x2;
        tpg.pd.ch_move_step.y2=MAX_CHS_BOARD_Y -1 -tpg.pd.ch_move_step.y2;
    }
    if(tpg.pd.ch_move_step.x1==tpg.pd.ch_move_step.y1){
        tpg.p_type=REQUEST_GIVE;
    }
    return &tpg;
}

bool dummy_remote_player::send_package(trans_package*tp)
{
    return true;
}



net_remote_player::net_remote_player()
  : data_left_len(0)
{
    df("net remote start\n\r");
    fflush(stdout);
}

net_remote_player::~net_remote_player()
{
    df("net remote ~\r\n");
    fflush(stdout);
    mynt.deinit();
}

bool net_remote_player::init(const char*ip, u_short port)
{
    //connec_is_rdy = true;
    memset(&tpg, 0, sizeof(trans_package));
    if(!ip){
        if(mynt.init(port)){
            connec_is_rdy=true;
        }
        else{
            connec_is_rdy=false;
        }
    }
    else{
        if(mynt.init(ip,port)){
            connec_is_rdy=true;
        }
        else{
            connec_is_rdy=false;
        }
    }
    return connec_is_rdy;
}

bool net_remote_player::is_ready()
{
    return connec_is_rdy;
}

trans_package* net_remote_player::get_trans_pack_buf()
{
    return &tpg;
}

trans_package* net_remote_player::get_recved_ok()
{
    int len;
    void *tmpbuf;
    if(data_left_len == 0){
        if(NULL==(tmpbuf=mynt.net_recv(&len))){
            return NULL;
        }
        else{
            memcpy(&tpg, tmpbuf, sizeof(trans_package));
            if(len > sizeof(trans_package)){
                data_left_len=len-sizeof(trans_package);
                data_left_buf = (char*)tmpbuf+sizeof(trans_package);
            }
            else{
                mynt.buf_return();
            }
            df("%s p_type %d", __func__, tpg.p_type);
            return &tpg;
        }
    }
    else{
        memcpy(&tpg, data_left_buf, sizeof(trans_package));
        data_left_len -= sizeof(trans_package);
        data_left_buf += sizeof(trans_package);
        if(data_left_len==0){
            mynt.buf_return();
        }
        df("%s p_type:%d", __func__, tpg.p_type);
        return &tpg;
    }
}

bool net_remote_player::send_package(trans_package*tp)
{
    df("%s %d", __func__, tp->p_type);
    mynt.net_send((const char*)tp, sizeof(trans_package));
    return true;
}

bool net_remote_player::send_cmd(package_type pt)
{
    tpg.p_type = pt;
    send_package(&tpg);
    return true;
}

bool net_remote_player::send_cur_timeout(int to)
{
    tpg.p_type = SEND_CURRENT_TIMEOUT;
    tpg.pd.timeout = to;
    send_package(&tpg);
    return true;
}
