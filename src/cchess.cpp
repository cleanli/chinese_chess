#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>
#include <math.h>

#include "play_control.h"
#include "ch_chess.h"
#include "coordinate_trans.h"
#include "remote_player.h"
#include "config.h"
#include "text_rc.h"
#include "rc.h"

//MESS_PRINT
#define MESS_SIZE 480
#define MPBUF_SIZE 512
static char mpbuf0[MPBUF_SIZE];
static char mpbuf1[MPBUF_SIZE];
static char* mpbuf[2]={mpbuf0, mpbuf1};
static int mpbuf_index = 0;
#define MESS_PRINT(fmt,arg...) \
    {   \
        int tmplen; \
        sprintf(mpbuf[mpbuf_index], fmt"\r\n", ##arg); \
        df(fmt, ##arg);\
        tmplen = strlen(mpbuf[mpbuf_index]); \
        memcpy(mpbuf[mpbuf_index]+tmplen, mpbuf[1-mpbuf_index], MESS_SIZE-tmplen); \
        mpbuf[mpbuf_index][MESS_SIZE-1]=0; \
        SetWindowText(MessageHd, mpbuf[mpbuf_index]);\
        mpbuf_index = 1 - mpbuf_index; \
    }

//debug
char debug_buf[1024];

//config
ch_config g_cconfig;
int log_to_file = 0;

//global variable
int check_load_file = 1;
text_rc* gp_text_rc = &eng_tr;
int wait_net_connect = CONNECT_NOT_STARTED;
int handshake_enable = 1;
bool autorecover_enable = true;
HMENU hRoot;
int timer_count=0;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
char strbuf[128];
HINSTANCE hg_app;
HWND mainHd;
HWND editHd;
HWND rb1Hd;
HWND rb2Hd;
HWND rb3Hd;
HWND seripHd;
HWND tmrLocalHd;
HWND tmrRemoHd;
HWND startButtonHd;
HWND Button1Hd;
HWND Button2Hd;
HWND Button3Hd;
HWND Button4Hd;
HWND ButtonPauseHd;
HWND ButtonLoadHd;
HWND ButtonAutoCoverHd;
HWND MessageHd;
RUN_MODE running_mode = TBD;
RUN_STATE running_state = INIT_STATE;
HANDLE_TYPE chess_playing_handle[SIDE_MAX];

chess_game g_chess_game(300);
c_coordinate_trans g_cdtts(false);
PLAYING_SIDE local_player = SIDE_BLACK;
//remote_player* remote_side = new dummy_remote_player();
remote_player* remote_side = new net_remote_player();
int movingx = 100;
int movingy = 100;
LONG volatile timer_guard = 0;

//function declare
void CreateMyMenu();//create menu
void show_message(char*ms);
void save_chess_game();
void mode_init(HWND hwnd);
void message_print(const char *fmt, ...);
void enable_by_id(int id, int enable);
LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
void debug_str_dump(const char*s);
bool CheckFolderExist(const char*strPath);

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    char tBuf[128];
    static RUN_STATE last_game_state = INIT_STATE;
    HWND hwnd;
    timer_count++;
    g_chess_game.timer_click();
    if (lpParam != NULL)
    {
        trans_package* tptmp = NULL;
        hwnd=(HWND)lpParam;

        //player timeout show
        memset(tBuf, 0, 128);
        sprintf(tBuf, "%04.1f    ", (float)g_chess_game.get_timeout(SIDE_BLACK)/10);
        if(local_player == SIDE_BLACK){
            SetWindowText(tmrLocalHd, tBuf);
        }
        else{
            SetWindowText(tmrRemoHd, tBuf);
        }
        memset(tBuf, 0, 128);
        sprintf(tBuf, "%04.1f    ", (float)g_chess_game.get_timeout(SIDE_RED)/10);
        //MESS_PRINT("%d", timer_count);
        //MESS_PRINT("%d", g_chess_game.get_timeout(SIDE_BLACK));
        if(local_player == SIDE_BLACK){
            SetWindowText(tmrRemoHd, tBuf);
        }
        else{
            SetWindowText(tmrLocalHd, tBuf);
        }

        //'start' button
        RUN_STATE rs_tmp = g_chess_game.get_running_state();
        if(rs_tmp != last_game_state){
            if(rs_tmp == END_STATE){
                EnableWindow(Button2Hd, true);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
        last_game_state = rs_tmp;

        //handle connecting state when just start connect
        if(wait_net_connect == CONNECT_WAITING){
            mode_init(hwnd);
        }
        //df("timer ----");

        //handling recv
        InterlockedIncrement(&timer_guard);
        if(remote_side->is_ready() && timer_guard == 1){
            while((tptmp = remote_side->get_recved_ok())!=NULL){
                df("timer:recv remote message id %d", tptmp->pk_id);
                switch(tptmp->p_type){
                    case CHESS_STEP:
                        {
                            bool ret;
                            df("recvstep %d-%d-%d-%d",
                                    tptmp->pd.ch_move_step.x1,
                                    tptmp->pd.ch_move_step.y1,
                                    tptmp->pd.ch_move_step.x2,
                                    tptmp->pd.ch_move_step.y2
                                    );
                            if(PLAYING_STATE == g_chess_game.get_running_state() &&
                                    (NET_TYPE == chess_playing_handle[g_chess_game.get_current_playing_side()])){
                                if(g_chess_game.get_choosen_cp() == NULL){
                                    ret = g_chess_game.choose_point(tptmp->pd.ch_move_step.x1,
                                            tptmp->pd.ch_move_step.y1);
                                }
                                if(ret){
                                    char*tmpcharp;
                                    ret = g_chess_game.moveto_point(tptmp->pd.ch_move_step.x2,
                                            tptmp->pd.ch_move_step.y2, &tmpcharp);
                                    if(tmpcharp){
                                        //MESS_PRINT("%s", tmpcharp);
                                    }
                                }
                                if(ret){
                                    InvalidateRect(hwnd,NULL,TRUE);
                                }
                            }
                            df("remote timeout %d local %d",
                                    tptmp->pd.timeout, g_chess_game.get_timeout(OTHER_SIDE(local_player)));
                            g_chess_game.set_idleside_timeout(tptmp->pd.timeout+10);
                        }
                        break;
                    case REQUEST_DRAWN:
                        if(g_chess_game.request_drawn_side(OTHER_SIDE(local_player))){
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        //MessageBox(hwnd, "Remote request DRAWN", "Notice", MB_ICONQUESTION);
                        MESS_PRINT("%s", gp_text_rc->text_message_drawn);
                        break;
                    case REQUEST_SWITCH:
                        //MessageBox(hwnd, "Remote request switch side", "Notice", MB_ICONQUESTION);
                        MESS_PRINT("%s", gp_text_rc->text_message_switch);
                        break;
                    case REQUEST_GIVE:
                        MESS_PRINT("%s", gp_text_rc->text_message_give);
                        g_chess_game.set_win(local_player, OPPONENT_GIVE);
                        InvalidateRect(hwnd,NULL,TRUE);
                        //MessageBox(hwnd, "Remote request GIVE", "Notice", MB_ICONQUESTION);
                        break;
                    case SET_REMOTE_PLAYER:
                        df("got setremote cmd");
                        local_player = tptmp->pd.remote_side;
                        g_cdtts.set_revert(local_player == SIDE_RED);
                        chess_playing_handle[local_player] = SCREEN_CLICK_TYPE;
                        chess_playing_handle[(local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED] = NET_TYPE;
                        g_chess_game.set_timeout(local_player, g_cconfig.timeout);
                        if(remote_side->is_ready()){
                            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                            tp_tmp->p_type = SET_TIMEOUT;
                            tp_tmp->pd.timeout = g_cconfig.timeout;
                            remote_side->send_package(tp_tmp);
                        }
                        InvalidateRect(hwnd,NULL,TRUE);
                        break;
                    case NETCMD_START_BUTTON:
                        {
                            df("got netcmd start button");
                            RUN_STATE rstmp = g_chess_game.get_running_state();
                            if(END_STATE == rstmp || REVIEW_STATE == rstmp){
                                if(!g_chess_game.is_saved()){
                                    save_chess_game();
                                }
                                g_chess_game.reset();
                                EnableWindow(Button1Hd, true);
                                SetWindowText(Button1Hd, gp_text_rc->text_switch);
                                SetWindowText(Button3Hd, gp_text_rc->text_drawn);
                                SetWindowText(Button4Hd, gp_text_rc->text_give);
                            }
                            else if(INIT_STATE == rstmp){
                                SYSTEMTIME time;
                                GetLocalTime(&time);
                                sprintf(strbuf, "chess_%d%02d%02d_%02d%02d%02d_%03d",
                                        time.wYear, time.wMonth, time.wDay, time.wHour,
                                        time.wMinute, time.wSecond, time.wMilliseconds);
                                g_chess_game.set_starttime(strbuf);
                                g_chess_game.start();
                                EnableWindow(Button2Hd, false);
                                EnableWindow(Button1Hd, false);
                            }
                            InvalidateRect(hwnd,NULL,TRUE);
                            break;
                        }
                    case SET_TIMEOUT:
                        {
                            df("got set timeout");
                            int net_timeout = tptmp->pd.timeout;
                            g_chess_game.set_timeout(OTHER_SIDE(local_player), net_timeout);
                        }
                        break;
                    case TIMER_PAUSE:
                        {
                            df("got timer pause");
                            if(g_chess_game.get_current_playing_side() == local_player){
                                g_chess_game.set_timer_pause();
                            }
                        }
                        break;
                    case APP_QUIT:
                        df("got app quit");
                        //MessageBox(hwnd, "Remote side have left", "Notice", MB_ICONQUESTION);
                        MESS_PRINT("%s", gp_text_rc->text_message_leave);
                        remote_side->set_quit();
                        break;
                    case HANDSHAKE:
                        break;
                    case STRING:
                        MESS_PRINT("remote str:%s", tptmp->pd.str_message);
                        break;
                    default:
                        break;
                }
            }
            //net check by handshake package
            if(handshake_enable){
                static int count = 0;
                static int reset_count = 0;
                static int last_error = 0;
                static int net_ok_count = 0;
                int count_max = 10;
                if(running_mode==CLIENT_MODE){//make server & client not same interval
                    count_max = 18;
                }
                if(count++>count_max){
                    count = 0;
                    //1 second doing things
                    if(remote_side->is_ready()){
                        //df("send hand shake package");
                        remote_side->send_cmd(HANDSHAKE);
                    }
                    //check net error status
                    if(remote_side->get_error_status()){
                        MESS_PRINT("%s", gp_text_rc->text_message_net_error);
                        //debug_str_dump(gp_text_rc->text_message_net_error);
                        //MessageBox(hwnd, "Connection is ERROR", "Notice", MB_ICONQUESTION);
                        last_error = 1;
#if 1
                        df("reset_count %d", reset_count);
                        if(reset_count++ == 0 && autorecover_enable){
                            MESS_PRINT("reset connect");
                            remote_side->reset_connect();
                        }
                        if(running_mode==CLIENT_MODE){
                            count_max = 3;
                        }
                        else{
                            count_max = 10;
                        }
                        if(reset_count > count_max){
                            reset_count = 0;
                        }
#endif
                    }
                }
                if(remote_side->get_error_status()){
                    g_chess_game.set_timer_pause(true);
                    net_ok_count = 0;
                }
                else{
                    if(net_ok_count++>600){
                        MESS_PRINT("NET is OK for 60 seconds");
                        df("set reset_count 0");
                        reset_count = 0;
                        net_ok_count = 0;
                    }
                    if(last_error == 1){
                        MESS_PRINT("%s", gp_text_rc->text_message_net_recover);
                        //debug_str_dump(gp_text_rc->text_message_net_recover);
                        last_error = 0;
                        g_chess_game.set_timer_pause(false);
                    }
                }
            }
        }
        InterlockedDecrement(&timer_guard);
        //message_print("%d", timer_count);
        //MESS_PRINT("%d", timer_count);
#if 0
        movingx++;
        movingy++;
        RECT rctA = {(LONG)(movingx-1),(LONG)(movingy-1),(LONG)(movingx+CELL_SIZE),(LONG)(movingy+CELL_SIZE)};
        InvalidateRect(hwnd,&rctA,TRUE);
        rctA = {(LONG)(500-movingx+1),(LONG)(movingy-1),(LONG)(500-movingx+CELL_SIZE+9),(LONG)(movingy+CELL_SIZE)};
        InvalidateRect(hwnd,&rctA,TRUE);
#endif
    }
}

void debug_str_dump(const char*s)
{
    const char*p = s;
    while(*p != 0){
        df("(char)0x%02x,",*p++);
    }
}

void mode_init(HWND hwnd)
{
    static volatile LONG func_guard = 0;
    int start_init = 0;
    int cur_net_init_state = remote_side->get_init_state();

    InterlockedIncrement(&func_guard);
    if(func_guard>1){
        df("mode_init is running, quit");
        InterlockedDecrement(&func_guard);
        return;
    }

    if(running_mode == LOCAL_MODE){
        start_init = 1;
    }
    else{
        if(cur_net_init_state == FAILED){
            //MessageBox(hwnd, "Net init failed", "Notice", MB_ICONQUESTION);
            MESS_PRINT("Net init failed");
            enable_by_id(IDC_RADBTN1, 1);
            enable_by_id(IDC_RADBTN2, 1);
            enable_by_id(IDC_RADBTN3, 1);
            enable_by_id(ID_DATA, 1);
            HWND hdtmp = GetDlgItem(hwnd, IDB_FIVE);
            EnableWindow(hdtmp, true);
            wait_net_connect = CONNECT_NOT_STARTED;
            running_mode = TBD;
        }
        else if(cur_net_init_state == READY){
            start_init = 1;
            check_load_file == 0;
        }
    }

    if(start_init){
        if(running_mode == SERVER_MODE && (remote_side->is_ready())){
            df("line %d: start_init+", __LINE__);
            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
            tp_tmp->p_type = SET_REMOTE_PLAYER;
            tp_tmp->pd.remote_side = OTHER_SIDE(local_player);
            remote_side->send_package(tp_tmp);
            tp_tmp = remote_side->get_trans_pack_buf();
            tp_tmp->p_type = SET_TIMEOUT;
            tp_tmp->pd.timeout = g_cconfig.timeout;
            remote_side->send_package(tp_tmp);
            df("line %d: start_init-", __LINE__);
        }
        wait_net_connect = CONNECT_DONE;
        enable_by_id(IDC_RADBTN1, 0);
        enable_by_id(IDC_RADBTN2, 0);
        enable_by_id(IDC_RADBTN3, 0);
        enable_by_id(ID_DATA, 0);
        EnableWindow(Button1Hd, true);
        EnableWindow(Button2Hd, true);
        EnableWindow(Button3Hd, true);
        EnableWindow(Button4Hd, true);
        EnableWindow(ButtonLoadHd, true);
        HWND hdtmp = GetDlgItem(hwnd, IDB_FIVE);
        SetWindowText(hdtmp, "Reset");
        EnableWindow(hdtmp, true);
        InvalidateRect(hwnd,NULL,TRUE);
        InterlockedIncrement(&func_guard);//never enter here again
    }
    InterlockedDecrement(&func_guard);
}

//entry of program
int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow
        )
{
    if(!CheckFolderExist("logs")){
        CreateDirectory("logs", NULL);
    }
    g_cconfig.get_config();
    log_to_file=g_cconfig.log;
    if(g_cconfig.language == 0){//eng
        gp_text_rc = &eng_tr;
    }
    else{
        gp_text_rc = &GBK_tr;
    }
    hRoot = LoadMenu(hInstance, MAKEINTRESOURCE(IDM_MENU));
    CreateMyMenu();
    // class name
    const char* cls_Name = "My Class";
    // design windows class
    WNDCLASS wc = { };
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = cls_Name;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon  = LoadIcon (hInstance,(LPCTSTR)(MAIN_CHESS_ICON));
    //wc.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
    // register windows class
    RegisterClass(&wc);

    hg_app = hInstance;
    // create windows
    HWND hwnd = CreateWindow(
            cls_Name,           //class name, same with registered
            gp_text_rc->text_app_title,  //title of windows
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX& ~WS_THICKFRAME,
            38,                 //x cordinate in father windows
            20,                 //y cordinate in father windows
            FIX_WIN_WIDTH,                // width of windos
            FIX_WIN_HEIGHT,                // height of windows
            NULL,               //no father windows, is NULL
            hRoot,               //menu
            hInstance,          //handle of current windows
            NULL);              //no attached data, NULL
    if(hwnd == NULL) // check if successfully created windows
        return 0;
    mainHd = hwnd;
    ShowWindow(hwnd, SW_SHOW);

    UpdateWindow(hwnd);

    //message loop
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

char* get_saved_chess_file(char*input)
{

    OPENFILENAME ofn = { 0 };
    TCHAR strFilename[MAX_PATH] = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("*.chss\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = strFilename;
    ofn.nMaxFile = sizeof(strFilename);
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = TEXT("Please choose file");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if(GetOpenFileName(&ofn))
    {
        //MessageBox(NULL, strFilename, TEXT("choosed file"), 0);
        strcpy(input, strFilename);
    }
    return input;

}

bool read_chess_file(char*fn)
{
    char read_line[64];
    FILE* f = fopen(fn, "r");
    if(!f)
    {
        df("open chess_file fail\n");
        return false;
    }
    else {
        g_chess_game.review_reset();
        do{
            fgets(read_line, 100, f);
            //df("read:%s", read_line);
        }
        while(!feof(f) && g_chess_game.read_step(read_line));
        fclose(f);
        g_chess_game.read_step("eeee");
        df("read %s done", fn);
        return true;
    }
}

void save_chess_game()
{
    char tmp[64];
    char*fn = g_chess_game.save_hint();
    if(strlen(fn)==0){
        df("no game ran, no save");
        return;
    }
    sprintf(tmp, "chss_save/%s", fn);
    df("fn %s", fn);
    if(!CheckFolderExist("chss_save")){
        CreateDirectory("chss_save", NULL);
    }
    FILE* f = fopen(tmp, "w");
    char*write_line;
    if(!f)
    {
        df("open chess_save_file fail\n");
    }
    else {
        fprintf(f, ";%s\n", fn);
        while(NULL!=(write_line=g_chess_game.get_save_line())){
            fprintf(f, "%s\n", write_line);
        }
        fclose(f);
        df("save_chess_game %s done", fn);
    }
}

DWORD WINAPI thread_save_chess_game(LPVOID lpThreadParameter)
{
    save_chess_game();
    return 0;
}

void create_thread_save_chess_game()
{
    DWORD dwThreadID;
    HANDLE hHandle = CreateThread(0, 0, thread_save_chess_game, (LPVOID)NULL, 0, &dwThreadID);
}

void timer_init(HWND hwnd)
{
    //timer init
    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        //MessageBox(hwnd, "CreateTimerQueue failed", "Error", MB_ICONERROR);
        MESS_PRINT("CreateTimerQueue failed");
        return;
    }
    // Set a timer to call the timer routine in 10 seconds.
    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue,
                (WAITORTIMERCALLBACK)TimerRoutine, hwnd , 500, 100, 0))
    {
        //MessageBox(hwnd, "CreateTimerQueueTimer failed", "Error", MB_ICONERROR);
        MESS_PRINT("CreateTimerQueueTimer failed");
        return;
    }

}

bool CheckFolderExist(const char*strPath)
{
    WIN32_FIND_DATA FindFileData;
    bool bValue = false;
    HANDLE hFind = FindFirstFile(strPath,  &FindFileData);
    if ((hFind != INVALID_HANDLE_VALUE) && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        bValue = TRUE;
    }
    FindClose(hFind);
    return bValue;
}

void show_message(char*ms)
{
    MESS_PRINT("%s", ms);
}

void enable_by_id(int id, int enable)
{
    HWND hdtmp = GetDlgItem(mainHd, id);
    if(hdtmp != NULL){
        EnableWindow(hdtmp, enable);
    }
}

LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        )
{
    HBITMAP hBitmap;
    HBITMAP hBitmapCP[DCP_NUM_MAX];
    static HDC s_hdcMem;
    static HDC s_hdcMemCP[DCP_NUM_MAX];
    int chess_pieces_bitmap_id;

    switch(uMsg)
    {
        case WM_CREATE:
            {
                g_chess_game.set_show_message(show_message);
                int dpx = RIGHT_CONTROL_X, dpy = RIGHT_CONTROL_Y;
                //create three button
                CreateWindow("Button", gp_text_rc->text_start_up, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx, dpy, 60, 20, hwnd, (HMENU)IDB_FIVE, hg_app, NULL);
                ButtonAutoCoverHd = CreateWindow("Button", "AutoReset:ON", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                        dpx+68, dpy, 120, 20, hwnd, (HMENU)IDB_AUTOCOVER, hg_app, NULL);
                SendMessage(ButtonAutoCoverHd, BM_SETCHECK, BST_CHECKED, 0);
                dpx+=5;
                dpy+=23;
                rb3Hd = CreateWindow("Button", gp_text_rc->text_local,
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                        dpx, dpy, 80, 20,
                        hwnd,(HMENU)IDC_RADBTN3,hg_app,NULL);
                HWND hdtmp = GetDlgItem(hwnd, IDC_RADBTN3);
                SendMessage(hdtmp, BM_SETCHECK, 1, 0);
                dpy+=23;
                rb1Hd = CreateWindow("Button",gp_text_rc->text_server,
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                        dpx, dpy, 80, 20,
                        hwnd,
                        (HMENU)IDC_RADBTN1,
                        hg_app,NULL);
                dpy+=23;
                seripHd = CreateWindow("Static","server ip",
                        SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        dpx+15,dpy,150,20,
                        hwnd, NULL,
                        hg_app,
                        NULL);
                dpy+=23;
                rb2Hd = CreateWindow("Button",gp_text_rc->text_client,
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                        dpx, dpy, 80, 20,
                        hwnd,(HMENU)IDC_RADBTN2,hg_app,NULL);
                dpy+=23;
                //CreateWindow(TEXT("edit"),TEXT("myedit"),WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,
                editHd = CreateWindow(TEXT("edit"),TEXT(g_cconfig.ip),WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                        dpx+15, dpy, 160, 20, hwnd,(HMENU)ID_DATA, hg_app,NULL);
                dpy+=50;
                MessageHd = CreateWindow("Static","message\r\nmore message will be shown here. This is only test message",
                        WS_CHILD | WS_VISIBLE,
                        dpx-5,dpy,185,400,
                        hwnd, (HMENU)IDS_MESSAGE,
                        hg_app,
                        NULL);
                //////
                dpy+=450;
                Button1Hd = CreateWindow("Button", gp_text_rc->text_switch, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx-5, dpy, 60, 20, hwnd, (HMENU)IDB_ONE, hg_app, NULL);
                Button2Hd = CreateWindow("Button", gp_text_rc->text_start, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx+59, dpy, 60, 20, hwnd, (HMENU)IDB_TWO, hg_app, NULL);
                Button3Hd = CreateWindow("Button", gp_text_rc->text_drawn, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx+123, dpy, 60, 20, hwnd, (HMENU)IDB_THREE, hg_app, NULL);
                dpy+=30;
                Button4Hd = CreateWindow("Button", gp_text_rc->text_give, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx-5, dpy, 60, 20, hwnd, (HMENU)IDB_FOUR, hg_app, NULL);
                ButtonLoadHd = CreateWindow("Button", gp_text_rc->text_load, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        dpx+59, dpy, 60, 20, hwnd, (HMENU)IDB_LOAD, hg_app, NULL);
                ButtonPauseHd = CreateWindow("Button", gp_text_rc->text_pause, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        7, 220, 40, 20, hwnd, (HMENU)IDB_PAUSE, hg_app, NULL);
                EnableWindow(Button1Hd, false);
                EnableWindow(Button2Hd, false);
                EnableWindow(Button3Hd, false);
                EnableWindow(Button4Hd, false);
                EnableWindow(ButtonLoadHd, false);
                enable_by_id(IDM_OPT1, 0);
                enable_by_id(IDM_OPT2, 0);
                tmrLocalHd = CreateWindow("Static","local",
                        SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        3,450,250,60,
                        hwnd, NULL,
                        hg_app,
                        NULL);
                tmrRemoHd = CreateWindow("Static","remote",
                        SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        3,250,250,60,
                        hwnd, NULL,
                        hg_app,
                        NULL);
            }

            memset(strbuf, 0, 128);
            // set dlg size changable
            //SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_SIZEBOX);
            // load image
#if 1
            //hBitmap = (HBITMAP)LoadImage(NULL, "qipan.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            hBitmap = LoadBitmap((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),MAKEINTRESOURCE(QIPAN_BMP));
            if (hBitmap == NULL)
            {
                //MessageBox(hwnd, "LoadImage failed", "Error", MB_ICONERROR);
                MESS_PRINT("LoadImage qipan failed");
            }
            else
            {
                // put image to HDC - s_hdcMem
                HDC hdc;
                hdc = GetDC(hwnd);
                s_hdcMem = CreateCompatibleDC(hdc);
                SelectObject(s_hdcMem, hBitmap);
                ReleaseDC(hwnd, hdc);
            }
#endif
            chess_pieces_bitmap_id = CHESS_PIECE_BP_01;
            for(int i=0;i<DCP_NUM_MAX;i++){
                hBitmapCP[i] = LoadBitmap((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),MAKEINTRESOURCE(chess_pieces_bitmap_id));
                if (hBitmapCP[i] == NULL)
                {
                    //MessageBox(hwnd, "LoadImage failed", "Error", MB_ICONERROR);
                    MESS_PRINT("LoadImage %d failed", i);
                }
                else
                {
                    // put image to HDC - s_hdcMem
                    HDC hdc;
                    hdc = GetDC(hwnd);
                    s_hdcMemCP[i] = CreateCompatibleDC(hdc);
                    SelectObject(s_hdcMemCP[i], hBitmapCP[i]);
                    ReleaseDC(hwnd, hdc);
                }
                chess_pieces_bitmap_id++;
            }
            timer_init(hwnd);
            MESS_PRINT("version:v%s", VERSION);
            break;
        case WM_CONTEXTMENU:
            {
                //load menu rc
                //HMENU hroot = LoadMenu((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDM_MENU));
                df("right click:running state %d", g_chess_game.get_running_state());
                if(INIT_STATE == g_chess_game.get_running_state()&& running_mode != TBD){
                    HMENU hroot = hRoot;
                    if(hroot)
                    {
                        // get first pop menu
                        HMENU hpop = GetSubMenu(hroot,3);
                        // get cordinate of mouse
                        int px = GET_X_LPARAM(lParam);
                        int py = GET_Y_LPARAM(lParam);
                        // display pop menu
                        TrackPopupMenu(hpop,
                                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
                                px,
                                py,
                                0,
                                (HWND)wParam,
                                NULL);
                        // destroy after use
                        //DestroyMenu(hroot);
                    }
                }
            }
            break;
        case WM_COMMAND:
            {
                // get Id of menu and check which menu user selected
                switch(LOWORD(wParam))
                {
                    case IDM_OPT1:
                        g_cconfig.timeout -= 50;
                        g_chess_game.set_timeout(local_player, g_cconfig.timeout);
                        if(remote_side->is_ready()){
                            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                            tp_tmp->p_type = SET_TIMEOUT;
                            tp_tmp->pd.timeout = g_cconfig.timeout;
                            remote_side->send_package(tp_tmp);
                        }
                        //MessageBox(hwnd,"timeout=timeout-50","Notice",MB_OK);
                        df("timeout=timeout-50");
                        InvalidateRect(hwnd,NULL,TRUE);
                        break;
                    case IDM_OPT2:
                        g_cconfig.timeout += 50;
                        g_chess_game.set_timeout(local_player, g_cconfig.timeout);
                        if(remote_side->is_ready()){
                            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                            tp_tmp->p_type = SET_TIMEOUT;
                            tp_tmp->pd.timeout = g_cconfig.timeout;
                            remote_side->send_package(tp_tmp);
                        }
                        //MessageBox(hwnd,"timeout=timeout+50","notice",MB_OK);
                        df("timeout=timeout+50");
                        InvalidateRect(hwnd,NULL,TRUE);
                        break;
                    case IDB_ONE://switch
                        df("button 1 clicked");
                        if(END_STATE == g_chess_game.get_running_state() ||
                                REVIEW_STATE == g_chess_game.get_running_state()){
                            g_chess_game.review_reset();
                            MESS_PRINT("%s", gp_text_rc->text_review);
                            InvalidateRect(hwnd,NULL,TRUE);
                            SetWindowText(Button4Hd, gp_text_rc->text_prev);
                            SetWindowText(Button3Hd, gp_text_rc->text_next);
                            break;
                        }
                        if(running_mode == SERVER_MODE || running_mode == LOCAL_MODE){
                            local_player = (local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED;
                            g_cdtts.set_revert(local_player == SIDE_RED);
                            chess_playing_handle[local_player] = SCREEN_CLICK_TYPE;
                            if(running_mode == LOCAL_MODE){
                                chess_playing_handle[OTHER_SIDE(local_player)] = SCREEN_CLICK_TYPE;
                            }
                            else{
                                chess_playing_handle[OTHER_SIDE(local_player)] = NET_TYPE;
                            }
                            g_chess_game.set_timeout(local_player, g_cconfig.timeout);
                            if(remote_side->is_ready()){
                                trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = SET_REMOTE_PLAYER;
                                tp_tmp->pd.remote_side = OTHER_SIDE(local_player);
                                df("send_package:set remote player");
                                remote_side->send_package(tp_tmp);
                                tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = SET_TIMEOUT;
                                tp_tmp->pd.timeout = g_cconfig.timeout;
                                remote_side->send_package(tp_tmp);
                            }
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        else if(remote_side->is_ready()){
                            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                            tp_tmp->p_type = REQUEST_SWITCH;
                            remote_side->send_package(tp_tmp);
                            df("send_package:request switch");
                        }
                        break;
                    case IDB_TWO://start
                        df("button 2 clicked");
                        //MessageBox(hwnd, "your clicked two", "Notice", MB_OK | MB_ICONINFORMATION);
                        //SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"second clicked");
                        {
                            if((running_mode == SERVER_MODE || running_mode == CLIENT_MODE)
                                    && !remote_side->is_ready()){
                                //MessageBox(hwnd, "Connection is not ready", "Notice", MB_OK | MB_ICONINFORMATION);
                                MESS_PRINT("Connection is not ready");
                                break;
                            }
                            if(running_mode == CLIENT_MODE){
                                MESS_PRINT("Will wait Server start");
                                //MessageBox(hwnd, "Will wait Server start", "Notice", MB_OK | MB_ICONINFORMATION);
                            }
                            else{
                                RUN_STATE rstmp = g_chess_game.get_running_state();
                                if(END_STATE == rstmp || REVIEW_STATE == rstmp){
                                    if(!g_chess_game.is_saved()){
                                        save_chess_game();
                                    }
                                    g_chess_game.reset();
                                    EnableWindow(Button1Hd, true);
                                    SetWindowText(Button1Hd, gp_text_rc->text_switch);
                                    SetWindowText(Button3Hd, gp_text_rc->text_drawn);
                                    SetWindowText(Button4Hd, gp_text_rc->text_give);
                                }
                                else if(INIT_STATE == rstmp){
                                    SYSTEMTIME time;
                                    GetLocalTime(&time);

                                    sprintf(strbuf, "chess_%d%02d%02d_%02d%02d%02d_%03d",
                                            time.wYear, time.wMonth, time.wDay, time.wHour,
                                            time.wMinute, time.wSecond, time.wMilliseconds);
                                    g_chess_game.set_starttime(strbuf);
                                    g_chess_game.start();
                                    EnableWindow(Button2Hd, false);
                                    EnableWindow(Button1Hd, false);
                                    MESS_PRINT("New game start!");
                                }
                                InvalidateRect(hwnd,NULL,TRUE);
                                if(remote_side->is_ready()){
                                    df("send_package:send start");
                                    remote_side->send_cmd(NETCMD_START_BUTTON);
                                }
                            }
                        }
                        break;
                    case IDB_THREE://drawn
                        {
                            df("button 3 clicked");
                            RUN_STATE rstmp = g_chess_game.get_running_state();
                            switch(rstmp){
                                case REVIEW_STATE:
                                    g_chess_game.review_next();
                                    InvalidateRect(hwnd,NULL,TRUE);
                                    break;
                                case PLAYING_STATE:
                                    {
                                        switch(running_mode){
                                            case SERVER_MODE:
                                            case CLIENT_MODE:
                                                if(remote_side->is_ready()){
                                                    trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                                    tp_tmp->p_type = REQUEST_DRAWN;
                                                    remote_side->send_package(tp_tmp);
                                                    df("send_package:request drawn");
                                                }
                                                if(g_chess_game.request_drawn_side(local_player)){
                                                    InvalidateRect(hwnd,NULL,TRUE);
                                                }
                                                break;
                                            case LOCAL_MODE:
                                            default:
                                                if(g_chess_game.request_drawn_side(g_chess_game.get_current_playing_side())){
                                                    InvalidateRect(hwnd,NULL,TRUE);
                                                }
                                                break;
                                        }
                                    }
                                default:
                                    break;
                            }
                        }
                        break;
                    case IDB_AUTOCOVER:
                        if(autorecover_enable){
                            autorecover_enable = false;
                            SetWindowText(ButtonAutoCoverHd, "AutoReset:OFF");
                            SendMessage(ButtonAutoCoverHd, BM_SETCHECK, BST_UNCHECKED, 0);
                        }
                        else{
                            autorecover_enable = true;
                            SetWindowText(ButtonAutoCoverHd, "AutoReset:ON");
                            SendMessage(ButtonAutoCoverHd, BM_SETCHECK, BST_CHECKED, 0);
                        }
                        break;
                    case IDB_LOAD://load
                        df("button load clicked");
                        memset(strbuf, 0, 128);
                        if(PLAYING_STATE != g_chess_game.get_running_state()){
                            if(!g_chess_game.is_saved()){
                                save_chess_game();
                            }
                            EnableWindow(Button1Hd, false);
                            EnableWindow(Button2Hd, false);
                            EnableWindow(Button3Hd, false);
                            EnableWindow(Button4Hd, false);
                            EnableWindow(ButtonLoadHd, false);
                            get_saved_chess_file(strbuf);
                            EnableWindow(Button1Hd, true);
                            EnableWindow(Button2Hd, true);
                            EnableWindow(Button3Hd, true);
                            EnableWindow(Button4Hd, true);
                            EnableWindow(ButtonLoadHd, true);
                            if(strlen(strbuf) !=0){
                                df("will open %s", strbuf);
                                if(read_chess_file(strbuf)){
                                    SetWindowText(Button4Hd, gp_text_rc->text_prev);
                                    SetWindowText(Button3Hd, gp_text_rc->text_next);
                                    SetWindowText(Button1Hd, gp_text_rc->text_review);
                                    g_chess_game.review_reset();
                                    InvalidateRect(hwnd,NULL,TRUE);
                                }
                            }
                        }
                        break;
                    case IDB_PAUSE://pause
                        {
                            if(running_mode == LOCAL_MODE ||
                                    (running_mode != LOCAL_MODE &&
                                     g_chess_game.get_current_playing_side() != local_player)){
                                g_chess_game.set_timer_pause();
                                if(remote_side->is_ready()){
                                    remote_side->send_cmd(TIMER_PAUSE);
                                }
                            }
                        }
                        break;
                    case IDB_FOUR://give
                        df("button 4 clicked");
                        if(PLAYING_STATE == g_chess_game.get_running_state()){
                            if(remote_side->is_ready()){
                                trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = REQUEST_GIVE;
                                remote_side->send_package(tp_tmp);
                            }
                            g_chess_game.set_win(OTHER_SIDE(g_chess_game.get_current_playing_side()), OPPONENT_GIVE);
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        else if(REVIEW_STATE == g_chess_game.get_running_state()){
                            g_chess_game.review_prev();
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        break;
                    case IDB_FIVE://choose mode
                        {
                            df("button 5 clicked");
                            if(running_mode != TBD){
                                if(running_mode == CLIENT_MODE){
                                    MESS_PRINT("Reset client");
                                }
                                else if(running_mode == SERVER_MODE){
                                    MESS_PRINT("Reset server");
                                }
                                df("reset connection");
                                remote_side->reset_connect();
                                break;
                            }
                            g_chess_game.reset();
                            g_chess_game.set_timeout(local_player, g_cconfig.timeout);

                            if(Button_GetCheck(rb3Hd)){
                                running_mode = LOCAL_MODE;
                                chess_playing_handle[SIDE_RED] = SCREEN_CLICK_TYPE;
                                chess_playing_handle[SIDE_BLACK] = SCREEN_CLICK_TYPE;
                                MESS_PRINT("Running as local");
                            }
                            if(Button_GetCheck(rb2Hd)){
                                running_mode = CLIENT_MODE;
                                MESS_PRINT("Running as client");
                                chess_playing_handle[local_player] = SCREEN_CLICK_TYPE;
                                chess_playing_handle[(local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED] = NET_TYPE;
                                memset(strbuf, 0, 128);
                                GetWindowText(editHd, strbuf, 128);
                                strcpy(g_cconfig.ip, strbuf);
                                df("ip input:%s", strbuf);
                                remote_side->init(strbuf,(u_short)PORT_NUM);
                            }
                            if(Button_GetCheck(rb1Hd)){
                                SetWindowText(seripHd, net_trans::get_local_ip());
                                running_mode = SERVER_MODE;
                                chess_playing_handle[local_player] = SCREEN_CLICK_TYPE;
                                chess_playing_handle[(local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED] = NET_TYPE;
                                remote_side->init(NULL,(u_short)PORT_NUM);
                                MESS_PRINT("Running as server");
                            }
                            wait_net_connect = CONNECT_WAITING;
                            enable_by_id(IDC_RADBTN1, 0);
                            enable_by_id(IDC_RADBTN2, 0);
                            enable_by_id(IDC_RADBTN3, 0);
                            enable_by_id(ID_DATA, 0);
                            HWND hdtmp = GetDlgItem(hwnd, IDB_FIVE);
                            EnableWindow(hdtmp, false);
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_LBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                //df("left mouse %d %d", x, y);
                //MESS_PRINT("left mouse %d %d", x, y);
                if(PLAYING_STATE == g_chess_game.get_running_state() &&
                        (SCREEN_CLICK_TYPE == chess_playing_handle[g_chess_game.get_current_playing_side()]) &&
                        g_cdtts.is_in_chessboard(x,y))
                {
                    bool ret;
                    int chess_x = g_cdtts.screen_to_chess_x(x);
                    int chess_y = g_cdtts.screen_to_chess_y(y);
                    if(g_chess_game.get_choosen_cp() == NULL){
                        ret = g_chess_game.choose_point(chess_x, chess_y);
                    }
                    else{
                        char*tmpcharp;
                        ret = g_chess_game.moveto_point(chess_x, chess_y, &tmpcharp);
                        if(g_chess_game.get_choosen_cp() == NULL){
                            if(remote_side->is_ready()){
                                move_step*mstmp=g_chess_game.get_lastmove();
                                trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = CHESS_STEP;
                                tp_tmp->pd.ch_move_step.x1 = mstmp->x1;
                                tp_tmp->pd.ch_move_step.y1 = mstmp->y1;
                                tp_tmp->pd.ch_move_step.x2 = mstmp->x2;
                                tp_tmp->pd.ch_move_step.y2 = mstmp->y2;
                                tp_tmp->pd.timeout = g_chess_game.get_timeout(local_player);
                                remote_side->send_package(tp_tmp);
                            }
                            if(tmpcharp){
                                //MESS_PRINT("%s", tmpcharp);
                            }
                            /*
                            df("move %d-%d-%d-%d",
                                    tp_tmp->pd.ch_move_step.x1,
                                    tp_tmp->pd.ch_move_step.y1,
                                    tp_tmp->pd.ch_move_step.x2,
                                    tp_tmp->pd.ch_move_step.y2
                                    );
                                    */
                        }
                    }
                    if(ret){
                        InvalidateRect(hwnd,NULL,TRUE);
                    }
                }
            }
            break;
        case WM_PAINT:
            {
                HDC hdc;
                RECT rt;
                PAINTSTRUCT ps;
                hdc = BeginPaint(hwnd, &ps);

                //image of background
                GetClientRect(hwnd, &rt);
                BitBlt(hdc, QIPAN_LOAD_POINT_X, QIPAN_LOAD_POINT_Y, rt.right, rt.bottom, s_hdcMem, 0, 0, SRCCOPY);
                if(running_mode != TBD && wait_net_connect != CONNECT_WAITING){
#if 1
                    HBRUSH qipan_hb = CreateSolidBrush(RGB(255,255,255));
                    HBRUSH qipan_orgBrs = (HBRUSH)SelectObject(ps.hdc, qipan_hb);
                    int tmpx, tmpy, tmpd;
                    for(int i = 0;i < CP_NUM_MAX;i++){
                        chess_piece * cptmp = g_chess_game.get_cp((CHESS_PIECES_INDEX)i);
                        if(cptmp && cptmp->is_alive()){
                            tmpx = cptmp->get_p_x();
                            tmpy = cptmp->get_p_y();
                            tmpd = CHESS_PIECE_SIZE/2;
                            //MESS_PRINT("bitblt %d %d",  cptmp->get_p_x(), cptmp->get_p_y());
                            Ellipse(ps.hdc,g_cdtts.chess_to_screen_x(tmpx)-tmpd,g_cdtts.chess_to_screen_y(tmpy)-tmpd,g_cdtts.chess_to_screen_x(tmpx)+tmpd,g_cdtts.chess_to_screen_y(tmpy)+tmpd);
                            BitBlt(hdc, g_cdtts.chess_to_screen_x(cptmp->get_p_x())-CHESS_WORD_SIZE/2, g_cdtts.chess_to_screen_y(cptmp->get_p_y())-CHESS_WORD_SIZE/2, rt.right, rt.bottom, s_hdcMemCP[cp_display_map[i]], 0, 0, SRCCOPY);
                        }
                    }
                    SelectObject(ps.hdc, qipan_orgBrs);
                    DeleteObject(qipan_hb);
#else
                    for(int i = 0; i<MAX_CHS_BOARD_Y;i++){
                        for(int j = 0; j<MAX_CHS_BOARD_X;j++){
                            //cpes_board[i][j] = NULL;
                            chess_piece * cptmp = g_chess_game.get_cp(j,i);
                            if(cptmp && cptmp->is_alive()){
                                //MESS_PRINT("bitblt %d %d",  cptmp->get_p_x(), cptmp->get_p_y());
                                BitBlt(hdc, g_cdtts.chess_to_screen_x(cptmp->get_p_x())-CELL_SIZE/2, g_cdtts.chess_to_screen_y(cptmp->get_p_y())-CELL_SIZE/2, rt.right, rt.bottom, s_hdcMemCP[cp_display_map[cptmp->get_cpid()]], 0, 0, SRCCOPY);
                            }
                        }
                    }
#endif
                    {
                        //HBRUSH hb = CreateSolidBrush(RGB(255,255,0));
                        //HBRUSH hb = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HPEN hPen = CreatePen(PS_SOLID,4,RGB(0,95,255));;
                        HPEN orgPen = (HPEN)SelectObject(ps.hdc, hPen);
                        if(g_chess_game.get_choosen_cp() != NULL){
                            int x, y;
                            x = g_chess_game.get_choosen_cp()->get_p_x();
                            y = g_chess_game.get_choosen_cp()->get_p_y();
                            //Ellipse(ps.hdc,g_cdtts.chess_to_screen_x(x-0.6),g_cdtts.chess_to_screen_y(y-0.6),g_cdtts.chess_to_screen_x(x+0.6),g_cdtts.chess_to_screen_y(y+0.6));
#define RTIA 0.5
                            MoveToEx(ps.hdc, g_cdtts.chess_to_screen_x(x-RTIA),g_cdtts.chess_to_screen_y(y-RTIA), NULL);
                            LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x-RTIA),g_cdtts.chess_to_screen_y(y+RTIA));
                            LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x+RTIA),g_cdtts.chess_to_screen_y(y+RTIA));
                            LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x+RTIA),g_cdtts.chess_to_screen_y(y-RTIA));
                            LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x-RTIA),g_cdtts.chess_to_screen_y(y-RTIA));
                        }
                        if(g_chess_game.get_lastmove() != NULL){
                            move_step*mstmp=g_chess_game.get_lastmove();
                            //df("paint last move %d %d %d %d", mstmp->x1, mstmp->y1, mstmp->x2, mstmp->y2);
#define ARROWLEN 0.2f
#define V_ARROWLEN 0.1f
                            float dx = mstmp->x2 - mstmp->x1;
                            float dy = mstmp->y2 - mstmp->y1;
                            float len = sqrt(dx*dx + dy*dy);
                            if(len != 0){
                                float sin_angle = dy/len;
                                float cos_angle = dx/len;
                                float x3 = mstmp->x2-ARROWLEN*cos_angle+V_ARROWLEN*sin_angle;
                                float y3 = mstmp->y2-ARROWLEN*sin_angle-V_ARROWLEN*cos_angle;
                                float x4 = mstmp->x2-ARROWLEN*cos_angle-V_ARROWLEN*sin_angle;
                                float y4 = mstmp->y2-ARROWLEN*sin_angle+V_ARROWLEN*cos_angle;
#define RTIA2 0.1
                                Ellipse(ps.hdc,g_cdtts.chess_to_screen_x(mstmp->x1-RTIA2),g_cdtts.chess_to_screen_y(mstmp->y1-RTIA2),g_cdtts.chess_to_screen_x(mstmp->x1+RTIA2),g_cdtts.chess_to_screen_y(mstmp->y1+RTIA2));
                                MoveToEx(ps.hdc, g_cdtts.chess_to_screen_x(mstmp->x1),g_cdtts.chess_to_screen_y(mstmp->y1), NULL);
                                LineTo(ps.hdc, g_cdtts.chess_to_screen_x(mstmp->x2),g_cdtts.chess_to_screen_y(mstmp->y2));
                                LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x3),g_cdtts.chess_to_screen_y(y3));
                                LineTo(ps.hdc, g_cdtts.chess_to_screen_x(x4),g_cdtts.chess_to_screen_y(y4));
                                LineTo(ps.hdc, g_cdtts.chess_to_screen_x(mstmp->x2),g_cdtts.chess_to_screen_y(mstmp->y2));
                            }
                        }
                        SelectObject(ps.hdc, orgPen);
                        DeleteObject(hPen);
                    }
                    if(g_chess_game.get_running_state() == END_STATE){
                        float x, y;
                        switch(g_chess_game.get_game_result()){
                            case RESULT_RED_WIN:
                                x = 4;
                                y = 1;
                                break;
                            case RESULT_BLACK_WIN:
                                x = 4;
                                y = 8;
                                break;
                            case RESULT_DRAWN:
                                x = 4;
                                y = 4.5;
                                break;
                        }
                        HBRUSH hb = CreateSolidBrush(RGB(255,255,0));
                        HBRUSH orgBrs = (HBRUSH)SelectObject(ps.hdc, hb);
                        Ellipse(ps.hdc,g_cdtts.chess_to_screen_x(x-1),g_cdtts.chess_to_screen_y(y-1),g_cdtts.chess_to_screen_x(x+1),g_cdtts.chess_to_screen_y(y+1));
                        SelectObject(ps.hdc, orgBrs);
                        DeleteObject(hb);
                        SetWindowText(Button1Hd, gp_text_rc->text_review);
                        EnableWindow(Button1Hd, true);
                    }
                    if(g_chess_game.get_running_state() == PLAYING_STATE){
                        float x, y;
                        if(g_chess_game.get_current_playing_side() == OTHER_SIDE(local_player)){
                            x = 29;
                            y = 258;
                        }
                        else{
                            x = 29;
                            y = 460;
                        }
                        HBRUSH hb = CreateSolidBrush(RGB(0,0,255));
                        HBRUSH orgBrs = (HBRUSH)SelectObject(ps.hdc, hb);
#define TIMER_DISPLAY_SIZE 30
                        Ellipse(ps.hdc,x-TIMER_DISPLAY_SIZE,y-TIMER_DISPLAY_SIZE,x+TIMER_DISPLAY_SIZE,y+TIMER_DISPLAY_SIZE);
                        SelectObject(ps.hdc, orgBrs);
                        DeleteObject(hb);
                    }
                }
                sprintf(strbuf, "Built @ %s %s, V%s", __DATE__, __TIME__, VERSION);
                TextOut(ps.hdc,0, BUILD_MESSAGE_Y, strbuf,strlen(strbuf));
#if 0
                BitBlt(hdc, movingx, movingy, rt.right, rt.bottom, s_hdcMemBin, 0, 0, SRCCOPY);
                BitBlt(hdc, 500-movingx, movingy, rt.right, rt.bottom, s_hdcMemBin, 0, 0, SRCCOPY);
                memset(strbuf, 0, 128);
                sprintf(strbuf, "%02d", timer_count);
                SetTextColor(ps.hdc, RGB(10, 0, 255));
                DrawText(ps.hdc, "hello friends",strlen("hello friends"), &(ps.rcPaint), DT_CENTER);
                int arr1[2]= {45,0};
                int arr2[3] = { 35, 40, 0 };
                int arr3[3] = { 9, 10, 0 };
                POLYTEXT polys[] =  { {2,25,3,"AL",ETO_CLIPPED,ps.rcPaint,&arr1[0]},
                    {2,65,3,"hap",ETO_CLIPPED,ps.rcPaint,&arr2[0]},
                    {2,90,3,strbuf,ETO_CLIPPED,ps.rcPaint,&arr3[0]}
                };
                PolyTextOut(ps.hdc, &polys[0],3);
                HBRUSH hb = CreateSolidBrush(RGB(0,255,0));
                HBRUSH orgBrs = (HBRUSH)SelectObject(ps.hdc, hb);
                Ellipse(ps.hdc,135,35,202,170);
                SelectObject(ps.hdc, orgBrs);
                DeleteObject(hb);
#endif
                EndPaint(hwnd, &ps);
            }
            break;
        case WM_DESTROY:
            df("quit");
            if(!g_chess_game.is_saved()){
                save_chess_game();
            }
            g_cconfig.save_config();
            // Delete all timers in the timer queue.
            if (!DeleteTimerQueue(hTimerQueue))
                printf("DeleteTimerQueue failed (%d)\n", GetLastError());
            if(remote_side->is_ready()){
                remote_side->send_cmd(APP_QUIT);
            }
            Sleep(300);
            {
                PostQuitMessage(0);
                return 0;
            }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateMyMenu()
{
    static char menu_char[]= "timeout+50";
    //hRoot = CreateMenu();
    if(!hRoot)
        return;
    HMENU pop1 = CreatePopupMenu();
    AppendMenu(hRoot,
            MF_POPUP,
            (UINT_PTR)pop1,
            "Timeout");
    //one way is using AppendMenu
    AppendMenu(pop1,
            MF_STRING,
            IDM_OPT1,
            "timeout-50");

    //another way is using InsertMenuItem
    MENUITEMINFO mif;
    mif.cbSize = sizeof(MENUITEMINFO);
    mif.cch = 100;
    mif.dwItemData  = (ULONG_PTR)NULL;
    mif.dwTypeData = menu_char;
    mif.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
    mif.fState = MFS_ENABLED;
    mif.fType = MIIM_STRING;
    mif.wID = IDM_OPT2;

    InsertMenuItem(pop1,IDM_OPT2,FALSE,&mif);
}
#if 0
void message_print(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    sprintf(mpbuf, fmt, args);
    va_end(args);
    SetWindowText(MessageHd, mpbuf);
}
#endif
