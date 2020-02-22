#include "ch_chess.h"

#ifndef null
#define null 0
#endif

chess_piece::chess_piece(chess_game* games, int p_x, int p_y, PLAYING_SIDE side)
    :isalive(true),
    pside(side)
{
    chg = games;
    cp.x = p_x;
    cp.y = p_y;
}

int chess_piece::get_p_x()
{
    return cp.x;
}

int chess_piece::get_p_y()
{
    return cp.y;
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
    return null;
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
    PLAYING_SIDE p_side;
    for(int i = 0; i<CP_NUM_MAX;i++){
        p_side = (i < CP_RB_LINE)?SIDE_RED:SIDE_BLACK;
        cpes[i] = create_cp(&cp_create_map[i]);
    }
}

void chess_game::init()
{
}

bool chess_game::choose_point(chess_point*p)
{
    return true;
}

bool chess_game::moveto_point(chess_point*p)
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

