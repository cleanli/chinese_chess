#include "text_rc.h"

//GBK
const char GBK_text_app_title[]={
    (char)0xd6,(char)0xd0,(char)0xb9,(char)0xfa,(char)0xcf,(char)0xf3,(char)0xc6,(char)0xe5,
    (char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_start_up[]={
    (char)0xc6,(char)0xf4,(char)0xb6,(char)0xaf,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_local[]={
    (char)0xb1,(char)0xbe,(char)0xb5,(char)0xd8,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_server[]={
    (char)0xb7,(char)0xfe,(char)0xce,(char)0xf1,(char)0xc6,(char)0xf7,(char)0x00,(char)0x00,
};
const char GBK_text_client[]={
    (char)0xbf,(char)0xcd,(char)0xbb,(char)0xa7,(char)0xb6,(char)0xcb,(char)0x00,(char)0x00,
};
const char GBK_text_switch[]={
    (char)0xbd,(char)0xbb,(char)0xbb,(char)0xbb,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_start[]={
    (char)0xbf,(char)0xaa,(char)0xca,(char)0xbc,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_drawn[]={
    (char)0xba,(char)0xcd,(char)0xc6,(char)0xe5,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_give[]={
    (char)0xc8,(char)0xcf,(char)0xca,(char)0xe4,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_load[]={
    (char)0xc6,(char)0xe5,(char)0xc6,(char)0xd7,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_review[]={
    (char)0xb8,(char)0xb4,(char)0xc5,(char)0xcc,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_next[]={
    (char)0xc7,(char)0xb0,(char)0xbd,(char)0xf8,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_prev[]={
    (char)0xba,(char)0xf3,(char)0xcd,(char)0xcb,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_message_switch[]={
    (char)0xb6,(char)0xd4,(char)0xb7,(char)0xbd,(char)0xc7,(char)0xeb,(char)0xc7,(char)0xf3,
    (char)0xbd,(char)0xbb,(char)0xbb,(char)0xbb,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_message_drawn[]={
    (char)0xb6,(char)0xd4,(char)0xb7,(char)0xbd,(char)0xc7,(char)0xf3,(char)0xba,(char)0xcd,
    (char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_message_give[]={
    (char)0xb6,(char)0xd4,(char)0xb7,(char)0xbd,(char)0xc8,(char)0xcf,(char)0xca,(char)0xe4,
    (char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};
const char GBK_text_message_leave[]={
    (char)0xb6,(char)0xd4,(char)0xb7,(char)0xbd,(char)0xd2,(char)0xd1,(char)0xc0,(char)0xeb,
    (char)0xbf,(char)0xaa,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,(char)0x00,
};

//eng
const char* eng_text_app_title="Chinese Chess";
const char* eng_text_start_up="Start Up";
const char* eng_text_local="Local";
const char* eng_text_server="Server";
const char* eng_text_client="Client";
const char* eng_text_switch="Switch";
const char* eng_text_start="Start";
const char* eng_text_drawn="Drawn";
const char* eng_text_give="Give";
const char* eng_text_load="Load";
const char* eng_text_review="Review";
const char* eng_text_next="Next";
const char* eng_text_prev="Prev";
const char* eng_text_message_switch="Remote request Switch";
const char* eng_text_message_drawn="Remote request Drawn";
const char* eng_text_message_give="Remote request Give";
const char* eng_text_message_leave="Remote has left";

text_rc GBK_tr={
    GBK_text_app_title,
    GBK_text_start_up,
    GBK_text_local,
    GBK_text_server,
    GBK_text_client,
    GBK_text_switch,
    GBK_text_start,
    GBK_text_drawn,
    GBK_text_give,
    GBK_text_load,
    GBK_text_review,
    GBK_text_next,
    GBK_text_prev,
    GBK_text_message_switch,
    GBK_text_message_drawn,
    GBK_text_message_give,
    GBK_text_message_leave,
};

text_rc eng_tr={
    eng_text_app_title,
    eng_text_start_up,
    eng_text_local,
    eng_text_server,
    eng_text_client,
    eng_text_switch,
    eng_text_start,
    eng_text_drawn,
    eng_text_give,
    eng_text_load,
    eng_text_review,
    eng_text_next,
    eng_text_prev,
    eng_text_message_switch,
    eng_text_message_drawn,
    eng_text_message_give,
    eng_text_message_leave,
};
