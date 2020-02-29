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

int chess_piece::dist_sq(int dx, int dy)
{
    return dx*dx+dy*dy;
}

bool chess_piece_king::can_goto_point(int p_x, int p_y)
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
    if(chess_piece::dist_sq((p_x-current_x),(p_y-current_y)) != 1){
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
    return true;
}

bool chess_piece_knight::can_goto_point(int p_x, int p_y)
{
    return true;
}

bool chess_piece_rook::can_goto_point(int p_x, int p_y)
{
    return true;
}

bool chess_piece_cannon::can_goto_point(int p_x, int p_y)
{
    return true;
}

bool chess_piece_pawn::can_goto_point(int p_x, int p_y)
{
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

chess_game::chess_game(int timeout)
  : saved_timeout(timeout),
    running_state(INIT_STATE),
    choosen_cp(NULL),
    red_request_drawn(false),
    black_request_drawn(false),
    current_playing_side(SIDE_RED)
{
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i] = create_cp(&cp_create_map[i]);
    }
}

move_step*chess_game::get_lastmove()
{
    if(running_step == 0)return NULL;
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
    ps == SIDE_RED?red_request_drawn = true:black_request_drawn = true;
    if(red_request_drawn && black_request_drawn){
        running_state = END_STATE;
        playresult=RESULT_DRAWN;
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

void chess_game::reset()
{
    for(int i = 0; i<MAX_CHS_BOARD_Y;i++){
        for(int j = 0; j<MAX_CHS_BOARD_X;j++){
            cpes_board[i][j] = NULL;
        }
    }
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i]->set_alive(true);
        cpes[i]->moveto(cp_create_map[i].cp_x, cp_create_map[i].cp_y);
        cpes_board[cp_create_map[i].cp_y][cp_create_map[i].cp_x] = cpes[i];
    }
    running_state = INIT_STATE;
    current_playing_side=SIDE_RED;
    red_timeout = saved_timeout*10;
    black_timeout = saved_timeout*10;
    running_step = 0;
    choosen_cp = NULL;
    red_request_drawn = false;
    black_request_drawn = false;
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

bool chess_game::moveto_point(int x, int y)
{
    if(cpes_board[y][x] != NULL &&
            current_playing_side == cpes_board[y][x]->get_cp_side()){
        choosen_cp = cpes_board[y][x];
        return true;
    }
    else if(choosen_cp->can_goto_point(x,y)){
        if(cpes_board[y][x] != NULL){
            cpes_board[y][x]->set_alive(false);
            if(cpes_board[y][x]->get_cpid() == CP_RED_KING){
                set_win(SIDE_BLACK);
            }
            if(cpes_board[y][x]->get_cpid() == CP_BLACK_KING){
                set_win(SIDE_RED);
            }
        }
        cpes_board[choosen_cp->get_p_y()][choosen_cp->get_p_x()] = NULL;
        lastmove.x1=choosen_cp->get_p_x();
        lastmove.y1=choosen_cp->get_p_y();
        lastmove.x2=x;
        lastmove.y2=y;
        choosen_cp->moveto(x,y);
        cpes_board[y][x] = choosen_cp;
        switch_turn();
        choosen_cp = NULL;
        running_step++;
        return true;
    }
    else{
        return false;
    }
        return false;
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

void chess_game::set_win(PLAYING_SIDE sd)
{
    if(running_state != PLAYING_STATE)return;
    running_state = END_STATE;
    (sd == SIDE_RED)?playresult=RESULT_RED_WIN:playresult=RESULT_BLACK_WIN;
}

void chess_game::timer_click()
{
    if(running_state == PLAYING_STATE){
        (current_playing_side== SIDE_RED)?red_timeout--:black_timeout--;
    }
    if(!red_timeout)set_win(SIDE_BLACK);
    if(!black_timeout)set_win(SIDE_RED);
}

