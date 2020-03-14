#ifndef _TEXT_RC_H
#define _TEXT_RC_H

struct text_rc {
    const char* text_app_title;
    const char* text_start_up;
    const char* text_local;
    const char* text_server;
    const char* text_client;
    const char* text_switch;
    const char* text_start;
    const char* text_drawn;
    const char* text_give;
    const char* text_load;
    const char* text_review;
    const char* text_next;
    const char* text_prev;
    const char* text_pause;
    const char* text_message_switch;
    const char* text_message_drawn;
    const char* text_message_give;
    const char* text_message_leave;
    const char* text_message_net_error;
    const char* text_message_net_recover;
};

extern text_rc GBK_tr;
extern text_rc eng_tr;
#endif
