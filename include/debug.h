#ifndef _DEBUG_H
#define _DEBUG_H

#include "windows.h"
extern char debug_buf[1024];
extern int log_to_file;
#define df(fmt,arg...) \
    { \
        SYSTEMTIME time; \
        DWORD thisid = GetCurrentProcessId();\
        DWORD thistid = GetCurrentThreadId();\
        GetLocalTime(&time);\
        sprintf(debug_buf, "%d%02d%02d-%02d:%02d:%02d.%03d %05d %05d " fmt "\n", \
                time.wYear, time.wMonth, time.wDay, time.wHour, \
                time.wMinute, time.wSecond, time.wMilliseconds, \
                thisid, thistid, ##arg);\
        printf("%s", debug_buf);\
        fflush(stdout); \
        if(log_to_file){char filenm[256];\
        sprintf(filenm, "logs/chess_log_%d.txt", thisid);\
        FILE* f = fopen(filenm, "a");\
        if(!f){printf("open log fail\n");}\
        else {fprintf(f, "%s", debug_buf);\
        fclose(f);}\
        }\
    }
#endif
