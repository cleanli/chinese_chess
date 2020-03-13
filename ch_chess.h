#ifndef _CH_CHESS
#define _CH_CHESS

#include"debug.h"

#define MAX_CHS_BOARD_X 9
#define MAX_CHS_BOARD_Y 10
#define RED_PAWN_LINE_Y 4
#define BLACK_PAWN_LINE_Y 5
#define MAX_MOVES_NUM 2048

const char GBK_empty[2] = {(char)0x00, (char)0x00};
const char GBK_win[2] = {(char)0xca, (char)0xa4};
const char GBK_give[4] = {(char)0xc8, (char)0xcf, (char)0xb8, (char)0xba};
const char GBK_timeout[4] = {(char)0xb3, (char)0xac, (char)0xca, (char)0xb1};
const char GBK_drawn_result[4] = {(char)0xba, (char)0xcd, (char)0xc6, (char)0xe5};
const char GBK_forward[2] = {(char)0xdf, (char)0x4d};
const char GBK_back[2] = {(char)0xcd, (char)0xcb};
const char GBK_horizontal[2] = {(char)0xc6, (char)0xbd};
const char GBK_front[2] = {(char)0xc7, (char)0xb0};
const char GBK_behind[2] = {(char)0xba, (char)0xf3};
const char GBK_middle[2] = {(char)0xd6, (char)0xd0};
const char GBK_king[2][2] = {
    {(char)0xcb, (char)0xa7},
    {(char)0xbd, (char)0xab},
};
const char GBK_guard[2][2] = {
    {(char)0xca, (char)0xcb},
    {(char)0xca, (char)0xbf},
};
const char GBK_minister[2][2] = {
    {(char)0xcf, (char)0xf3},
    {(char)0xcf, (char)0xe0},
};
const char GBK_knight[2][2] = {
    {(char)0xf1, (char)0x52},
    {(char)0xf1, (char)0x52},
};
const char GBK_rook[2][2] = {
    {(char)0xdc, (char)0x87},
    {(char)0xdc, (char)0x87},
};
const char GBK_cannon[2][2] = {
    {(char)0xc5, (char)0xda},
    {(char)0xb3, (char)0x68},
};
const char GBK_pawn[2][2] = {
    {(char)0xb1, (char)0xf8},
    {(char)0xd7, (char)0xe4},
};
const char GBK_side[2][2] = {
    {(char)0xba, (char)0xec},
    {(char)0xba, (char)0xda},
};
const char GBK_number[2][9][2] = {
    {
        {(char)0xd2, (char)0xbb},
        {(char)0xb6, (char)0xfe},
        {(char)0xc8, (char)0xfd},
        {(char)0xcb, (char)0xc4},
        {(char)0xce, (char)0xe5},
        {(char)0xc1, (char)0xf9},
        {(char)0xc6, (char)0xdf},
        {(char)0xb0, (char)0xcb},
        {(char)0xbe, (char)0xc5},
    },
    {
        {(char)0xa3, (char)0xb1},
        {(char)0xa3, (char)0xb2},
        {(char)0xa3, (char)0xb3},
        {(char)0xa3, (char)0xb4},
        {(char)0xa3, (char)0xb5},
        {(char)0xa3, (char)0xb6},
        {(char)0xa3, (char)0xb7},
        {(char)0xa3, (char)0xb8},
        {(char)0xa3, (char)0xb9},
    },
};

enum WIN_REASON
{
    KING_KILL,
    OPPONENT_GIVE,
    OPPONENT_TIMEOUT,
};

enum RUN_STATE
{
    UNINIT_STATE = -1,
    INIT_STATE = 0,
    PLAYING_STATE,
    END_STATE,
    REVIEW_STATE
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
    const char* ch_name;
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
        chess_piece*dead_link;
        int dead_step;
        const char*chinese_name;
    protected:
        int current_x;
        int current_y;
        bool isalive;
        chess_game *chg;
        PLAYING_SIDE pside;
        CHESS_PIECES_INDEX cpid;
        int dist_sq(int dx, int dy);
        int num_between(int, int);
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
        void review_reset();
        void review_next();
        void review_prev();
        void start();
        void set_timer_pause();
        void set_win(PLAYING_SIDE, WIN_REASON);
        void set_timeout(PLAYING_SIDE, int t);
        void set_idleside_timeout(int t);
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
        void set_starttime(char*);
        char* save_hint();
        char* get_save_line();
        bool read_step(const char*);
        bool is_saved();
        ~chess_game();
    private:
        bool is_pause;
        chess_piece* dead_link_head;
        chess_piece* choosen_cp;
        chess_piece* cpes[CP_NUM_MAX];
        chess_piece* cpes_board[MAX_CHS_BOARD_Y][MAX_CHS_BOARD_X];
        PLAYING_SIDE current_playing_side;
        int running_step;
        int black_saved_timeout;
        int red_saved_timeout;
        int red_timeout;
        int black_timeout;
        bool red_request_drawn;
        bool black_request_drawn;
        RUN_STATE running_state;
        PLAYING_RESULT playresult;
        move_step lastmove;
        unsigned short move_steps_record[MAX_MOVES_NUM];
        char chinese_move_steps[MAX_MOVES_NUM][9];
        void dump_steps();
        chess_piece* push_dead_cp(chess_piece*cp);
        chess_piece* pop_dead_cp();
        chess_piece* get_next_dead();
        char starttime[128];
        char save_line[64];
        bool saved;
        const cp_create_info cp_create_map[CP_NUM_MAX]={
            {CP_TYPE_ROOK,     SIDE_RED,  0, 0,  CP_RED_R_ROOK,      GBK_rook[0]},
            {CP_TYPE_KNIGHT,   SIDE_RED,  1, 0,  CP_RED_R_KNIGHT,    GBK_knight[0]},
            {CP_TYPE_MINISTER, SIDE_RED,  2, 0,  CP_RED_R_MINISTER,  GBK_minister[0]},
            {CP_TYPE_GUARD,    SIDE_RED,  3, 0,  CP_RED_R_GUARD,     GBK_guard[0]},
            {CP_TYPE_KING,     SIDE_RED,  4, 0,  CP_RED_KING,        GBK_king[0]},
            {CP_TYPE_GUARD,    SIDE_RED,  5, 0,  CP_RED_L_GUARD,     GBK_guard[0]},
            {CP_TYPE_MINISTER, SIDE_RED,  6, 0,  CP_RED_L_MINISTER,  GBK_minister[0]},
            {CP_TYPE_KNIGHT,   SIDE_RED,  7, 0,  CP_RED_L_KNIGHT,    GBK_knight[0]},
            {CP_TYPE_ROOK,     SIDE_RED,  8, 0,  CP_RED_L_ROOK,      GBK_rook[0]},
            {CP_TYPE_CANNON,   SIDE_RED,  1, 2,  CP_RED_R_CANNON,    GBK_cannon[0]},
            {CP_TYPE_CANNON,   SIDE_RED,  7, 2,  CP_RED_L_CANNON,    GBK_cannon[0]},
            {CP_TYPE_PAWN,     SIDE_RED,  0, 3,  CP_RED_R_PAWN,      GBK_pawn[0]},
            {CP_TYPE_PAWN,     SIDE_RED,  2, 3,  CP_RED_RM_PAWN,     GBK_pawn[0]},
            {CP_TYPE_PAWN,     SIDE_RED,  4, 3,  CP_RED_M_PAWN,      GBK_pawn[0]},
            {CP_TYPE_PAWN,     SIDE_RED,  6, 3,  CP_RED_LM_PAWN,     GBK_pawn[0]},
            {CP_TYPE_PAWN,     SIDE_RED,  8, 3,  CP_RED_L_PAWN,      GBK_pawn[0]},

            {CP_TYPE_ROOK,     SIDE_BLACK, 8, 9,  CP_BLACK_R_ROOK,     GBK_rook[1]},
            {CP_TYPE_KNIGHT,   SIDE_BLACK, 7, 9,  CP_BLACK_R_KNIGHT,   GBK_knight[1]},
            {CP_TYPE_MINISTER, SIDE_BLACK, 6, 9,  CP_BLACK_R_MINISTER, GBK_minister[1]},
            {CP_TYPE_GUARD,    SIDE_BLACK, 5, 9,  CP_BLACK_R_GUARD,    GBK_guard[1]},
            {CP_TYPE_KING,     SIDE_BLACK, 4, 9,  CP_BLACK_KING,       GBK_king[1]},
            {CP_TYPE_GUARD,    SIDE_BLACK, 3, 9,  CP_BLACK_L_GUARD,    GBK_guard[1]},
            {CP_TYPE_MINISTER, SIDE_BLACK, 2, 9,  CP_BLACK_L_MINISTER, GBK_minister[1]},
            {CP_TYPE_KNIGHT,   SIDE_BLACK, 1, 9,  CP_BLACK_L_KNIGHT,   GBK_knight[1]},
            {CP_TYPE_ROOK,     SIDE_BLACK, 0, 9,  CP_BLACK_L_ROOK,     GBK_rook[1]},
            {CP_TYPE_CANNON,   SIDE_BLACK, 7, 7,  CP_BLACK_R_CANNON,   GBK_cannon[1]},
            {CP_TYPE_CANNON,   SIDE_BLACK, 1, 7,  CP_BLACK_L_CANNON,   GBK_cannon[1]},
            {CP_TYPE_PAWN,     SIDE_BLACK, 8, 6,  CP_BLACK_R_PAWN,     GBK_pawn[1]},
            {CP_TYPE_PAWN,     SIDE_BLACK, 6, 6,  CP_BLACK_RM_PAWN,    GBK_pawn[1]},
            {CP_TYPE_PAWN,     SIDE_BLACK, 4, 6,  CP_BLACK_M_PAWN,     GBK_pawn[1]},
            {CP_TYPE_PAWN,     SIDE_BLACK, 2, 6,  CP_BLACK_LM_PAWN,    GBK_pawn[1]},
            {CP_TYPE_PAWN,     SIDE_BLACK, 0, 6,  CP_BLACK_L_PAWN,     GBK_pawn[1]},
        };
};

#endif
