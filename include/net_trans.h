#ifndef NET_TRANS_H
#define NET_TRANS_H

#include <windows.h>
#include <iostream>
#include <winsock.h>

#define NO_FLAGS_SET 0
#define MAXBUFLEN 1024
using namespace std;

enum net_trans_status{
    NOT_INITED,
    SERVER_READY,
    CLIENT_READY,
    SERVER_INIT_FAILED,
    CLIENT_INIT_FAILED,
    NET_TRANS_STATUS_MAX
};


class net_trans
{
    public:
        static char *get_local_ip();
        net_trans();
        bool init(u_short port);
        bool init(const char* ip, u_short port);
        bool try_init(const char* ip, u_short port);
        bool get_status();
        bool net_send(const char* buf, int len);
        char* net_recv(int*len, int maxlen);
        bool buf_return();
        bool recv_lost();
        bool deinit();
        static char local_ip[];
        
    private:
        WSADATA Data;
        SOCKADDR_IN serverSockAddr;
        SOCKADDR_IN usingSockAddr;
        SOCKET serverSocket;
        SOCKET usingSocket;
        u_short port;
        bool received_data_lost;
        char buffer_out[MAXBUFLEN];
        char buffer_recv[MAXBUFLEN];
        net_trans_status connect_status;
        bool bufout_ready;
        LONG volatile send_guard;
        LONG volatile recv_guard;
};

#endif
