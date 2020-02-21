#include "ch_chess.h"

chess_piece::chess_piece(chess_game* games, int p_x, int p_y)
    :isalive(true)
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
    return false;
}

bool chess_piece_guard::can_goto_point(int p_x, int p_y)
{
    return false;
}

bool chess_piece_minister::can_goto_point(int p_x, int p_y)
{
    return false;
}

bool chess_piece_knight::can_goto_point(int p_x, int p_y)
{
    return false;
}

bool chess_piece_rook::can_goto_point(int p_x, int p_y)
{
    return false;
}

bool chess_piece_cannon::can_goto_point(int p_x, int p_y)
{
    return false;
}

bool chess_piece_pawn::can_goto_point(int p_x, int p_y)
{
    return false;
}

chess_game::chess_game(int timeout)
  : red_timeout(timeout),
    black_timeout(timeout),
    current_playing_side(SIDE_RED)
{

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

