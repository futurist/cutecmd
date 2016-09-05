#include <windows.h>
#include <stdio.h>
/* #define _DEBUG */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef _DEBUG
FILE* debugFile;
#endif

LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
//HINSTANCE ShellExecute( HWND ,  LPCTSTR , LPCTSTR ,  LPCTSTR ,  LPCTSTR , INT  );

void SetKeyboardHook(int, HOOKPROC, HINSTANCE, DWORD);


HHOOK hhkKeyboard;
LONG commandMode = 0;
LONG prevTime = 0;
LONG prevKey = 0;
LONG timeDiff = 9999;
BOOL bCtrlG = FALSE;
BOOL bCtrlF = FALSE;

LONG winWidth = 500;
LONG winHeight = 55;
LONG winLeftPos = 300;
LONG winTopPos = 300;
float winPosRatio = .3;

HMENU ID_EDIT1 =  (HMENU)0x8801;

char cmd[1024];  // cache command string
char seps[]   = " ,;\t\n"; // command param seperator

HWND hWnd;
HWND hWndEdit;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

char *TrimWhiteSpace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
HFONT hfReg;

void ClickOnWindow(){
  POINT p;
  GetCursorPos(&p);
  int screenX = GetSystemMetrics( SM_CXVIRTUALSCREEN );
  int screenY = GetSystemMetrics( SM_CYVIRTUALSCREEN );
  DWORD L = (DWORD)((winLeftPos+10)*(65535/screenX));
  DWORD T = (DWORD)((winTopPos+10)*(65535/screenY));
  mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, L, T, 0, 0);
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);  // relative pos
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);  // relative pos
  SetCursorPos(p.x, p.y);
}

void ShowCmd(){
  ShowWindow(hWnd, SW_SHOW);
  SetFocus(hWndEdit);
}

void HideCmd(){
  prevTime = 0;
  commandMode = 0;
  bCtrlF = FALSE;
  bCtrlG = FALSE;
  ShowWindow(hWnd, SW_HIDE);
}

HINSTANCE RunCmd(){
  GetWindowText(hWndEdit, cmd, sizeof(cmd));
  SetWindowText(hWndEdit, "");
  char *cmd2 = TrimWhiteSpace(cmd);
  if(strlen(cmd2)==0) return 0;

  int len = strlen(cmd2);
  char *command = strtok( cmd2, seps );
  char *args = (strlen(command)<len) ? cmd2+strlen(command)+1 : "";
  return (HINSTANCE)ShellExecuteA( NULL, NULL, command, args, NULL, SW_SHOWNORMAL);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
  debugFile = fopen("logFile.txt", "a");
#endif
  // Low-level keyboard hook
  SetKeyboardHook(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

  winLeftPos = (GetSystemMetrics(SM_CXVIRTUALSCREEN) - winWidth) * winPosRatio;  // left pos always in ratio
  winTopPos = (GetSystemMetrics(SM_CYVIRTUALSCREEN) - winHeight) * 0.5;  // Init top is in center

  LPTSTR windowClass = TEXT("KeyHook");
  LPTSTR windowTitle = TEXT("KeyHook Window");
  WNDCLASSEX wcex;

  wcex.cbClsExtra = 0;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.cbWndExtra = 0;
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hInstance = hInstance;
  wcex.lpfnWndProc = WndProc;
  wcex.lpszClassName = windowClass;
  wcex.lpszMenuName = NULL;
  wcex.style = CS_DROPSHADOW;     //CS_HREDRAW | CS_VREDRAW; don't use this 2 to save memory
  if (!RegisterClassEx(&wcex))
    {
      MessageBox(NULL, TEXT("RegisterClassEx Failed!"), TEXT("Error"),
                 MB_ICONERROR);
      return 1;
    }

  if (!(hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, windowClass, windowTitle, WS_POPUP,
                              winLeftPos, winTopPos, winWidth, winHeight, NULL, NULL, hInstance, NULL)))
    {
      MessageBox(NULL, TEXT("CreateWindow Failed!"), TEXT("Error"), MB_ICONERROR);
      return 1;
    }


  hWndEdit = CreateWindow(TEXT("Edit"), TEXT(""),
                          WS_CHILD | WS_VISIBLE | WS_GROUP | WS_BORDER, 10, 10, winWidth-20,
                          winHeight-20, hWnd, ID_EDIT1, NULL, NULL);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  HideCmd();

  hfReg = CreateFont(30, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, "Arial");
  SendMessage(hWndEdit, WM_SETFONT, (WPARAM)hfReg, MAKELPARAM(FALSE, 0));

  // Keep only one instance using mutex
  HANDLE m_hMutex;
  m_hMutex  =  CreateMutex(NULL, TRUE,  "KeyHookProg" );

  if ((m_hMutex  !=  NULL)  &&  (GetLastError()  ==  ERROR_ALREADY_EXISTS))
    {
      ReleaseMutex(m_hMutex);
      return  FALSE;
    }

  MSG Msg;
  while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);

    }

  // release mutex
  if (m_hMutex  !=  NULL)
    {
      ReleaseMutex(m_hMutex);
      CloseHandle(m_hMutex);
    }

  UnhookWindowsHookEx(hhkKeyboard);


  return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
      switch(wParam)
        {
        case 0:  //FALSE or WM_INACTIVE
          HideCmd();
          break;
        case 1:  //TRUE or WM_ACTIVE or WM_CLICKACTIVE
        case 2:
          ShowCmd();
          break;
        }
      return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_DESTROY:
#ifdef _DEBUG
      fclose(debugFile);
#endif
      PostQuitMessage(0);
    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
    }
  return FALSE;
}

void SetKeyboardHook(int idHook, HOOKPROC  lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
  // Install the keyboard hook
  hhkKeyboard = SetWindowsHookEx(idHook, lpfn, hMod, dwThreadId);
}



LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  BOOL bKeyHooked = FALSE;
  BOOL bControl = FALSE;
  BOOL bCmdKey = FALSE;
  BOOL isDown = FALSE;
  BOOL isUp = FALSE;
  BOOL retVal;
  HINSTANCE ShellRet=0;

  if (nCode == HC_ACTION)
    {
      PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

      isDown = wParam==WM_KEYDOWN || wParam==WM_SYSKEYDOWN;
      isUp = wParam==WM_KEYUP || wParam==WM_SYSKEYUP;

      if (isDown) {

        //** key is ctrl+shift+SPC
        /* bKeyHooked = (( p->vkCode == VK_SPACE ) && */
        /*                   (( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ) && */
        /*                   ((GetKeyState( VK_LSHIFT ) & 0x8000) != 0) */
        /*                   ); */

        if(commandMode){

          prevTime = 0;

          bCtrlG = (p->vkCode== 0x47 && ( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ); /* Ctrl+G */
          bCtrlF = (p->vkCode== 0x20 && ( GetKeyState( VK_LMENU ) & 0x8000) != 0 ); /* Ctrl+F */

          if( p->vkCode == VK_UP){
            winTopPos = MAX(0, winTopPos-100);
            SetWindowPos(hWnd, NULL, winLeftPos, winTopPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }

          if( p->vkCode == VK_DOWN){
            winTopPos = MIN(winTopPos+100, GetSystemMetrics(SM_CYVIRTUALSCREEN)-100);
            SetWindowPos(hWnd, NULL, winLeftPos, winTopPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }

          if( p->vkCode == VK_RETURN || !bCtrlF && p->vkCode == VK_SPACE ){
            ShellRet = RunCmd();
          }

          /* if( p->vkCode == VK_SPACE ){ */
          /*   // ENTER key down */
          /*   keybd_event(VK_RETURN, 0x9C, 0, 0); */
          /*   // ENTER key up */
          /*   keybd_event(VK_RETURN, 0x9C, KEYEVENTF_KEYUP, 0); */
          /* } */

          if( bCtrlF ){
            // Append Space
            SendMessage(hWndEdit, EM_REPLACESEL, 0, (LPARAM)TEXT(" "));
          }

          if( ShellRet> (HINSTANCE) 32 && // RunCmd succeed
              ( p->vkCode == VK_RETURN ||
                p->vkCode == VK_SPACE)
              || p->vkCode == VK_ESCAPE || bCtrlG
              ){
            HideCmd();
            bCmdKey = TRUE;
          }

        }
      }

      //** key is ctrl+ctrl within 500 ms
      bControl = (p->vkCode == VK_LCONTROL);
      if( bControl ) {

        if(isUp) {

#ifdef _DEBUG
          fprintf(debugFile, "%ld, %ld\n", prevTime, GetTickCount());
          /* _trace( L"timeDiff is %d\n",  timeDiff ); */
#endif

          timeDiff = GetTickCount() - prevTime;
          prevTime = GetTickCount();
          bKeyHooked = timeDiff<500;
        }
      } else {
        prevTime = 0;
        bKeyHooked = FALSE;
      }
    }

  retVal = ( (bKeyHooked && !bControl || bCmdKey) ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
  if(bCmdKey) bCmdKey = FALSE;

  if(bKeyHooked){
    ShowCmd();
    ClickOnWindow();
    commandMode = 1;
  }

  return retVal;
}
