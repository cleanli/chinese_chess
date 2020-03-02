#ifndef _DEBUG_H
#define _DEBUG_H

#include "windows.h"
extern char debug_buf[1024];
#define LEN_TIMESTAMP 28
#define df(fmt,arg...) \
    { \
        SYSTEMTIME time;\
        GetLocalTime(&time);\
        char* full_str = debug_buf + LEN_TIMESTAMP;\
        sprintf(debug_buf, "%d%02d%02d-%02d:%02d:%02d.%03d %05d ", \
                time.wYear, time.wMonth, time.wDay, time.wHour, \
                time.wMinute, time.wSecond, time.wMilliseconds, \
                GetCurrentProcessId());\
        sprintf(full_str, fmt"\n", ##arg);\
        printf("%s", debug_buf);\
        fflush(stdout); \
        FILE* f = fopen("chess_log", "a");\
        if(!f){printf("open log fail\n");}\
        else {fprintf(f, "%s", debug_buf);\
        fclose(f);}\
    }
#endif
