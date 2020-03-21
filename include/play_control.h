#ifndef _PLAY_CONTROL
#define _PLAY_CONTROL

/*
 * 0.1 local mode works
 * 0.2 net mode works
 * 0.6 config & timeout ok
 * 0.73 init connect in thread
 * 0.81 chess review/save
 * 0.92 read saved chess file
 * 0.95 Chinese language
 * 0.98 net,timer init protection, more chinese
 * 0.995 chinese move step output
 * 0.996 chinese move step output
 * 0.997 add connect reset
#define VERSION "0.998 add timer pause"
#define VERSION "0.999 add icon"
#define VERSION "1.0 add hand shake package detect"
#define VERSION "1.02 output chinese step"
#define VERSION "1.03 update hand shake"
#define VERSION "1.04 add auto recover net func"
#define VERSION "1.06 issue fix"
#define VERSION "1.08 remove some log"
 * */
#define VERSION "1.10 add tpg_manager"
#define PORT_NUM 34567

//display control
#define FIX_WIN_WIDTH 900
#define FIX_WIN_HEIGHT 750
#define QIPAN_LOAD_POINT_X 0
#define QIPAN_LOAD_POINT_Y 0
#define BUILD_MESSAGE_Y (FIX_WIN_HEIGHT-65)
#define RIGHT_CONTROL_X 705
#define RIGHT_CONTROL_Y 5
//display end

#define CONNECT_NOT_STARTED 0
#define CONNECT_WAITING 1
#define CONNECT_DONE 2

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
#endif
