#include "net_trans.h"
#include "debug.h"

char net_trans::local_ip[128]={0};
net_trans::net_trans()
  : connect_status(NOT_INITED),
    bufout_ready(true),
    received_data_lost(false)
{
    memset(buffer_out, 0, MAXBUFLEN);
    memset(buffer_recv, 0, MAXBUFLEN);
}


char* net_trans::get_local_ip()
{
    //1.init wsa
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        return NULL;
    }
    //2.get hostname
    char hostname[256];
    ret = gethostname(hostname, sizeof(hostname));
    if (ret == SOCKET_ERROR)
    {
        return NULL;
    }
    //3.get ip
    HOSTENT* host = gethostbyname(hostname);
    if (host == NULL)
    {
        return NULL;
    }
    //4.return
    memset(local_ip, 0, 128);
    strcpy(local_ip, inet_ntoa(*(in_addr*)*host->h_addr_list));
    return local_ip;
}

bool net_trans::try_init(const char* ip, u_short port)
{
    if(ip == NULL){
        return init(port);
    }
    else{
        return init(ip, port);
    }
}

bool net_trans::init(const char*ip, u_short pt)
{
    unsigned long usingAddr;  
    int status;  
    int numsnt;  
    unsigned long ul = 1;

    connect_status = CLIENT_INIT_FAILED;
    port = pt;
    /* initialize the Windows Socket DLL */  
    status=WSAStartup(MAKEWORD(1, 1), &Data);  
    if (status != 0){
        cerr << "ERROR: WSAStartup unsuccessful" << endl;  
        return false;
    }

    /* convert IP address into in_addr form */  
    usingAddr=inet_addr(ip);  
    /* copy usingAddr into sockaddr_in structure */  
    memcpy(&usingSockAddr.sin_addr, &usingAddr, sizeof(usingAddr));  
    /* specify the port portion of the address */  
    usingSockAddr.sin_port=htons(pt);  
    /* specify the address family as Internet */  
    usingSockAddr.sin_family=AF_INET;  

    /* create a socket */  
    usingSocket=socket(AF_INET, SOCK_STREAM, 0);  
    if (usingSocket == INVALID_SOCKET)  
    {  
        cerr << "ERROR: socket unsuccessful" << endl;  

        status=WSACleanup();  
        if (status == SOCKET_ERROR)  
            cerr << "ERROR: WSACleanup unsuccessful" << endl;  
        return(false);  
    }  

    /* connect to the server */  
    df("connecting to server...");
    status=connect(usingSocket, (LPSOCKADDR)&usingSockAddr, sizeof(usingSockAddr));  
    if (status == SOCKET_ERROR)  
    {  
        df("ERROR: connect failed");

        status=closesocket(usingSocket);  
        if (status == SOCKET_ERROR)  
            cerr << "ERROR: closesocket failed"<< endl;
        status=WSACleanup();  
        if (status == SOCKET_ERROR)  
            cerr << "ERROR: WSACleanup failed"<< endl;
        return(false);  
    }
    df("connect OK");
    ioctlsocket(usingSocket,FIONBIO,(unsigned long *)&ul);
    connect_status = CLIENT_READY;
    return true;
}  
  

bool net_trans::init(u_short pt)
{
    int addrLen=sizeof(SOCKADDR_IN);
    int status;
    int numrcv;
    unsigned long ul = 1;

    connect_status = SERVER_INIT_FAILED;
    port = pt;
    /* initialize the Windows Socket DLL */
    status=WSAStartup(MAKEWORD(1, 1), &Data);
    if (status != 0){
        cerr << "ERROR: WSAStartup unsuccessful" << endl;
        return false;
    }

    /* zero the sockaddr_in structure */
    memset(&serverSockAddr, 0, sizeof(serverSockAddr));

    /* specify the port portion of the address */
    serverSockAddr.sin_port=htons(port);
    /* specify the address family as Internet */
    serverSockAddr.sin_family=AF_INET;
    /* specify that the address does not matter */
    serverSockAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    /* create a socket socket(area, socket type, specific protocol*/
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET){
        cerr << "ERROR: socket unsuccessful" << endl;
        return false;
    }

    /* associate the socket with the address */
    status=bind(serverSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
    if (status == SOCKET_ERROR){
        cerr << "ERROR: bind unsuccessful" << endl;
        return false;
    }

    /* allow the socket to take connections listen*/
    status=listen(serverSocket, 1);
    if (status == SOCKET_ERROR){
        cerr << "ERROR: listen unsuccessful" << endl;
        return false;
    }

    fd_set sock_set;
    FD_ZERO(&sock_set);
    timeval timeout{10, 0};	//wait time(sec,u_sec)

    FD_SET(serverSocket, &sock_set);
    select(0, &sock_set, nullptr, nullptr, &timeout);
    if((FD_ISSET(serverSocket, &sock_set) == 0))
    {
        int sts;
        df("No client connecting...quit");
        sts=closesocket(serverSocket);
        if (sts == SOCKET_ERROR)
            df("close error");;
        sts=WSACleanup();
        if (sts == SOCKET_ERROR)
            df("cleanup error");
        return false;
    }

    /* accept the connection request when one is received */
    usingSocket=accept(serverSocket, (LPSOCKADDR)&usingSockAddr, &addrLen);

    cout << "Got the connection..." << endl;
    connect_status = SERVER_READY;
    ioctlsocket(usingSocket,FIONBIO,(unsigned long *)&ul);
    return true;
}

bool net_trans::deinit()
{
    int sts;
    sts=closesocket(usingSocket);
    if (sts == SOCKET_ERROR)
        df("error closesocket(usingSocket)");
    sts=closesocket(serverSocket);
    if (sts == SOCKET_ERROR)
        df("error closesocket(serverSocket)");
    sts=WSACleanup();
    if (sts == SOCKET_ERROR)
        df("error WSACleanup");
    return true;
    
}

bool net_trans::get_status()
{
    return connect_status;
}

bool net_trans::recv_lost()
{
    return received_data_lost;
}

bool net_trans::buf_return()
{
    bufout_ready = true;
    return true;
#if 0
    if(buf == buffer_out){
        bufout_ready = true;
        return true;
    }
    else{
        return false;
    }
#endif
}

bool net_trans::net_send(const char*buf, int len)
{
   int numsnt=send(usingSocket, buf, len, NO_FLAGS_SET);  
    df("%s %p %d ret %d", __func__, buf, len, numsnt);
   if(numsnt == len)
       return true;
   else
       return false;
}

char* net_trans::net_recv(int*len)
{
   int numrcv=recv(usingSocket, buffer_recv, MAXBUFLEN, NO_FLAGS_SET);
   if ((numrcv == 0) || (numrcv == SOCKET_ERROR)){
       return NULL;
   }
   else{
       df("net_recv:%d", numrcv);
       if(bufout_ready){
           memcpy(buffer_out, buffer_recv, numrcv);
           bufout_ready = false;
           *len=numrcv;
           return buffer_out;
       }
       else{
           df("net_recv:data lost");
           received_data_lost = true;
           return NULL;
       }
   }
}

