#include <windows.h>
#include <stdio.h>
/* #define _DEBUG */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef _DEBUG
#include <stdio.h>
bool _trace(TCHAR *format, ...)
{
  TCHAR buffer[1000];

  va_list argptr;
  va_start(argptr, format);
  wvsprintf(buffer, format, argptr);
  va_end(argptr);

  OutputDebugString(buffer);

  return true;
}
#endif

LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
//HINSTANCE ShellExecute( HWND ,  LPCTSTR , LPCTSTR ,  LPCTSTR ,  LPCTSTR , INT  );

void SetKeyboardHook(int, HOOKPROC, HINSTANCE, DWORD);


HHOOK hhkKeyboard;
LONG commandMode = 0;
LONG prevTime = 0;
LONG timeDiff = 9999;
BOOL bCtrlG = FALSE;

LONG winWidth = 200;
LONG winHeight = 90;
LONG winLeftPos = 300;
LONG winTopPos = 300;
float winPosRatio = .3;

HMENU ID_EDIT1 =  (HMENU)0x8801;
HMENU ID_BUTTON_OK = (HMENU)0x8802;
HMENU ID_BUTTON_CANCEL = (HMENU)0x8803;

HWND hWnd;
HWND hWndEdit;
HWND hWndOK;
HWND hWndCancel;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void SetForegroundWindowInternal(HWND hWnd)
{
    if(!IsWindow(hWnd)) return;

    //relation time of SetForegroundWindow lock
    DWORD lockTimeOut = 0;
    HWND  hCurrWnd = GetForegroundWindow();
    DWORD dwThisTID = GetCurrentThreadId(),
          dwCurrTID = GetWindowThreadProcessId(hCurrWnd,0);

    //we need to bypass some limitations from Microsoft :)
    if(dwThisTID != dwCurrTID)
    {
        AttachThreadInput(dwThisTID, dwCurrTID, TRUE);

        SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&lockTimeOut,0);
        SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

        AllowSetForegroundWindow(ASFW_ANY);
    }

    SetForegroundWindow(hWnd);

    if(dwThisTID != dwCurrTID)
    {
        SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(PVOID)lockTimeOut,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
        AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
    }
}


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

void ShowCmd(){
  ShowWindow(hWnd, SW_SHOW);
  SetForegroundWindowInternal(hWnd);
  SetFocus(hWndEdit);
}

void HideCmd(){
  prevTime = 0;
  commandMode = 0;
  ShowWindow(hWnd, SW_HIDE);
}

HINSTANCE RunCmd(){
  char cmd[1024];
  GetWindowText(hWndEdit, cmd, sizeof(cmd));
  SetWindowText(hWndEdit, "");
  char *cmd2 = TrimWhiteSpace(cmd);
  if(strlen(cmd2)==0) return 0;
  return (HINSTANCE)ShellExecuteA( NULL, NULL, cmd2, NULL, NULL, SW_SHOWNORMAL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Low-level keyboard hook
  SetKeyboardHook(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

  winLeftPos = (GetSystemMetrics(SM_CXSCREEN) - winWidth) * winPosRatio;  // left pos always in ratio
  winTopPos = (GetSystemMetrics(SM_CYSCREEN) - winHeight) * 0.5;  // Init top is in center

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
  wcex.style = CS_HREDRAW | CS_VREDRAW;
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


  LONG window_style = GetWindowLong(hWnd, GWL_STYLE);
  LONG window_ex_style = GetWindowLong(hWnd, GWL_EXSTYLE);

  /* SetWindowLong(hWnd, GWL_STYLE, 0); */
  /* SetWindowLong(hWnd, GWL_STYLE, window_style & ~(WS_CAPTION | WS_THICKFRAME)); */
  /* SetWindowLong(hWnd, GWL_EXSTYLE, window_ex_style & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE)); */

  hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT(""),
                                 WS_CHILD | WS_VISIBLE | WS_GROUP, 10, 10, 180,
                                 35, hWnd, ID_EDIT1, NULL, NULL);

  hWndOK = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Button"), TEXT("Run"),
                                 WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 50, 80,
                                 30, hWnd, ID_BUTTON_OK, NULL, NULL);

  hWndCancel = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Button"), TEXT("Cancel"),
                                 WS_CHILD | WS_VISIBLE, 110, 50, 80,
                                 30, hWnd, ID_BUTTON_CANCEL, NULL, NULL);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  SetFocus(hWndEdit);
  HideCmd();

  MSG Msg;
  while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);

    }


  UnhookWindowsHookEx(hhkKeyboard);


  return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
      /* case WM_KILLFOCUS: */
    case WM_SETFOCUS:
      ShowCmd();
      return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
      switch(wParam)
        {
        case 0:  //FALSE or WM_INACTIVE
          HideCmd();
          break;
        case 1:  //TRUE or WM_ACTIVE or WM_CLICKACTIVE
        case 2:
          /* ShowCmd(); */
          break;
        }
      return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_COMMAND:
      if((HMENU)LOWORD(wParam) == (HMENU)ID_BUTTON_OK) { // HIWORD(wParam) == BN_CLICKED
        /* MessageBox (hWnd, "The Enter/Return key was pressed", txt, MB_OK); */
        RunCmd();
        HideCmd();
      }
      if((HMENU)LOWORD(wParam) == (HMENU)ID_BUTTON_CANCEL) {
        HideCmd();
      }
      return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_DESTROY:
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

  // Keep this app running until we're told to stop

  //MessageBox(NULL,
  //  TEXT("Enter Key is now disabled.\n")
  //  TEXT("Click \"Ok\" to terminate this application and re-enable that key."),
  //  TEXT("Disable Low-Level Keys"), MB_OK);

  // UnhookWindowsHookEx(hhkKeyboard);
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
        bKeyHooked = 0&& (( p->vkCode == VK_SPACE ) &&
                          (( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ) &&
                          ((GetKeyState( VK_LSHIFT ) & 0x8000) != 0)
                          );

        if(commandMode){
          prevTime = 0;
          if( p->vkCode == VK_UP){
            winTopPos = MAX(0, winTopPos-100);
            SetWindowPos(hWnd, NULL, winLeftPos, winTopPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }

          if( p->vkCode == VK_DOWN){
            winTopPos = MIN(winTopPos+100, GetSystemMetrics(SM_CYSCREEN)-100);
            SetWindowPos(hWnd, NULL, winLeftPos, winTopPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
          }

          if( p->vkCode == VK_RETURN || p->vkCode == VK_SPACE ){
            ShellRet = RunCmd();
          }

          /* if( p->vkCode == VK_SPACE ){ */
          /*   // ENTER key down */
          /*   keybd_event(VK_RETURN, 0x9C, 0, 0); */
          /*   // ENTER key up */
          /*   keybd_event(VK_RETURN, 0x9C, KEYEVENTF_KEYUP, 0); */
          /* } */

          bCtrlG = (p->vkCode== 0x47 && ( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ); /* Ctrl+G */
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
      if( bControl ){

        if(isUp) {

          timeDiff = GetTickCount() - prevTime;
          prevTime = GetTickCount();
#ifdef _DEBUG
          _trace( L"timeDiff is %d\n",  timeDiff );
#endif
          bKeyHooked = timeDiff<500;
        }
      } else {
        prevTime = 0;
      }
    }

  retVal = ( (bKeyHooked && !bControl || bCmdKey) ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
  if(bCmdKey) bCmdKey = FALSE;

  if(bKeyHooked){
    ShowCmd();
    commandMode = 1;
  }

  return retVal;
}

