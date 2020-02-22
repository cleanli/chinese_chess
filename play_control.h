#ifndef _PLAY_CONTROL
#define _PLAY_CONTROL
enum RUN_MODE
{
    TBD = 0,
    LOCAL_MODE,
    SERVER_MODE,
    CLIENT_MODE
};

enum HANDLE_TYPE
{
    SCREEN_CLICK_TYPE,
    NET_TYPE,
    HANDLE_TYPE_MAX
};

enum DISPLAY_CHESS_PIECE_TYPE
{
    DCP_RED_KING,
    DCP_RED_GUARD,
    DCP_RED_MINISTER,
    DCP_RED_KNIGHT,
    DCP_RED_ROOK,
    DCP_RED_CANNON,
    DCP_RED_PAWN,
    DCP_BLACK_KING,
    DCP_BLACK_GUARD,
    DCP_BLACK_MINISTER,
    DCP_BLACK_KNIGHT,
    DCP_BLACK_ROOK,
    DCP_BLACK_CANNON,
    DCP_BLACK_PAWN,
    DCP_NUM_MAX
};

DISPLAY_CHESS_PIECE_TYPE cp_display_map[]=
{
    DCP_RED_ROOK,
    DCP_RED_KNIGHT,
    DCP_RED_MINISTER,
    DCP_RED_GUARD,
    DCP_RED_KING,
    DCP_RED_GUARD,
    DCP_RED_MINISTER,
    DCP_RED_KNIGHT,
    DCP_RED_ROOK,
    DCP_RED_CANNON,
    DCP_RED_CANNON,
    DCP_RED_PAWN,
    DCP_RED_PAWN,
    DCP_RED_PAWN,
    DCP_RED_PAWN,
    DCP_RED_PAWN,
    DCP_BLACK_ROOK,
    DCP_BLACK_KNIGHT,
    DCP_BLACK_MINISTER,
    DCP_BLACK_GUARD,
    DCP_BLACK_KING,
    DCP_BLACK_GUARD,
    DCP_BLACK_MINISTER,
    DCP_BLACK_KNIGHT,
    DCP_BLACK_ROOK,
    DCP_BLACK_CANNON,
    DCP_BLACK_CANNON,
    DCP_BLACK_PAWN,
    DCP_BLACK_PAWN,
    DCP_BLACK_PAWN,
    DCP_BLACK_PAWN,
    DCP_BLACK_PAWN,
};

const char* chess_pieces_bmp_path[]=
{
    "chesspieces/red_king.bmp",
    "chesspieces/red_guard.bmp",
    "chesspieces/red_minister.bmp",
    "chesspieces/red_knight.bmp",
    "chesspieces/red_rook.bmp",
    "chesspieces/red_cannon.bmp",
    "chesspieces/red_pawn.bmp",
    "chesspieces/black_king.bmp",
    "chesspieces/black_guard.bmp",
    "chesspieces/black_minister.bmp",
    "chesspieces/black_knight.bmp",
    "chesspieces/black_rook.bmp",
    "chesspieces/black_cannon.bmp",
    "chesspieces/black_pawn.bmp",
};
#define CELL_SIZE (364.0f/8)
#define CHESS_BOARD_X_START 104.0f
#define CHESS_BOARD_Y_START 154.0f

float chess_to_display_x(int cx)
{
    return CHESS_BOARD_X_START + CELL_SIZE * cx;
}

float chess_to_display_y(int cy)
{
    return CHESS_BOARD_Y_START + CELL_SIZE * cy;
}
#endif
