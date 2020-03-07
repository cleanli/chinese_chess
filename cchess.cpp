#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>
#include <math.h>

#include "play_control.h"
#include "ch_chess.h"
#include "coordinate_trans.h"
#include "remote_player.h"
#include "config.h"

#define IDR_CONTEXT  200
#define IDM_OPT1     301
#define IDM_OPT2     302


#define IDB_ONE     3301
#define IDB_TWO     3302
#define IDB_THREE   3303
#define IDB_FOUR   3304
#define IDB_FIVE   3305
#define ID_DATA     3306
#define IDB_LOAD   3307

#define IDC_RADBTN1		50001
#define IDC_RADBTN2		50002
#define IDC_RADBTN3		50003
#define IDC_RADBTNBLUE		51001
#define IDC_RADBTNRED		51002
#define IDC_RADBTNGREEN		51003

#define IDS_MESSAGE 51004

#define MESS_SIZE 120
static char mpbuf0[256];
static char mpbuf1[256];
static char* mpbuf[2]={mpbuf0, mpbuf1};
static int mpbuf_index = 0;
char debug_buf[1024];
ch_config g_cconfig;
int log_to_file = 0;
int check_load_file = 1;
#define CONNECT_NOT_STARTED 0
#define CONNECT_WAITING 1
#define CONNECT_DONE 2
int wait_net_connect = CONNECT_NOT_STARTED;

#define MESS_PRINT(fmt,arg...) \
    {   \
        int tmplen; \
        sprintf(mpbuf[mpbuf_index], fmt"\r\n", ##arg); \
        tmplen = strlen(mpbuf[mpbuf_index]); \
        memcpy(mpbuf[mpbuf_index]+tmplen, mpbuf[1-mpbuf_index], MESS_SIZE-tmplen); \
        mpbuf[mpbuf_index][MESS_SIZE-1]=0; \
        SetWindowText(MessageHd, mpbuf[mpbuf_index]);\
        mpbuf_index = 1 - mpbuf_index; \
    }
HMENU hRoot;
void CreateMyMenu();//create menu
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
HWND ButtonLoadHd;
HWND MessageHd;
RUN_MODE running_mode = TBD;
RUN_STATE running_state = INIT_STATE;
HANDLE_TYPE chess_playing_handle[SIDE_MAX];

chess_game g_chess_game(300);
c_coordinate_trans g_cdtts(false);
PLAYING_SIDE local_player = SIDE_BLACK;
//remote_player* remote_side = new dummy_remote_player();
remote_player* remote_side = new net_remote_player();

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

int movingx = 100;
int movingy = 100;
VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    static RUN_STATE last_game_state = INIT_STATE;
    HWND hwnd;
    timer_count++;
    g_chess_game.timer_click();
    if (lpParam != NULL)
    {
        trans_package* tptmp = NULL;
        HWND hwnd=(HWND)lpParam;
        char tBuf[1000];
        memset(tBuf, 0, 1000);
        sprintf(tBuf, "%04.1f    ", (float)g_chess_game.get_timeout(SIDE_BLACK)/10);
        if(local_player == SIDE_BLACK){
            SetWindowText(tmrLocalHd, tBuf);
        }
        else{
            SetWindowText(tmrRemoHd, tBuf);
        }
        memset(tBuf, 0, 1000);
        sprintf(tBuf, "%04.1f    ", (float)g_chess_game.get_timeout(SIDE_RED)/10);
        //MESS_PRINT("%d", timer_count);
        //MESS_PRINT("%d", g_chess_game.get_timeout(SIDE_BLACK));
        if(local_player == SIDE_BLACK){
            SetWindowText(tmrRemoHd, tBuf);
        }
        else{
            SetWindowText(tmrLocalHd, tBuf);
        }
        RUN_STATE rs_tmp = g_chess_game.get_running_state();
        if(rs_tmp != last_game_state){
            if(rs_tmp == END_STATE){
                EnableWindow(Button2Hd, true);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
        last_game_state = rs_tmp;
        if(wait_net_connect == CONNECT_WAITING){
            mode_init(hwnd);
        }
        //df("timer ----");
        if(remote_side->is_ready()){
            while((tptmp = remote_side->get_recved_ok())!=NULL){
                df("timer:recv remote message id %d", tptmp->pk_id);
                remote_side->send_ack(tptmp->pk_id);
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
                                    ret = g_chess_game.moveto_point(tptmp->pd.ch_move_step.x2,
                                            tptmp->pd.ch_move_step.y2);
                                }
                                if(ret){
                                    InvalidateRect(hwnd,NULL,TRUE);
                                }
                            }
                            MESS_PRINT("remote timeout %d local %d",
                                    tptmp->pd.timeout, g_chess_game.get_timeout(OTHER_SIDE(local_player)));
                            g_chess_game.set_idleside_timeout(tptmp->pd.timeout);
                        }
                        break;
                    case REQUEST_DRAWN:
                        if(g_chess_game.request_drawn_side(OTHER_SIDE(local_player))){
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        MessageBox(hwnd, "Remote request DRAWN", "Notice", MB_ICONQUESTION);
                        break;
                    case REQUEST_SWITCH:
                        MessageBox(hwnd, "Remote request switch side", "Notice", MB_ICONQUESTION);
                        break;
                    case REQUEST_GIVE:
                        g_chess_game.set_win(local_player);
                        InvalidateRect(hwnd,NULL,TRUE);
                        MessageBox(hwnd, "Remote request GIVE", "Notice", MB_ICONQUESTION);
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
                            RUN_STATE rstmp = g_chess_game.get_running_state();
                            if(END_STATE == rstmp || REVIEW_STATE == rstmp){
                                if(!g_chess_game.is_saved()){
                                    save_chess_game();
                                }
                                g_chess_game.reset();
                                EnableWindow(Button1Hd, true);
                                SetWindowText(Button3Hd, "Drawn");
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
                            int net_timeout = tptmp->pd.timeout;
                            g_chess_game.set_timeout(OTHER_SIDE(local_player), net_timeout);
                        }
                        break;
                    case APP_QUIT:
                        MessageBox(hwnd, "Remote side have left", "Notice", MB_ICONQUESTION);
                        break;
                    case STRING:
                        MESS_PRINT("remote str:%s", tptmp->pd.str_message);
                        break;
                    default:
                        break;
                }
            }
        }
        if(remote_side->get_error_status()){
            MessageBox(hwnd, "Connection is ERROR", "Notice", MB_ICONQUESTION);
        }
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

void mode_init(HWND hwnd)
{
    int start_init = 0;
    int cur_net_init_state = remote_side->get_init_state();

    if(running_mode == LOCAL_MODE){
        start_init = 1;
    }
    else{
        if(cur_net_init_state == FAILED){
            //MessageBox(hwnd, "Net init failed", "Notice", MB_ICONQUESTION);
            df("Net init failed");
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
            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
            tp_tmp->p_type = SET_REMOTE_PLAYER;
            tp_tmp->pd.remote_side = OTHER_SIDE(local_player);
            MESS_PRINT("send_package:set remote player");
            remote_side->send_package(tp_tmp);
            MESS_PRINT("Running as server");
            tp_tmp->p_type = SET_TIMEOUT;
            tp_tmp->pd.timeout = g_cconfig.timeout;
            remote_side->send_package(tp_tmp);
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
        EnableWindow(hdtmp, false);
        InvalidateRect(hwnd,NULL,TRUE);
    }
}

//entry of program
int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow
        )
{
    g_cconfig.get_config();
    log_to_file=g_cconfig.log;
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
    // register windows class
    RegisterClass(&wc);

    hg_app = hInstance;
    // create windows
    HWND hwnd = CreateWindow(
            cls_Name,           //class name, same with registered
            "My window",  //title of windows
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX& ~WS_THICKFRAME,
            38,                 //x cordinate in father windows
            20,                 //y cordinate in father windows
            530,                // width of windos
            705,                // height of windows
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
        MessageBox(NULL, strFilename, TEXT("choosed file"), 0);
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
        do{
            fgets(read_line, 100, f);
            df("read:%s", read_line);
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
    char*fn = g_chess_game.save_hint();
    if(strlen(fn)==0){
        df("no game ran, no save");
        return;
    }
    df("fn %s", fn);
    FILE* f = fopen(fn, "w");
    char*write_line;
    if(!f)
    {
        df("open chess_save_file fail\n");
    }
    else {
        while(NULL!=(write_line=g_chess_game.get_save_line())){
            fprintf(f, "%s\n", write_line);
        }
        fclose(f);
        df("save_chess_game %s done", fn);
    }
}

void timer_init(HWND hwnd)
{
    //timer init
    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        MessageBox(hwnd, "CreateTimerQueue failed", "Error", MB_ICONERROR);
        return;
    }
    // Set a timer to call the timer routine in 10 seconds.
    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue,
                (WAITORTIMERCALLBACK)TimerRoutine, hwnd , 500, 100, 0))
    {
        MessageBox(hwnd, "CreateTimerQueueTimer failed", "Error", MB_ICONERROR);
        return;
    }

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

    switch(uMsg)
    {
        case WM_CREATE:
            {
                //create three button
                CreateWindow("Button", "Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        5, 5, 60, 20, hwnd, (HMENU)IDB_FIVE, hg_app, NULL);
                Button1Hd = CreateWindow("Button", "Switch", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        60, 610, 60, 20, hwnd, (HMENU)IDB_ONE, hg_app, NULL);
                Button2Hd = CreateWindow("Button", "Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        135, 610, 60, 20, hwnd, (HMENU)IDB_TWO, hg_app, NULL);
                Button3Hd = CreateWindow("Button", "Drawn", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        215, 610, 60, 20, hwnd, (HMENU)IDB_THREE, hg_app, NULL);
                Button4Hd = CreateWindow("Button", "Give", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        290, 610, 60, 20, hwnd, (HMENU)IDB_FOUR, hg_app, NULL);
                ButtonLoadHd = CreateWindow("Button", "Load", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        365, 610, 60, 20, hwnd, (HMENU)IDB_LOAD, hg_app, NULL);
                EnableWindow(Button1Hd, false);
                EnableWindow(Button2Hd, false);
                EnableWindow(Button3Hd, false);
                EnableWindow(Button4Hd, false);
                EnableWindow(ButtonLoadHd, false);
                enable_by_id(IDM_OPT1, 0);
                enable_by_id(IDM_OPT2, 0);
                //CreateWindow(TEXT("edit"),TEXT("myedit"),WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,
                editHd = CreateWindow(TEXT("edit"),TEXT(g_cconfig.ip),WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                        100, 73, 190, 20, hwnd,(HMENU)ID_DATA, hg_app,NULL);
                // y cordinate, base
                int yLoc = 0;
                // text
                yLoc += 10;
                seripHd = CreateWindow("Static","server ip",
                        SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        100,50,160,20,
                        hwnd, NULL,
                        hg_app,
                        NULL);
                // group 1
                yLoc += 20;
                rb3Hd = CreateWindow("Button","local",
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                        10, 27, 80, 20,
                        hwnd,(HMENU)IDC_RADBTN3,hg_app,NULL);
                HWND hdtmp = GetDlgItem(hwnd, IDC_RADBTN3);
                SendMessage(hdtmp, BM_SETCHECK, 1, 0);
                rb1Hd = CreateWindow("Button","server",
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                        10, 50, 80, 20,
                        hwnd,
                        (HMENU)IDC_RADBTN1,
                        hg_app,NULL);
                rb2Hd = CreateWindow("Button","client",
                        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                        10, 73, 80, 20,
                        hwnd,(HMENU)IDC_RADBTN2,hg_app,NULL);
                MessageHd = CreateWindow("Static","message\r\nmore message will be shown here. This is only test message",
                        WS_CHILD | WS_VISIBLE,
                        300,0,225,100,
                        hwnd, (HMENU)IDS_MESSAGE,
                        hg_app,
                        NULL);
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
                MESS_PRINT("wm create done");
            }

            memset(strbuf, 0, 128);
            SetWindowText(hwnd, "Chinese Chess");
            // set dlg size changable
            //SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_SIZEBOX);
            // load image
#if 1
            hBitmap = (HBITMAP)LoadImage(NULL, "qipan.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            if (hBitmap == NULL)
            {
                MessageBox(hwnd, "LoadImage failed", "Error", MB_ICONERROR);
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
            for(int i=0;i<DCP_NUM_MAX;i++){
                hBitmapCP[i] = (HBITMAP)LoadImage(NULL, chess_pieces_bmp_path[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                if (hBitmapCP[i] == NULL)
                {
                    MessageBox(hwnd, "LoadImage failed", "Error", MB_ICONERROR);
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
            }
            timer_init(hwnd);
            break;
        case WM_CONTEXTMENU:
            {
                //load menu rc
                //HMENU hroot = LoadMenu((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDR_CONTEXT));
                df("running state %d", g_chess_game.get_running_state());
                if(INIT_STATE == g_chess_game.get_running_state()&& running_mode != TBD){
                    HMENU hroot = hRoot;
                    if(hroot)
                    {
                        // get first pop menu
                        HMENU hpop = GetSubMenu(hroot,0);
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
                        InvalidateRect(hwnd,NULL,TRUE);
                        break;
                    case IDB_ONE://switch
                        if(END_STATE == g_chess_game.get_running_state() ||
                                REVIEW_STATE == g_chess_game.get_running_state()){
                            if(!g_chess_game.is_saved()){
                                save_chess_game();
                            }
                            g_chess_game.review_reset();
                            InvalidateRect(hwnd,NULL,TRUE);
                            SetWindowText(Button3Hd, "Next");
                            SetWindowText(Button4Hd, "Prev");
                            break;
                        }
                        if(running_mode == SERVER_MODE || running_mode == LOCAL_MODE){
                            local_player = (local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED;
                            g_cdtts.set_revert(local_player == SIDE_RED);
                            chess_playing_handle[local_player] = SCREEN_CLICK_TYPE;
                            chess_playing_handle[(local_player == SIDE_RED)?SIDE_BLACK:SIDE_RED] = NET_TYPE;
                            g_chess_game.set_timeout(local_player, g_cconfig.timeout);
                            if(remote_side->is_ready()){
                                trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = SET_REMOTE_PLAYER;
                                tp_tmp->pd.remote_side = OTHER_SIDE(local_player);
                                MESS_PRINT("send_package:set remote player");
                                remote_side->send_package(tp_tmp);
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
                            MESS_PRINT("send_package:request switch");
                        }
                        break;
                    case IDB_TWO://start
                        //MessageBox(hwnd, "your clicked two", "Notice", MB_OK | MB_ICONINFORMATION);
                        //SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"second clicked");
                        {
                            if((running_mode == SERVER_MODE || running_mode == CLIENT_MODE)
                                    && !remote_side->is_ready()){
                                MessageBox(hwnd, "Connection is not ready", "Notice", MB_OK | MB_ICONINFORMATION);
                                break;
                            }
                            if(running_mode == CLIENT_MODE){
                                MessageBox(hwnd, "Will wait Server start", "Notice", MB_OK | MB_ICONINFORMATION);
                            }
                            else{
                                RUN_STATE rstmp = g_chess_game.get_running_state();
                                if(remote_side->is_ready()){
                                    MESS_PRINT("send_package:send start");
                                    remote_side->send_cmd(NETCMD_START_BUTTON);
                                }
                                if(END_STATE == rstmp || REVIEW_STATE == rstmp){
                                    if(!g_chess_game.is_saved()){
                                        save_chess_game();
                                    }
                                    g_chess_game.reset();
                                    EnableWindow(Button1Hd, true);
                                    SetWindowText(Button1Hd, "Switch");
                                    SetWindowText(Button3Hd, "Drawn");
                                    SetWindowText(Button4Hd, "Give");
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
                            }
                        }
                        break;
                    case IDB_THREE://drawn
                        {
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
                                                    MESS_PRINT("send_package:request drawn");
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
                    case IDB_LOAD://load
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
                                    SetWindowText(Button4Hd, "Prev");
                                    SetWindowText(Button3Hd, "Next");
                                    SetWindowText(Button1Hd, "Review");
                                    g_chess_game.review_reset();
                                }
                            }
                        }
                        break;
                    case IDB_FOUR://give
                        if(PLAYING_STATE == g_chess_game.get_running_state()){
                            if(remote_side->is_ready()){
                                trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                                tp_tmp->p_type = REQUEST_GIVE;
                                remote_side->send_package(tp_tmp);
                                MESS_PRINT("send_package:request give");
                            }
                            g_chess_game.set_win((g_chess_game.get_current_playing_side()==SIDE_RED)?SIDE_BLACK:SIDE_RED);
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        else if(REVIEW_STATE == g_chess_game.get_running_state()){
                            g_chess_game.review_prev();
                            InvalidateRect(hwnd,NULL,TRUE);
                        }
                        break;
                    case IDB_FIVE://choose mode
                        {
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
                MESS_PRINT("left mouse %d %d", x, y);
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
                        ret = g_chess_game.moveto_point(chess_x, chess_y);
                        if(g_chess_game.get_choosen_cp() == NULL){
                            move_step*mstmp=g_chess_game.get_lastmove();
                            trans_package* tp_tmp = remote_side->get_trans_pack_buf();
                            tp_tmp->p_type = CHESS_STEP;
                            tp_tmp->pd.ch_move_step.x1 = mstmp->x1;
                            tp_tmp->pd.ch_move_step.y1 = mstmp->y1;
                            tp_tmp->pd.ch_move_step.x2 = mstmp->x2;
                            tp_tmp->pd.ch_move_step.y2 = mstmp->y2;
                            tp_tmp->pd.timeout = g_chess_game.get_timeout(local_player);
                            if(remote_side->is_ready()){
                                remote_side->send_package(tp_tmp);
                            }
                            MESS_PRINT("send_package:step move");
                            df("move %d-%d-%d-%d",
                                    tp_tmp->pd.ch_move_step.x1,
                                    tp_tmp->pd.ch_move_step.y1,
                                    tp_tmp->pd.ch_move_step.x2,
                                    tp_tmp->pd.ch_move_step.y2
                                    );
                            MESS_PRINT("send_package:curtimeout %d",
                                    g_chess_game.get_timeout(local_player));
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
                BitBlt(hdc, 0, 100, rt.right, rt.bottom, s_hdcMem, 0, 0, SRCCOPY);
                if(running_mode != TBD && wait_net_connect != CONNECT_WAITING){
#if 1
                    for(int i = 0;i < CP_NUM_MAX;i++){
                        chess_piece * cptmp = g_chess_game.get_cp((CHESS_PIECES_INDEX)i);
                        if(cptmp && cptmp->is_alive()){
                            //MESS_PRINT("bitblt %d %d",  cptmp->get_p_x(), cptmp->get_p_y());
                            BitBlt(hdc, g_cdtts.chess_to_screen_x(cptmp->get_p_x())-CELL_SIZE/2, g_cdtts.chess_to_screen_y(cptmp->get_p_y())-CELL_SIZE/2, rt.right, rt.bottom, s_hdcMemCP[cp_display_map[i]], 0, 0, SRCCOPY);
                        }
                    }
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
                        else if(g_chess_game.get_lastmove() != NULL){
                            move_step*mstmp=g_chess_game.get_lastmove();
                            df("last move %d %d %d %d", mstmp->x1, mstmp->y1, mstmp->x2, mstmp->y2);
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
                        SetWindowText(Button1Hd, "Review");
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
                TextOut(ps.hdc,0, 640, strbuf,strlen(strbuf));
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
    hRoot = CreateMenu();
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
    mif.dwItemData  = NULL;
    mif.dwTypeData = "timeout+50";
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
