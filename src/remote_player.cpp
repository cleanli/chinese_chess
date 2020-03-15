#include "remote_player.h"

const char* pk_type_str[]={
    "CHESS_STEP",
    "REQUEST_DRAWN",
    "REQUEST_SWITCH",
    "REQUEST_GIVE",
    "SET_REMOTE_PLAYER",
    "NETCMD_START_BUTTON",
    "SET_TIMEOUT",
    "ACK",
    "APP_QUIT",
    "TIMER_PAUSE",
    "HANDSHAKE",
    "STRING"
};


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

void dummy_remote_player::deinit()
{
}

bool dummy_remote_player::send_package(trans_package*tp)
{
    return true;
}

remote_player::remote_player()
  : connec_is_rdy(false),
    error_status(0),
    init_state(NOT_CALLED)
{
}

net_remote_player::net_remote_player()
  : data_left_len(0),
    pk_pending_last(0),
    handshake_pk_pending_last(0),
    send_package_guard(0),
    reset_connect_guard(0),
    current_pk_id(1)
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
    if(ip != NULL){
        if(store_ip != ip){//avoid self copy
            strcpy(store_ip, ip);
            mip = store_ip;
        }
    }
    else{
        mip = NULL;
    }
    mport = port;
    df("net_remote_player::init");
    memset(&tpg, 0, sizeof(trans_package));
    memset(&ack_tpg, 0, sizeof(trans_package));
    memset(&handshake_tpg, 0, sizeof(trans_package));
    DWORD dwThreadID;
    HANDLE hHandle = CreateThread(0, 0, init_thread_func, (LPVOID)this, 0, &dwThreadID);
    return true;
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
    //check pk id ack
    if(pending_pk_id){
        pk_pending_last++;
    }
    if(pk_pending_last > MAX_PK_PENDING){
        bool ret;
        error_status = 1;
        df("Error! last package didn't get ack! id %d last %d",
                pending_pk_id, pk_pending_last);
        if(tpg_bak.pk_id == pending_pk_id && (pk_pending_last%10) == 0){
            ret = mynt.net_send((const char*)&tpg_bak, sizeof(trans_package));
            df("re-send last failed package id %d return %d", ret);
        }
        else if(tpg_bak.pk_id != pending_pk_id ){
            df("can't re-send last failed package, data lost");
        }
    }
    if(handshake_pending_pk_id){
        handshake_pk_pending_last++;
    }
    if(handshake_pk_pending_last > MAX_HK_PK_PENDING){
        bool ret;
        error_status = 1;
        df("Error! last handshake package didn't get ack! id %d last %d",
                handshake_pending_pk_id, handshake_pk_pending_last);
    }
    if(pk_pending_last > ERROR_PK_PENDING){
        df("connection is error, stop send/recv");
        error_status = 1;
        //connec_is_rdy = false;
    }
    //check pk id ack end
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
        }
    }
    else{
        memcpy(&tpg, data_left_buf, sizeof(trans_package));
        data_left_len -= sizeof(trans_package);
        data_left_buf += sizeof(trans_package);
        if(data_left_len==0){
            mynt.buf_return();
        }
    }
    error_status = 0;
    df("%s pk_id %d p_type:%d %s", __func__, tpg.pk_id, tpg.p_type, pk_type_str[tpg.p_type]);
    if(tpg.p_type == ACK){
        df("pending pk id %d", pending_pk_id);
        if(pending_pk_id == tpg.pk_id){
            df("pending last=%d, cleared", pk_pending_last);
            pk_pending_last = 0;
            pending_pk_id = 0;
        }
        if(handshake_pending_pk_id == tpg.pk_id){
            df("handshake pending last=%d, cleared", handshake_pk_pending_last);
            handshake_pk_pending_last = 0;
            handshake_pending_pk_id = 0;
        }
        return NULL;
    }
    return &tpg;
}

bool net_remote_player::send_package(trans_package*tp)
{
    bool ret;
    InterlockedIncrement(&send_package_guard);
    while(send_package_guard > 1){
        df("send_package in process, waiting...");
        Sleep(100);
        df("send_package in process, waiting done");
    }

    if(tp->p_type == ACK){
        ret = mynt.net_send((const char*)tp, sizeof(trans_package));
        df("send ack of id %d return %d", tp->pk_id, ret);
    }
    else if(tp->p_type == HANDSHAKE){
        tp->pk_id = current_pk_id++;
        handshake_pending_pk_id=tp->pk_id;
        handshake_pk_pending_last = 0;
        ret = mynt.net_send((const char*)tp, sizeof(trans_package));
    }
    else{
        tp->pk_id = current_pk_id++;
        df("%s type %d %s id %d", __func__, tp->p_type,
                pk_type_str[tp->p_type], tp->pk_id);
        pending_pk_id=tp->pk_id;
        pk_pending_last = 0;
        memset(&tpg_bak, 0, sizeof(trans_package));
        memcpy(&tpg_bak, (const char*)tp, sizeof(trans_package));
        ret = mynt.net_send((const char*)tp, sizeof(trans_package));
    }
    InterlockedDecrement(&send_package_guard);
    return ret;
}

bool net_remote_player::send_cmd(package_type pt)
{
    if(pt == HANDSHAKE){
        handshake_tpg.p_type = pt;
        send_package(&handshake_tpg);
    }
    else{
        tpg.p_type = pt;
        send_package(&tpg);
    }
    return true;
}

bool net_remote_player::send_ack(int id)
{
    ack_tpg.p_type = ACK;
    ack_tpg.pk_id = id;
    return send_package(&ack_tpg);
}

DWORD WINAPI net_remote_player::init_thread_func(LPVOID lpThreadParameter)
{
    net_remote_player* p = (net_remote_player*)lpThreadParameter;
    p->init_state = WAITING;
    df("%s run", __func__);
    if(p->mynt.try_init(p->mip,p->mport)){
        p->connec_is_rdy=true;
        p->init_state=READY;
        df("%s OK", __func__);
    }
    else{
        //p->connec_is_rdy=false;
        p->init_state=FAILED;
        df("%s failed", __func__);
    }
    InterlockedDecrement(&p->reset_connect_guard);
    df("%s --", __func__);
    return 0;
}

void net_remote_player::set_quit()
{
    connec_is_rdy=false;
}

void net_remote_player::reset_connect()
{
    df("%s +", __func__);
    InterlockedIncrement(&reset_connect_guard);
    if(reset_connect_guard > 1){
        df("net reset is still running, return");
        InterlockedDecrement(&reset_connect_guard);
        df("%s -", __func__);
        return;
    }
    mynt.deinit();
    init(mip, mport);
    df("%s -", __func__);
}

void net_remote_player::deinit()
{
    mynt.deinit();
}

