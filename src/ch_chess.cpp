#include "ch_chess.h"
#include<iostream>
#include <stdio.h>

#ifndef NULL 
#define NULL 0
#endif

chess_piece::chess_piece(chess_game* games, int p_x, int p_y, PLAYING_SIDE side, CHESS_PIECES_INDEX id)
    :isalive(true),
    cpid(id),
    pside(side)
{
    chg = games;
    current_x = p_x;
    current_y = p_y;
}

int chess_piece::get_p_x()
{
    return current_x;
}

int chess_piece::get_p_y()
{
    return current_y;
}

PLAYING_SIDE chess_piece::get_cp_side()
{
    return pside;
}

CHESS_PIECES_INDEX chess_piece::get_cpid()
{
    return cpid;
}

bool chess_piece::moveto(int x, int y)
{
    current_x = x;
    current_y = y;
    return true;
}

void chess_piece::set_alive(bool b)
{
    isalive = b;
}

bool chess_piece::is_alive()
{
    return isalive;
}

bool chess_piece::can_goto_point(int p_x, int p_y)
{
    if(chg->get_cp(p_x, p_y)!=NULL &&
            pside == chg->get_cp(p_x, p_y)->get_cp_side()){
        return false;
    }
    return true;
}

int chess_piece::num_between(int x, int y)
{
    int cmin, cmax;
    int ret = 0;
    if(((current_x!=x)&&(current_y!=y))||
            ((current_x==x)&&(current_y==y))){
        return -1;
    }
    if(x == current_x){
        cmin = (y > current_y)?current_y:y;
        cmax = (y > current_y)?y:current_y;
        for(int c = cmin+1; c < cmax; c++){
            if(chg->get_cp(x, c)!=NULL){
                ret++;
            }
        }
    }
    if(y == current_y){
        cmin = (x > current_x)?current_x:x;
        cmax = (x > current_x)?x:current_x;
        for(int c = cmin+1; c < cmax; c++){
            if(chg->get_cp(c, y)!=NULL){
                ret++;
            }
        }
    }
    return ret;
}

int chess_piece::dist_sq(int dx, int dy)
{
    return dx*dx+dy*dy;
}

bool chess_piece_king::can_goto_point(int p_x, int p_y)
{
    if(chg->get_cp(p_x,p_y)!=NULL &&
            (chg->get_cp(p_x,p_y)->get_cpid() == CP_RED_KING ||
            chg->get_cp(p_x,p_y)->get_cpid() == CP_BLACK_KING)&&
            current_x == p_x && num_between(p_x, p_y) == 0){
        return true;
    }
    if(!chess_piece::can_goto_point(p_x, p_y)){
        return false;
    }
    if(p_x<3 || p_x>5){
        return false;
    }
    if((pside==SIDE_RED && p_y>2) || (pside==SIDE_BLACK && p_y<7)){
        return false;
    }
    if(dist_sq((p_x-current_x),(p_y-current_y)) != 1){
        return false;
    }
    return true;
}

bool chess_piece_guard::can_goto_point(int p_x, int p_y)
{
    if(!chess_piece::can_goto_point(p_x, p_y)){
        return false;
    }
    if(p_x<3 || p_x>5){
        return false;
    }
    if((pside==SIDE_RED && p_y>2) || (pside==SIDE_BLACK && p_y<7)){
        return false;
    }
    if(dist_sq((p_x-current_x),(p_y-current_y)) != 2){
        return false;
    }
    return true;
}

bool chess_piece_minister::can_goto_point(int p_x, int p_y)
{
    int xz, yz;
    if(!chess_piece::can_goto_point(p_x, p_y)){
        return false;
    }
    if((pside==SIDE_RED && p_y>4) || (pside==SIDE_BLACK && p_y<5)){
        return false;
    }
    if(dist_sq((p_x-current_x),(p_y-current_y)) != 8){
        return false;
    }
    xz = current_x + (p_x - current_x)/2;
    yz = current_y + (p_y - current_y)/2;
    if(chg->get_cp(xz, yz) != NULL){
        return false;
    }
    return true;
}

bool chess_piece_knight::can_goto_point(int p_x, int p_y)
{
    int xz, yz;
    if(dist_sq((p_x-current_x),(p_y-current_y)) != 5){
        return false;
    }
    xz = current_x + (p_x - current_x)/2;
    yz = current_y + (p_y - current_y)/2;
    if(chg->get_cp(xz, yz) != NULL){
        return false;
    }
    return true;
}

bool chess_piece_rook::can_goto_point(int p_x, int p_y)
{
    if(num_between(p_x, p_y) == 0){
        return true;
    }
    else{
        return false;
    }
}

bool chess_piece_cannon::can_goto_point(int p_x, int p_y)
{
    int ib = num_between(p_x, p_y);
    if((ib == 0 && chg->get_cp(p_x, p_y)==NULL)||
            (ib == 1 && chg->get_cp(p_x, p_y)!=NULL
             && chg->get_cp(p_x, p_y)->get_cp_side()!=pside)
            ){
        return true;
    }
    else{
        return false;
    }
    return true;
}

bool chess_piece_pawn::can_goto_point(int p_x, int p_y)
{
    if(!chess_piece::can_goto_point(p_x, p_y)){
        return false;
    }
    if(p_x == 1 || p_x == 3 || p_x == 5 || p_x == 7){
        if((pside == SIDE_RED && p_y < 5) ||
                (pside == SIDE_BLACK && p_y > 4)){
            return false;
        }
    }
    if((pside==SIDE_RED&&p_y<current_y) ||
            (pside==SIDE_BLACK&&p_y>current_y)){
        return false;
    }
    if(dist_sq((p_x-current_x),(p_y-current_y)) != 1){
        return false;
    }
    return true;
}

chess_piece* chess_game::create_cp(const cp_create_info*cci)
{
    switch(cci->cp_tp){
        case CP_TYPE_KING:
            return new chess_piece_king(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_GUARD:
            return new chess_piece_guard(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_MINISTER:
            return new chess_piece_minister(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_KNIGHT:
            return new chess_piece_knight(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_ROOK:
            return new chess_piece_rook(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_CANNON:
            return new chess_piece_cannon(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
        case CP_TYPE_PAWN:
            return new chess_piece_pawn(this, cci->cp_x, cci->cp_y, cci->cp_sd, cci->cp_id);
    }
    return NULL;
}

chess_game::~chess_game()
{
    for(int i = 0; i<CP_NUM_MAX;i++){
        delete cpes[i];
    }
}

void chess_game::set_starttime(char*st)
{
    df("start time: %s", st);
    sprintf(starttime, "%s.chss", st);
}

bool chess_game::is_saved()
{
    return saved;
}

void chess_game::set_timer_pause(bool s)
{
    is_pause = s;
}

void chess_game::set_timer_pause()
{
    is_pause = !is_pause;
}

char* chess_game::save_hint()
{
    review_reset();
    return starttime;
}

void chess_game::set_show_message(callback_show_message c)
{
    csm = c;
}

bool chess_game::read_step(const char*input)
{
    saved=true;
    running_state = REVIEW_STATE;
    if(running_step >= MAX_MOVES_NUM){
        return false;
    }
    if(!input || input[0]==';'){
        return true;
    }
    //df("run step %d in read func", running_step);
    u_short ms;
    sscanf(input, "%x", &ms);
    move_steps_record[running_step++] = ms;
    if(ms>=0xeeee){
        return false;
    }
    return true;
}

char* chess_game::get_save_line()
{
    if(running_step >= MAX_MOVES_NUM){
        return NULL;
    }
    //df("run step %d in save func", running_step);
    u_short ms= move_steps_record[running_step];
    if(ms>0xeeee){
        saved=true;
        return NULL;
    }
    else{
        if(ms == 0xeeee){
            sprintf(save_line, "%04x;%s", ms,
                    chinese_move_steps[running_step]);
        }
        else{
            sprintf(save_line, "%04x;%s(%04d)", ms,
                    chinese_move_steps[running_step], running_step+1);
        }
        running_step++;
        return save_line;
    }
}

void chess_game::set_idleside_timeout(int t)
{
    if(running_state == PLAYING_STATE)
    {
        if(current_playing_side == SIDE_RED){
            black_timeout = t;
        }
        else{
            red_timeout = t;
        }
    }
}

void chess_game::set_timeout(PLAYING_SIDE sd, int t)
{
    if(running_state == INIT_STATE)
    {
        if(sd == SIDE_RED){
            red_saved_timeout = t;
            red_timeout = t*10;
        }
        else{
            black_saved_timeout = t;
            black_timeout = t*10;
        }
    }
}

chess_game::chess_game(int timeout)
  : red_saved_timeout(timeout),
    black_saved_timeout(timeout),
    running_state(UNINIT_STATE),
    choosen_cp(NULL),
    csm(NULL),
    red_request_drawn(false),
    black_request_drawn(false),
    current_playing_side(SIDE_RED)
{
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i] = create_cp(&cp_create_map[i]);
    }
    memset(starttime, 0, 128);
}

move_step*chess_game::get_lastmove()
{
    if(running_step == 0 &&
            running_state != REVIEW_STATE)
        return NULL;
    return &lastmove;
}

int chess_game::get_timeout(PLAYING_SIDE sd)
{
    switch(sd){
        case SIDE_RED:
            return red_timeout;
            break;
        case SIDE_BLACK:
        default:
            return black_timeout;
    }
}

bool chess_game::request_drawn_side(PLAYING_SIDE ps)
{
    if(running_state != PLAYING_STATE)return false;
    df("player %d request drawn", ps);
    ps == SIDE_RED?red_request_drawn = true:black_request_drawn = true;
    if(red_request_drawn && black_request_drawn){
        df("Drawn done!");
        running_state = END_STATE;
        playresult=RESULT_DRAWN;
        if(running_step < MAX_MOVES_NUM){
            char*ch_steps = chinese_move_steps[running_step];
            memcpy(ch_steps, GBK_drawn_result, 4);
            move_steps_record[running_step]=0xeeee;
        }
        //dump_steps();
        return true;
    }
    return false;
}

void chess_game::start()
{
    running_state = PLAYING_STATE;
}

PLAYING_RESULT chess_game::get_game_result()
{
    return playresult;
}

RUN_STATE chess_game::get_running_state()
{
    return running_state;
}

chess_piece* chess_game::push_dead_cp(chess_piece*cp)
{
    chess_piece*tmpp = dead_link_head;
    if(tmpp==NULL){
        dead_link_head = cp;
        return cp;
    }
    while(tmpp->dead_link != NULL){
        tmpp = tmpp->dead_link;
    }
    tmpp->dead_link = cp;
    return cp;
}

chess_piece* chess_game::pop_dead_cp()
{
    chess_piece*last_tmpp=NULL;
    chess_piece*tmpp = dead_link_head;
    if(tmpp==NULL){
        return NULL;
    }
    while(tmpp->dead_link != NULL){
        last_tmpp = tmpp;
        tmpp = tmpp->dead_link;
    }
    if(NULL!=last_tmpp)
        last_tmpp->dead_link = NULL;
    else
        dead_link_head = NULL;
    return tmpp;
}

chess_piece* chess_game::get_next_dead()
{
    chess_piece*tmpp = dead_link_head;
    if(tmpp==NULL){
        return NULL;
    }
    while(tmpp->dead_link != NULL){
        tmpp = tmpp->dead_link;
    }
    return tmpp;
}

void chess_game::review_prev()
{
    if(running_state != REVIEW_STATE)
        return;

    if(running_step==0){
        df("begining of chess");
        return;
    }
    running_step--;
    unsigned short mt = move_steps_record[running_step];
    int x1 = (0xf000 & mt) >> 12;
    int y1 = (0x0f00 & mt) >> 8;
    int x2 = (0x00f0 & mt) >> 4;
    int y2 = (0x000f & mt) >> 0;
    df("review prev steps %d", running_step);
    df("x1y1x2y2 %d %d %d %d", x1, y1, x2, y2);
    cpes_board[y2][x2]->moveto(x1,y1);
    cpes_board[y1][x1] = cpes_board[y2][x2];
    cpes_board[y2][x2] = NULL;
    if(get_next_dead()!=NULL && get_next_dead()->dead_step == running_step){
        cpes_board[y2][x2]=pop_dead_cp();
        cpes_board[y2][x2]->set_alive(true);
        cpes_board[y2][x2]->moveto(x2,y2);
    }
    lastmove.x1=x1;
    lastmove.y1=y1;
    lastmove.x2=x2;
    lastmove.y2=y2;
    return;
}

void chess_game::review_next()
{
    if(running_state != REVIEW_STATE)
        return;

    unsigned short mt = move_steps_record[running_step];
    if(mt == 0xeeee){
        df("end of chess");
        return;
    }
    df("review next steps %d", running_step);
    int x1 = (0xf000 & mt) >> 12;
    int y1 = (0x0f00 & mt) >> 8;
    int x2 = (0x00f0 & mt) >> 4;
    int y2 = (0x000f & mt) >> 0;
    df("x1y1x2y2 %d %d %d %d", x1, y1, x2, y2);
    if(cpes_board[y2][x2] != NULL){
        cpes_board[y2][x2]->set_alive(false);
        push_dead_cp(cpes_board[y2][x2]);
        cpes_board[y2][x2]->dead_step = running_step;
    }
    cpes_board[y1][x1]->moveto(x2,y2);
    cpes_board[y2][x2] = cpes_board[y1][x1];
    cpes_board[y1][x1] = NULL;
    lastmove.x1=x1;
    lastmove.y1=y1;
    lastmove.x2=x2;
    lastmove.y2=y2;
    running_step++;
    return;
}

void chess_game::review_reset()
{
    for(int i = 0; i<MAX_CHS_BOARD_Y;i++){
        for(int j = 0; j<MAX_CHS_BOARD_X;j++){
            cpes_board[i][j] = NULL;
        }
    }
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i]->chinese_name= cp_create_map[i].ch_name;
        cpes[i]->dead_link = NULL;
        cpes[i]->set_alive(true);
        cpes[i]->moveto(cp_create_map[i].cp_x, cp_create_map[i].cp_y);
        cpes_board[cp_create_map[i].cp_y][cp_create_map[i].cp_x] = cpes[i];
    }
    choosen_cp = NULL;
    dead_link_head = NULL;
    running_step = 0;
    running_state = REVIEW_STATE;
    lastmove.x1=0;
    lastmove.y1=0;
    lastmove.x2=0;
    lastmove.y2=0;
    is_pause = false;
}

void chess_game::reset()
{
    review_reset();
    running_state = INIT_STATE;
    current_playing_side=SIDE_RED;
    red_timeout = red_saved_timeout*10;
    black_timeout = black_saved_timeout*10;
    red_request_drawn = false;
    black_request_drawn = false;
    for(int i = 0;i<MAX_MOVES_NUM;i++){
        move_steps_record[i] = 0xffff;
        memset(chinese_move_steps[i], 0, 9);
    }
    memset(starttime, 0, 128);
    saved=false;
}

bool chess_game::choose_point(int x, int y)
{
    if(cpes_board[y][x] != NULL && current_playing_side == cpes_board[y][x]->get_cp_side()){
        choosen_cp = cpes_board[y][x];
        return true;
    }
    else
        return false;
}

bool chess_game::moveto_point(int x, int y, char**chinese_step)
{
    *chinese_step = NULL;
    if(cpes_board[y][x] != NULL &&
            current_playing_side == cpes_board[y][x]->get_cp_side()){
        choosen_cp = cpes_board[y][x];
        return true;
    }
    else if(choosen_cp->can_goto_point(x,y)){
        lastmove.x1=choosen_cp->get_p_x();
        lastmove.y1=choosen_cp->get_p_y();
        lastmove.x2=x;
        lastmove.y2=y;
        //record the move
        if(running_step < MAX_MOVES_NUM){
            move_steps_record[running_step]=
                lastmove.x1<<12 | lastmove.y1<<8 | lastmove.x2 << 4 | lastmove.y2;
            df("record move %d step: %04x", running_step, move_steps_record[running_step]);
            //chinese move record
            {
                char*ch_steps = chinese_move_steps[running_step];
                *chinese_step = ch_steps;
                int vertical_n;
                int dy=(y>choosen_cp->get_p_y())?y-choosen_cp->get_p_y():choosen_cp->get_p_y()-y;
                ch_steps[0]=choosen_cp->chinese_name[0];
                ch_steps[1]=choosen_cp->chinese_name[1];
                ch_steps[6]=GBK_number[choosen_cp->get_cp_side()][dy-1][0];
                ch_steps[7]=GBK_number[choosen_cp->get_cp_side()][dy-1][1];
                if(choosen_cp->get_cp_side() == SIDE_RED){
                    vertical_n = choosen_cp->get_p_x()+1;
                    if(choosen_cp->get_p_y() < y){
                        ch_steps[4]=GBK_forward[0];
                        ch_steps[5]=GBK_forward[1];
                    }
                    else if(choosen_cp->get_p_y() == y){
                        ch_steps[4]=GBK_horizontal[0];
                        ch_steps[5]=GBK_horizontal[1];
                        ch_steps[6]=GBK_number[choosen_cp->get_cp_side()][x][0];
                        ch_steps[7]=GBK_number[choosen_cp->get_cp_side()][x][1];
                    }
                    else{
                        ch_steps[4]=GBK_back[0];
                        ch_steps[5]=GBK_back[1];
                    }
                }
                else{
                    vertical_n = 9-choosen_cp->get_p_x();
                    if(choosen_cp->get_p_y() > y){
                        ch_steps[4]=GBK_forward[0];
                        ch_steps[5]=GBK_forward[1];
                    }
                    else if(choosen_cp->get_p_y() == y){
                        ch_steps[4]=GBK_horizontal[0];
                        ch_steps[5]=GBK_horizontal[1];
                        ch_steps[6]=GBK_number[choosen_cp->get_cp_side()][8-x][0];
                        ch_steps[7]=GBK_number[choosen_cp->get_cp_side()][8-x][1];
                    }
                    else{
                        ch_steps[4]=GBK_back[0];
                        ch_steps[5]=GBK_back[1];
                    }
                }
                ch_steps[2]=GBK_number[choosen_cp->get_cp_side()][vertical_n-1][0];
                ch_steps[3]=GBK_number[choosen_cp->get_cp_side()][vertical_n-1][1];
                //handle same type cp in same x line
                int before_choosen = 0, after_choosen = 0;
                //df("choosen_cp->get_cpid() %d", choosen_cp->get_cpid());
                for(int i = CP_RED_R_ROOK; i < CP_NUM_MAX; i++){
                    if(i == choosen_cp->get_cpid())
                        continue;
                    //df("cp_create_map[i].cp_tp %d side %d", cp_create_map[i].cp_tp, cpes[i]->get_cp_side());
                    if(cp_create_map[i].cp_tp == cp_create_map[choosen_cp->get_cpid()].cp_tp &&
                        choosen_cp->get_cp_side() == cpes[i]->get_cp_side() &&
                        choosen_cp->get_p_x() == cpes[i]->get_p_x() &&
                        cpes[i]->is_alive()){
                        if(cpes[i]->get_p_y()>choosen_cp->get_p_y()){
                            before_choosen++;
                        }
                        else{
                            after_choosen++;
                        }
                    }
                }
                df("before %d after %d", before_choosen, after_choosen);
                if(choosen_cp->get_cp_side() == SIDE_BLACK){
                    int t = before_choosen;
                    before_choosen =  after_choosen;
                    after_choosen = t;
                }
                int total = before_choosen+after_choosen;
                switch(total){
                    case 0:
                        break;
                    case 1:
                        ch_steps[2]= ch_steps[0];
                        ch_steps[3]= ch_steps[1];
                        if(before_choosen == 1){
                            ch_steps[0]=GBK_behind[0];
                            ch_steps[1]=GBK_behind[1];
                        }
                        else{
                            ch_steps[0]=GBK_front[0];
                            ch_steps[1]=GBK_front[1];
                        }
                        break;
                    case 2:
                        ch_steps[2]= ch_steps[0];
                        ch_steps[3]= ch_steps[1];
                        if(before_choosen == 0){
                            ch_steps[0]=GBK_front[0];
                            ch_steps[1]=GBK_front[1];
                        }
                        else if(before_choosen == 1){
                            ch_steps[0]=GBK_middle[0];
                            ch_steps[1]=GBK_middle[1];
                        }
                        else{
                            ch_steps[0]=GBK_behind[0];
                            ch_steps[1]=GBK_behind[1];
                        }
                        break;
                    case 3:
                    case 4:
                        ch_steps[2]= ch_steps[0];
                        ch_steps[3]= ch_steps[1];
                        if(before_choosen == 0){
                            ch_steps[0]=GBK_front[0];
                            ch_steps[1]=GBK_front[1];
                        }
                        else{
                            ch_steps[0]=GBK_number[choosen_cp->get_cp_side()][before_choosen][0];
                            ch_steps[1]=GBK_number[choosen_cp->get_cp_side()][before_choosen][1];
                        }
                        break;
                    default:
                        break;
                }
                //for guard, minister, knight, last char should be x cordinate
                CHESS_PIECE_TYPE choosen_cp_type = cp_create_map[choosen_cp->get_cpid()].cp_tp;
                if(choosen_cp_type == CP_TYPE_GUARD
                        || choosen_cp_type == CP_TYPE_MINISTER
                        || choosen_cp_type == CP_TYPE_KNIGHT){
                    if(choosen_cp->get_cp_side() == SIDE_RED){
                        ch_steps[6]=GBK_number[choosen_cp->get_cp_side()][x][0];
                        ch_steps[7]=GBK_number[choosen_cp->get_cp_side()][x][1];
                    }
                    else{
                        ch_steps[6]=GBK_number[choosen_cp->get_cp_side()][8-x][0];
                        ch_steps[7]=GBK_number[choosen_cp->get_cp_side()][8-x][1];
                    }
                }
                if(csm){
                    csm(ch_steps);
                }
            }
            //chinese move record end
        }
        running_step++;
        saved=false;
        if(cpes_board[y][x] != NULL){
            cpes_board[y][x]->set_alive(false);
            if(cpes_board[y][x]->get_cpid() == CP_RED_KING){
                set_win(SIDE_BLACK, KING_KILL);
            }
            if(cpes_board[y][x]->get_cpid() == CP_BLACK_KING){
                set_win(SIDE_RED, KING_KILL);
            }
        }
        cpes_board[choosen_cp->get_p_y()][choosen_cp->get_p_x()] = NULL;
        choosen_cp->moveto(x,y);
        cpes_board[y][x] = choosen_cp;
        switch_turn();
        choosen_cp = NULL;
        is_pause = false;
        return true;
    }
    else{
        return false;
    }
    return false;
}

void chess_game::dump_steps()
{
    for(int i = 0;i<MAX_MOVES_NUM;i++){
        df("%d: %04x", i, move_steps_record[i]);
        if(move_steps_record[i]==0xeeee)break;
    }
}

void chess_game::switch_turn()
{
    current_playing_side = (current_playing_side == SIDE_RED)?SIDE_BLACK:SIDE_RED;
}

chess_piece* chess_game::get_choosen_cp()
{
    return choosen_cp;
}

chess_piece* chess_game::get_cp(CHESS_PIECES_INDEX cp_ind)
{
    if(cp_ind>=CP_NUM_MAX)
        return NULL;
    return cpes[cp_ind];
}

chess_piece* chess_game::get_cp(int p_x, int p_y)
{
    return cpes_board[p_y][p_x];
}

PLAYING_SIDE chess_game::get_current_playing_side()
{
    return current_playing_side;
}

void chess_game::set_win(PLAYING_SIDE sd, WIN_REASON rs)
{
    if(running_state != PLAYING_STATE)return;
    df("player %d win", sd);
    running_state = END_STATE;
    (sd == SIDE_RED)?playresult=RESULT_RED_WIN:playresult=RESULT_BLACK_WIN;
    if(running_step < MAX_MOVES_NUM){
        char tmp[16];
        tmp[0] = GBK_side[sd][0];
        tmp[1] = GBK_side[sd][1];
        tmp[2] = GBK_win[0];
        tmp[3] = GBK_win[1];
        tmp[4] = 0;
        move_steps_record[running_step]=0xeeee;
        char*ch_steps = chinese_move_steps[running_step];
        switch(rs){
            case KING_KILL:
                strcpy(ch_steps, tmp);
                break;
            case OPPONENT_GIVE:
                ch_steps[0] = GBK_side[OTHER_SIDE(sd)][0];
                ch_steps[1] = GBK_side[OTHER_SIDE(sd)][1];
                ch_steps[2] = GBK_give[0];
                ch_steps[3] = GBK_give[1];
                ch_steps[4] = GBK_give[2];
                ch_steps[5] = GBK_give[3];
                ch_steps[6] = 0;
                strcat(ch_steps, tmp);
                break;
            case OPPONENT_TIMEOUT:
                ch_steps[0] = GBK_side[OTHER_SIDE(sd)][0];
                ch_steps[1] = GBK_side[OTHER_SIDE(sd)][1];
                ch_steps[2] = GBK_timeout[0];
                ch_steps[3] = GBK_timeout[1];
                ch_steps[4] = GBK_timeout[2];
                ch_steps[5] = GBK_timeout[3];
                ch_steps[6] = 0;
                strcat(ch_steps, tmp);
                break;
        }
        if(csm){
            csm(ch_steps);
        }
    }
    //dump_steps();
}

void chess_game::timer_click()
{
    if(is_pause)
        return;
    if(running_state == PLAYING_STATE){
        (current_playing_side== SIDE_RED)?red_timeout--:black_timeout--;
        if(!red_timeout){
            df("red timeout");
            set_win(SIDE_BLACK, OPPONENT_TIMEOUT);
        }
        if(!black_timeout){
            df("black timeout");
            set_win(SIDE_RED, OPPONENT_TIMEOUT);
        }
    }
}

