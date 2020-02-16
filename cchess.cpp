#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>

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

static char mpbuf[200];
#define MESS_PRINT(fmt,arg...) {sprintf(mpbuf, fmt, ##arg);SetWindowText(MessageHd, mpbuf);}
HMENU hRoot;
void CreateMyMenu();//create menu
int timer_count=0;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
char strbuf[128];
HINSTANCE hg_app;
HWND editHd;
HWND rb1Hd;
HWND tmrLocalHd;
HWND tmrRemoHd;
HWND startButtonHd;
HWND Button1Hd;
HWND Button2Hd;
HWND Button3Hd;
HWND Button4Hd;
HWND MessageHd;

void message_print(const char *fmt, ...);
LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    HWND hwnd;
    timer_count++;
    if (lpParam != NULL)
    {
        HWND hwnd=(HWND)lpParam;
        //InvalidateRect(hwnd,NULL,TRUE);
        char tBuf[1000];
        sprintf(tBuf, "%02d      ", timer_count);
        SetWindowText(tmrLocalHd, tBuf);
        //message_print("%d", timer_count);
        MESS_PRINT("%d", timer_count);
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
LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        )
{
    HBITMAP hBitmap;
    static HDC s_hdcMem;

    switch(uMsg)
    {
        case WM_CREATE:
            {
                //create three button
                CreateWindow("Button", "Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        5, 5, 60, 20, hwnd, (HMENU)IDB_FIVE, hg_app, NULL);
                Button1Hd = CreateWindow("Button", "B1", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        60, 610, 60, 20, hwnd, (HMENU)IDB_ONE, hg_app, NULL);
                Button2Hd = CreateWindow("Button", "B2", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        135, 610, 60, 20, hwnd, (HMENU)IDB_TWO, hg_app, NULL);
                Button3Hd = CreateWindow("Button", "B3", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        215, 610, 60, 20, hwnd, (HMENU)IDB_THREE, hg_app, NULL);
                Button4Hd = CreateWindow("Button", "B4", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
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
                CreateWindow("Button","local",
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
                CreateWindow("Button","client",
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
                message_print("wm create done");
            }

            memset(strbuf, 0, 128);
            SetWindowText(hwnd, "changed");
            // set dlg size changable
            //SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_SIZEBOX);
            // load image
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
                        if(Button_GetCheck(rb1Hd)){
                            sprintf(szBuf, "Male choosed");
                        }
                        MessageBox(hwnd, szBuf, "Notice", MB_OK | MB_ICONINFORMATION);
                        //SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"first clicked");
                        break;
                    case IDB_TWO:
                        //MessageBox(hwnd, "your clicked two", "Notice", MB_OK | MB_ICONINFORMATION);
                        SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"second clicked");
                        break;
                    case IDB_THREE:
                        //MessageBox(hwnd, "you clicked tree", "notice", MB_OK | MB_ICONINFORMATION);
                        SendMessage((HWND)lParam, WM_SETTEXT, (WPARAM)NULL, (LPARAM)"third clicked");
                        break;
                    case IDB_FIVE:
                        {
                            HWND hdtmp = GetDlgItem(hwnd, IDB_FIVE);
                            EnableWindow(hdtmp, false);
                            EnableWindow(Button1Hd, true);
                            EnableWindow(Button2Hd, true);
                            EnableWindow(Button3Hd, true);
                            EnableWindow(Button4Hd, true);
                        }
                        break;
                    default:
                        break;
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
#if 0
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

void message_print(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    sprintf(mpbuf, fmt, args);
    va_end(args);
    SetWindowText(MessageHd, mpbuf);
}
