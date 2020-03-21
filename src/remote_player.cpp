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
    tpgm.set_name("send");
    tpgm_recv.set_name("recv");
}

net_remote_player::net_remote_player()
  : data_left_len(0),
    pk_pending_last(0),
    handshake_pk_pending_last(0),
    send_package_guard(0),
    reset_connect_guard(0),
    current_recv_pk_id(1),
    current_handshake_id(1000000),
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
    return tpgm.get_tpg();
}

trans_package* net_remote_player::get_recved_ok()
{
    int len;
    void *tmpbuf;
    //check pk id ack
#if 0
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
#endif
    if(tpgm.check_pending()){
        trans_package*tmp_tpg_p;
        while(NULL!=(tmp_tpg_p=tpgm.get_next_pending_tpg())){
            if(tmp_tpg_p->pk_pending_last > MAX_PK_PENDING){
                bool ret;
                error_status = 1;
                df("Error! id %d didn't get ack! last %d",
                        tmp_tpg_p->pk_id, tmp_tpg_p->pk_pending_last);
                if((tmp_tpg_p->pk_pending_last%10) == 0){
                    ret = mynt.net_send((const char*)tmp_tpg_p, sizeof(trans_package));
                    df("re-send last failed package id %d return %d",
                            tmp_tpg_p->pk_id, ret);
                }
            }
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

    //recv handle
    {
        trans_package*tmp_p;
        if(NULL != (tmp_p = tpgm_recv.get_pending_tpg(current_recv_pk_id))){
            df("warning: get pk id %d in cache", current_recv_pk_id);
            memcpy(&tpg, tmp_p, sizeof(trans_package));
            tpgm_recv.put_tpg(current_recv_pk_id);
            current_recv_pk_id++;
            return &tpg;
        }
    }
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
        //df("pending pk id %d", pending_pk_id);
        if(handshake_pending_pk_id == tpg.pk_id){
            //df("handshake pending last=%d, cleared", handshake_pk_pending_last);
            handshake_pk_pending_last = 0;
            handshake_pending_pk_id = 0;
        }
        else if(tpgm.put_tpg(tpg.pk_id)){
            df("pending pk_id %d cleared", tpg.pk_id);
        }
        return NULL;
    }
    if(tpg.pk_id < current_recv_pk_id){
        df("warning: discard pkid %d", tpg.pk_id);
        return NULL;
    }
    else if(tpg.pk_id > current_recv_pk_id && tpg.p_type != HANDSHAKE){
        trans_package*tmp_p = tpgm_recv.get_tpg();
        memcpy(tmp_p, &tpg, sizeof(trans_package));
        tpgm_recv.set_pending(tmp_p);
        df("warning: cache recved buf. current pkid should be %d", current_recv_pk_id);
        return NULL;
    }
    if(tpg.p_type != HANDSHAKE && tpg.p_type != ACK){
        current_recv_pk_id++;
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
        //df("send ack of id %d return %d", tp->pk_id, ret);
    }
    else if(tp->p_type == HANDSHAKE){
        tp->pk_id = current_handshake_id++;
        handshake_pending_pk_id=tp->pk_id;
        handshake_pk_pending_last = 0;
        ret = mynt.net_send((const char*)tp, sizeof(trans_package));
    }
    else{
        tp->pk_id = current_pk_id++;
        df("%s type %d %s id %d", __func__, tp->p_type,
                pk_type_str[tp->p_type], tp->pk_id);
        tp->pk_pending_last = 0;
        memset(&tpg_bak, 0, sizeof(trans_package));
        memcpy(&tpg_bak, (const char*)tp, sizeof(trans_package));
        tpgm.set_pending(tp);
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
        trans_package*t = tpgm.get_tpg();
        t->p_type = pt;
        send_package(t);
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

//tpg_manager

tpg_manager::~tpg_manager()
{
    for(tpg_iter i = pending_tpg_list.begin();
            i != pending_tpg_list.end(); i++){
        free(*i);
        pending_tpg_list.erase(i);
    }
    for(tpg_iter i = free_tpg_list.begin();
            i != free_tpg_list.end(); i++){
        free(*i);
        free_tpg_list.erase(i);
    }
    DeleteCriticalSection(&g_tpgm_critc_sctn);
}

tpg_manager::tpg_manager()
{
    InitializeCriticalSection(&g_tpgm_critc_sctn);
    strcpy(name, "deft");
}

bool tpg_manager::check_pending()
{
    bool ret = false;
    EnterCriticalSection(&g_tpgm_critc_sctn);
    g_tpgit = pending_tpg_list.begin();
    for(tpg_iter i = pending_tpg_list.begin();
            i != pending_tpg_list.end(); i++){
        (*i)->pk_pending_last++;
        if((*i)->pk_pending_last > MAX_PK_PENDING){
            ret = true;
        }
    }
    LeaveCriticalSection(&g_tpgm_critc_sctn);
    return ret;
}

trans_package* tpg_manager::get_pending_tpg(int id)
{
    trans_package* ret = NULL;
    EnterCriticalSection(&g_tpgm_critc_sctn);
    for(tpg_iter i = pending_tpg_list.begin();
            i != pending_tpg_list.end(); i++){
        if((*i)->pk_id == id){
            ret = (*i);
            break;
        }
    }
    LeaveCriticalSection(&g_tpgm_critc_sctn);
    return ret;
}

trans_package* tpg_manager::get_next_pending_tpg()
{
    trans_package* ret = NULL;
    EnterCriticalSection(&g_tpgm_critc_sctn);
    if(g_tpgit != pending_tpg_list.end()){
        ret = *g_tpgit;
        g_tpgit++;
    }
    LeaveCriticalSection(&g_tpgm_critc_sctn);
    return ret;
}

bool tpg_manager::put_tpg(int pk_id)
{
    bool ret = false;
    //df("%s: free %d pending %d pkid %d", __func__,
    //        free_tpg_list.size(), pending_tpg_list.size(), pk_id);
    EnterCriticalSection(&g_tpgm_critc_sctn);
    for(tpg_iter i = pending_tpg_list.begin();
            i != pending_tpg_list.end(); i++){
        if((*i)->pk_id == pk_id){
            free_tpg_list.push_back(*i);
            pending_tpg_list.erase(i);
            ret = true;
            break;
        }
    }
    LeaveCriticalSection(&g_tpgm_critc_sctn);
    //df("%s: free %d pending %d ret %d", __func__,
    //        free_tpg_list.size(), pending_tpg_list.size(), ret);
    return ret;
}

void tpg_manager::set_name(const char*n)
{
    memcpy(name, n, 4);
    name[4]=0;
}

void tpg_manager::set_pending(trans_package*tpp)
{
    EnterCriticalSection(&g_tpgm_critc_sctn);
    pending_tpg_list.push_back(tpp);
    LeaveCriticalSection(&g_tpgm_critc_sctn);
}

trans_package* tpg_manager::get_tpg()
{
    trans_package*ret;
    df("tpg_manager %s: free %d pending %d", name,
            free_tpg_list.size(), pending_tpg_list.size());
    EnterCriticalSection(&g_tpgm_critc_sctn);
    if(!free_tpg_list.empty()){
        ret = free_tpg_list.front();
        free_tpg_list.pop_front();
    }
    else{
        ret = (trans_package*)malloc(sizeof(trans_package));
    }
    LeaveCriticalSection(&g_tpgm_critc_sctn);
    memset(ret, 0, sizeof(trans_package));
    return ret;
}
