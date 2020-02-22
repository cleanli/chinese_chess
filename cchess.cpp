#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>

#include "play_control.h"
#include "ch_chess.h"

#define IDR_CONTEXT  200
#define IDM_OPT1     301
#define IDM_OPT2     302


#define IDB_ONE     3301
#define IDB_TWO     3302
#define IDB_THREE   3303
#define IDB_FOUR   3304
#define IDB_FIVE   3305
#define ID_DATA     3306

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
HWND tmrLocalHd;
HWND tmrRemoHd;
HWND startButtonHd;
HWND Button1Hd;
HWND Button2Hd;
HWND Button3Hd;
HWND Button4Hd;
HWND MessageHd;
RUN_MODE running_mode = TBD;
RUN_STATE running_state = INIT_STATE;
HANDLE_TYPE chess_playing_handle[SIDE_MAX];

chess_game g_chess_game(19);

void message_print(const char *fmt, ...);
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
    HWND hwnd;
    timer_count++;
    g_chess_game.timer_click();
    if (lpParam != NULL)
    {
        HWND hwnd=(HWND)lpParam;
        char tBuf[1000];
        memset(tBuf, 0, 1000);
        sprintf(tBuf, "%04d  ", g_chess_game.get_timeout(SIDE_BLACK));
        SetWindowText(tmrLocalHd, tBuf);
        memset(tBuf, 0, 1000);
        sprintf(tBuf, "%04d  ", g_chess_game.get_timeout(SIDE_RED));
        MESS_PRINT("%d", timer_count);
        MESS_PRINT("%d", g_chess_game.get_timeout(SIDE_BLACK));
        SetWindowText(tmrRemoHd, tBuf);
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

//entry of program
int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow
        )
{
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
                g_chess_game.reset();

                //create three button
                CreateWindow("Button", "Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        5, 5, 60, 20, hwnd, (HMENU)IDB_FIVE, hg_app, NULL);
                Button1Hd = CreateWindow("Button", "Switch", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        60, 610, 60, 20, hwnd, (HMENU)IDB_ONE, hg_app, NULL);
                Button2Hd = CreateWindow("Button", "Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        135, 610, 60, 20, hwnd, (HMENU)IDB_TWO, hg_app, NULL);
                Button3Hd = CreateWindow("Button", "Retract", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        215, 610, 60, 20, hwnd, (HMENU)IDB_THREE, hg_app, NULL);
                Button4Hd = CreateWindow("Button", "Give", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        290, 610, 60, 20, hwnd, (HMENU)IDB_FOUR, hg_app, NULL);
                EnableWindow(Button1Hd, false);
                EnableWindow(Button2Hd, false);
                EnableWindow(Button3Hd, false);
                EnableWindow(Button4Hd, false);
                //CreateWindow(TEXT("edit"),TEXT("myedit"),WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,
                editHd = CreateWindow(TEXT("edit"),TEXT("ip addr"),WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                        100, 73, 190, 20, hwnd,(HMENU)ID_DATA, hg_app,NULL);
                // y cordinate, base
                int yLoc = 0;
                // text
                yLoc += 10;
                CreateWindow("Static","server ip",
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
            SetWindowText(hwnd, "changed");
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
            //timer init
            // Create the timer queue.
            hTimerQueue = CreateTimerQueue();
            if (NULL == hTimerQueue)
            {
                MessageBox(hwnd, "CreateTimerQueue failed", "Error", MB_ICONERROR);
                break;
            }
            // Set a timer to call the timer routine in 10 seconds.
            if (!CreateTimerQueueTimer( &hTimer, hTimerQueue,
                        (WAITORTIMERCALLBACK)TimerRoutine, hwnd , 2000, 1000, 0))
            {
                MessageBox(hwnd, "CreateTimerQueueTimer failed", "Error", MB_ICONERROR);
                break;
            }
            break;
        case WM_CONTEXTMENU:
            {
                //load menu rc
                //HMENU hroot = LoadMenu((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDR_CONTEXT));
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
            break;
        case WM_COMMAND:
            {
                // get Id of menu and check which menu user selected
                switch(LOWORD(wParam))
                {
                    case IDM_OPT1:
                        MessageBox(hwnd,"plane coming","Notice",MB_OK);
                        break;
                    case IDM_OPT2:
                        MessageBox(hwnd,"mt gun coming","notice",MB_OK);
                        break;
                    case IDB_ONE:
                        //MessageBox(hwnd, "you clicked first", "Notice", MB_OK | MB_ICONINFORMATION);
                        char szBuf[1000];
                        GetWindowText(editHd, szBuf, 1000);
                        MessageBox(hwnd, szBuf, "Notice", MB_OK | MB_ICONINFORMATION);
                        //SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"first clicked");
                        break;
                    case IDB_TWO:
                        //MessageBox(hwnd, "your clicked two", "Notice", MB_OK | MB_ICONINFORMATION);
                        //SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"second clicked");
                        g_chess_game.start();
                        break;
                    case IDB_THREE:
                        //MessageBox(hwnd, "you clicked tree", "notice", MB_OK | MB_ICONINFORMATION);
                        SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"third clicked");
                        break;
                    case IDB_FIVE:
                        {
                            HWND hdtmp = GetDlgItem(hwnd, IDB_FIVE);
                            EnableWindow(hdtmp, false);

                            if(Button_GetCheck(rb3Hd)){
                                running_mode = LOCAL_MODE;
                                chess_playing_handle[SIDE_RED] = SCREEN_CLICK_TYPE;
                                chess_playing_handle[SIDE_BLACK] = SCREEN_CLICK_TYPE;
                                MESS_PRINT("Running as local");
                            }
                            if(Button_GetCheck(rb2Hd)){
                                running_mode = CLIENT_MODE;
                                MESS_PRINT("Running as client");
                            }
                            if(Button_GetCheck(rb1Hd)){
                                running_mode = SERVER_MODE;
                                MESS_PRINT("Running as server");
                            }

                            enable_by_id(IDC_RADBTN1, 0);
                            enable_by_id(IDC_RADBTN2, 0);
                            enable_by_id(IDC_RADBTN3, 0);
                            enable_by_id(ID_DATA, 0);
                            EnableWindow(Button1Hd, true);
                            EnableWindow(Button2Hd, true);
                            EnableWindow(Button3Hd, true);
                            EnableWindow(Button4Hd, true);
                            InvalidateRect(hwnd,NULL,TRUE);
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
                if(running_mode != TBD){
#if 1
                    for(int i = 0;i < CP_NUM_MAX;i++){
                        chess_piece * cptmp = g_chess_game.get_cp((CHESS_PIECES_INDEX)i);
                        if(cptmp && cptmp->is_alive()){
                            //MESS_PRINT("bitblt %d %d",  cptmp->get_p_x(), cptmp->get_p_y());
                            BitBlt(hdc, chess_to_display_x(cptmp->get_p_x())-CELL_SIZE/2, chess_to_display_y(cptmp->get_p_y())-CELL_SIZE/2, rt.right, rt.bottom, s_hdcMemCP[cp_display_map[i]], 0, 0, SRCCOPY);
                        }
                    }
#else
                    for(int i = 0; i<MAX_CHS_BOARD_Y;i++){
                        for(int j = 0; j<MAX_CHS_BOARD_X;j++){
                            //cpes_board[i][j] = NULL;
                            chess_piece * cptmp = g_chess_game.get_cp(j,i);
                            if(cptmp && cptmp->is_alive()){
                                //MESS_PRINT("bitblt %d %d",  cptmp->get_p_x(), cptmp->get_p_y());
                                BitBlt(hdc, chess_to_display_x(cptmp->get_p_x())-CELL_SIZE/2, chess_to_display_y(cptmp->get_p_y())-CELL_SIZE/2, rt.right, rt.bottom, s_hdcMemCP[cp_display_map[cptmp->get_cpid()]], 0, 0, SRCCOPY);
                            }
                        }
                    }
#endif
                }
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
            // Delete all timers in the timer queue.
            if (!DeleteTimerQueue(hTimerQueue))
                printf("DeleteTimerQueue failed (%d)\n", GetLastError());
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
            "Opera");
    //one way is using AppendMenu
    AppendMenu(pop1,
            MF_STRING,
            IDM_OPT1,
            "Plane");

    //another way is using InsertMenuItem
    MENUITEMINFO mif;
    mif.cbSize = sizeof(MENUITEMINFO);
    mif.cch = 100;
    mif.dwItemData  = NULL;
    mif.dwTypeData = "MT_GUN";
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
