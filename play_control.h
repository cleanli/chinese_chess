#ifndef _PLAY_CONTROL
#define _PLAY_CONTROL
enum RUN_MODE
{
    TBD = 0,
    LOCAL_MODE,
    SERVER_MODE,
    CLIENT_MODE
};

enum RUN_STATE
{
    INIT_STATE = 0,
    PLAYING_STATE,
    END_STATE
};

#endif
