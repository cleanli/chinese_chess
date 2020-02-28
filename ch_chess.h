#ifndef _CH_CHESS
#define _CH_CHESS

#define MAX_CHS_BOARD_X 9
#define MAX_CHS_BOARD_Y 10
#define RED_PAWN_LINE_Y 4
#define BLACK_PAWN_LINE_Y 5

enum RUN_STATE
{
    INIT_STATE = 0,
    PLAYING_STATE,
    END_STATE
};

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

#define OTHER_SIDE(x) ((x == SIDE_RED)?SIDE_BLACK:SIDE_RED)

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

    CP_RB_LINE,

    CP_BLACK_R_ROOK = CP_RB_LINE,//16
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

struct cp_create_info{
    CHESS_PIECE_TYPE cp_tp;
    PLAYING_SIDE cp_sd;
    int cp_x;
    int cp_y;
    CHESS_PIECES_INDEX cp_id;
};

struct move_step{
    int x1;
    int y1;
    int x2;
    int y2;
};

class chess_game;
class chess_piece{
    public:
        chess_piece(chess_game*,int,int,PLAYING_SIDE,CHESS_PIECES_INDEX id);
        int get_p_x();
        int get_p_y();
        CHESS_PIECES_INDEX get_cpid();
        bool moveto(int x, int y);
        bool is_alive();
        void set_alive(bool);
        virtual bool can_goto_point(int x,int y)=0;
        PLAYING_SIDE get_cp_side();
    private:
        int current_x;
        int current_y;
        bool isalive;
        chess_game *chg;
        PLAYING_SIDE pside;
        CHESS_PIECES_INDEX cpid;
};

#define SUB_CP_DEF(cp_type) \
    class chess_piece_##cp_type : public chess_piece \
    { \
        public: \
        chess_piece_##cp_type(chess_game*cg,int x,int y,PLAYING_SIDE s,CHESS_PIECES_INDEX id) \
            :chess_piece(cg, x, y, s, id){} \
        bool can_goto_point(int, int); \
    }

SUB_CP_DEF(king);
SUB_CP_DEF(guard);
SUB_CP_DEF(minister);
SUB_CP_DEF(knight);
SUB_CP_DEF(rook);
SUB_CP_DEF(cannon);
SUB_CP_DEF(pawn);

class chess_game{
    public:
        chess_game(int timeout_in_01second);
        void reset();
        void start();
        void set_win(PLAYING_SIDE);
        void switch_turn();
        bool request_drawn_side(PLAYING_SIDE);
        bool choose_point(int x, int y);
        bool moveto_point(int x, int y);
        move_step *get_lastmove();
        chess_piece* get_choosen_cp();
        chess_piece* get_cp(int, int);
        chess_piece* get_cp(CHESS_PIECES_INDEX);
        PLAYING_SIDE get_current_playing_side();
        RUN_STATE get_running_state();
        PLAYING_RESULT get_game_result();
        int get_timeout(PLAYING_SIDE);
        void timer_click();
        chess_piece* create_cp(const cp_create_info*);
        ~chess_game();
    private:
        chess_piece* choosen_cp;
        chess_piece* cpes[CP_NUM_MAX];
        chess_piece* cpes_board[MAX_CHS_BOARD_Y][MAX_CHS_BOARD_X];
        PLAYING_SIDE current_playing_side;
        int running_step;
        int saved_timeout;
        int red_timeout;
        int black_timeout;
        bool red_request_drawn;
        bool black_request_drawn;
        RUN_STATE running_state;
        PLAYING_RESULT playresult;
        move_step lastmove;
        const cp_create_info cp_create_map[CP_NUM_MAX]={
            {CP_TYPE_ROOK,     SIDE_RED,  0, 0,  CP_RED_R_ROOK},
            {CP_TYPE_KNIGHT,   SIDE_RED,  1, 0,  CP_RED_R_KNIGHT},
            {CP_TYPE_MINISTER, SIDE_RED,  2, 0,  CP_RED_R_MINISTER},
            {CP_TYPE_GUARD,    SIDE_RED,  3, 0,  CP_RED_R_GUARD},
            {CP_TYPE_KING,     SIDE_RED,  4, 0,  CP_RED_KING},
            {CP_TYPE_GUARD,    SIDE_RED,  5, 0,  CP_RED_L_GUARD},
            {CP_TYPE_MINISTER, SIDE_RED,  6, 0,  CP_RED_L_MINISTER},
            {CP_TYPE_KNIGHT,   SIDE_RED,  7, 0,  CP_RED_L_KNIGHT},
            {CP_TYPE_ROOK,     SIDE_RED,  8, 0,  CP_RED_L_ROOK},
            {CP_TYPE_CANNON,   SIDE_RED,  1, 2,  CP_RED_R_CANNON},
            {CP_TYPE_CANNON,   SIDE_RED,  7, 2,  CP_RED_L_CANNON},
            {CP_TYPE_PAWN,     SIDE_RED,  0, 3,  CP_RED_R_PAWN},
            {CP_TYPE_PAWN,     SIDE_RED,  2, 3,  CP_RED_RM_PAWN},
            {CP_TYPE_PAWN,     SIDE_RED,  4, 3,  CP_RED_M_PAWN},
            {CP_TYPE_PAWN,     SIDE_RED,  6, 3,  CP_RED_LM_PAWN},
            {CP_TYPE_PAWN,     SIDE_RED,  8, 3,  CP_RED_L_PAWN},

            {CP_TYPE_ROOK,     SIDE_BLACK, 8, 9,  CP_BLACK_R_ROOK},
            {CP_TYPE_KNIGHT,   SIDE_BLACK, 7, 9,  CP_BLACK_R_KNIGHT},
            {CP_TYPE_MINISTER, SIDE_BLACK, 6, 9,  CP_BLACK_R_MINISTER},
            {CP_TYPE_GUARD,    SIDE_BLACK, 5, 9,  CP_BLACK_R_GUARD},
            {CP_TYPE_KING,     SIDE_BLACK, 4, 9,  CP_BLACK_KING},
            {CP_TYPE_GUARD,    SIDE_BLACK, 3, 9,  CP_BLACK_L_GUARD},
            {CP_TYPE_MINISTER, SIDE_BLACK, 2, 9,  CP_BLACK_L_MINISTER},
            {CP_TYPE_KNIGHT,   SIDE_BLACK, 1, 9,  CP_BLACK_L_KNIGHT},
            {CP_TYPE_ROOK,     SIDE_BLACK, 0, 9,  CP_BLACK_L_ROOK},
            {CP_TYPE_CANNON,   SIDE_BLACK, 7, 7,  CP_BLACK_R_CANNON},
            {CP_TYPE_CANNON,   SIDE_BLACK, 1, 7,  CP_BLACK_L_CANNON},
            {CP_TYPE_PAWN,     SIDE_BLACK, 8, 6,  CP_BLACK_R_PAWN},
            {CP_TYPE_PAWN,     SIDE_BLACK, 6, 6,  CP_BLACK_RM_PAWN},
            {CP_TYPE_PAWN,     SIDE_BLACK, 4, 6,  CP_BLACK_M_PAWN},
            {CP_TYPE_PAWN,     SIDE_BLACK, 2, 6,  CP_BLACK_LM_PAWN},
            {CP_TYPE_PAWN,     SIDE_BLACK, 0, 6,  CP_BLACK_L_PAWN},
        };
};

#endif
