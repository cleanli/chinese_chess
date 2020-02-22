#include "ch_chess.h"

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

bool chess_piece_king::can_goto_point(int p_x, int p_y)
{
    return true;
}

bool chess_piece_guard::can_goto_point(int p_x, int p_y)
{
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
  : red_timeout(timeout),
    black_timeout(timeout),
    running_state(INIT_STATE),
    choosen_cp(NULL),
    current_playing_side(SIDE_RED)
{
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i] = create_cp(&cp_create_map[i]);
    }
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
}

bool chess_game::choose_point(int x, int y)
{
    if(current_playing_side == cpes_board[y][x]->get_cp_side()){
        choosen_cp = cpes_board[y][x];
        return true;
    }
    else
        return false;
}

bool chess_game::moveto_point(int x, int y)
{
    return true;
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

void chess_game::timer_click()
{
    if(running_state == PLAYING_STATE){
        (current_playing_side== SIDE_RED)?red_timeout--:black_timeout--;
    }
    if(!red_timeout || !black_timeout){
        running_state = END_STATE;
        if(!red_timeout)playresult=RESULT_BLACK_WIN;
        if(!black_timeout)playresult=RESULT_RED_WIN;
    }
}

