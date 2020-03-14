#ifndef _CONFIG_H
#define _CONFIG_H

class ch_config{
    public:
        ch_config();
        void save_config();
        void get_config();
        char ip[64];
        int timeout;
        int log;
        int language;
};

extern ch_config g_cconfig;

#endif
