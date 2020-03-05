#include "config.h"
#include "string.h"
#include "stdio.h"
#include "debug.h"

ch_config::ch_config()
    :timeout(300),
    log(0)
{
    strcpy(ip, "127.0.0.1");
};

void ch_config::save_config(){
    FILE* f = fopen("chess_config", "w");
    if(!f){
        printf("open config fail\n");
        return;
    }
    fprintf(f, "ip=(%s)\n", ip);
    fprintf(f, "timeout=%d\n", timeout);
    fprintf(f, "log=%d\n", log);
    fclose(f);
}
void ch_config::get_config(){
    char lineBuf[256];
    FILE* f = fopen("chess_config", "r");
    if(!f){
        printf("open config fail\n");
        return;
    }
    fgets(lineBuf, 256, f);//
    char* pTmp = strchr(lineBuf, '=');
    char* pTmp2 = strchr(lineBuf, ')');
    *pTmp2 = 0;
    strcpy(ip, pTmp+2);
    fgets(lineBuf, 256, f);//
    sscanf(lineBuf, "timeout=%d\n", &timeout);
    sscanf(lineBuf, "log=%d\n", &log);
    fclose(f);
    df("get ip=%s", ip);
    df("get timeout=%d", timeout);
    df("get log=%d", log);
}
