#include "ch_chess.h"

#ifndef NULL 
#define NULL 0
#endif

chess_piece::chess_piece(chess_game* games, int p_x, int p_y, PLAYING_SIDE side)
    :isalive(true),
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
            return new chess_piece_king(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_GUARD:
            return new chess_piece_guard(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_MINISTER:
            return new chess_piece_minister(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_KNIGHT:
            return new chess_piece_knight(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_ROOK:
            return new chess_piece_rook(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_CANNON:
            return new chess_piece_cannon(this, cci->cp_x, cci->cp_y, cci->cp_sd);
        case CP_TYPE_PAWN:
            return new chess_piece_pawn(this, cci->cp_x, cci->cp_y, cci->cp_sd);
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
    current_playing_side(SIDE_RED)
{
    for(int i = 0; i<CP_NUM_MAX;i++){
        cpes[i] = create_cp(&cp_create_map[i]);
    }
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
}

bool chess_game::choose_point(int x, int y)
{
    return true;
}

bool chess_game::moveto_point(int x, int y)
{
    return true;
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
}

