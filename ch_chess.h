#ifndef _CH_CHESS
#define _CH_CHESS

#define MAX_CHS_BOARD_X 8
#define MAX_CHS_BOARD_Y 9
#define RED_PAWN_LINE_Y 4
#define BLACK_PAWN_LINE_Y 5

enum PLAYING_RESULT
{
    RESULT_UNKNOWN = 0,
    RESULT_DRAWN,
    RESULT_RED_WIN,
    RESULT_BLACK_WIN,
};

enum PLAYING_SIDE
{
    SIDE_RED = 0,
    SIDE_BLACK,
    SIDE_MAX
};

enum CHESS_PIECE_TYPE
{
    CP_TYPE_KING,
    CP_TYPE_GUARD,
    CP_TYPE_MINISTER,
    CP_TYPE_KNIGHT,
    CP_TYPE_ROOK,
    CP_TYPE_CANNON,
    CP_TYPE_PAWN,
    CP_TYPE_MAX
};

enum CHESS_PIECES_INDEX
{
    CP_RED_R_ROOK,
    CP_RED_R_KNIGHT,
    CP_RED_R_MINISTER,
    CP_RED_R_GUARD,
    CP_RED_KING,//4
    CP_RED_L_GUARD,
    CP_RED_L_MINISTER,
    CP_RED_L_KNIGHT,
    CP_RED_L_ROOK,//8
    CP_RED_R_CANNON,
    CP_RED_L_CANNON,
    CP_RED_R_PAWN,
    CP_RED_RM_PAWN,//12
    CP_RED_M_PAWN,
    CP_RED_LM_PAWN,
    CP_RED_L_PAWN,

    CP_BLACK_R_ROOK,//16
    CP_BLACK_R_KNIGHT,
    CP_BLACK_R_MINISTER,
    CP_BLACK_R_GUARD,
    CP_BLACK_KING,//20
    CP_BLACK_L_GUARD,
    CP_BLACK_L_MINISTER,
    CP_BLACK_L_KNIGHT,
    CP_BLACK_L_ROOK,//24
    CP_BLACK_R_CANNON,
    CP_BLACK_L_CANNON,
    CP_BLACK_R_PAWN,
    CP_BLACK_RM_PAWN,//28
    CP_BLACK_M_PAWN,
    CP_BLACK_LM_PAWN,
    CP_BLACK_L_PAWN,
    CP_NUM_MAX//32
};

struct chess_point{
    int x;
    int y;
};
class chess_game;
class chess_piece{
    public:
        chess_point* get_point();
        bool is_alive();
        virtual bool can_goto_point(chess_point*)=0;
    private:
        chess_point cp;
        bool isalive;
        chess_game *chg;
};

class chess_game{
    public:
        void init();
        bool choose_point(chess_point*);
        bool moveto_point(chess_point*);
    private:
        chess_piece* cpes[CP_NUM_MAX];
        chess_piece* cpes_board[MAX_CHS_BOARD_Y][MAX_CHS_BOARD_X];
};

#endif
