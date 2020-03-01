#ifndef _DEBUG_H
#define _DEBUG_H

#define df(fmt,arg...) \
    { \
        printf(fmt"\n", ##arg); \
        fflush(stdout); \
    }
#endif
