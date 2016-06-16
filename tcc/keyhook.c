#include <windows.h>
#include <stdio.h>
/* #define _DEBUG */

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
LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);
//HINSTANCE ShellExecute( HWND ,  LPCTSTR , LPCTSTR ,  LPCTSTR ,  LPCTSTR , INT  );

void KeyboardHook();
void LowLevelKeyboardHook(HINSTANCE);
void SetKeyboardHook(int, HOOKPROC, HINSTANCE, DWORD);


HHOOK hhkKeyboard;
LONG commandMode = 0;
LONG prevTime = 0;
LONG timeDiff = 9999;
BOOL bCtrlG = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Low-level keyboard hook
  SetKeyboardHook(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

  // Keyboard hook
  //SetKeyboardHook(WH_KEYBOARD, KeyboardProc, (HINSTANCE)NULL, GetCurrentThreadId());

  MSG Msg;
  while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);

    }


  UnhookWindowsHookEx(hhkKeyboard);


  return 0;
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


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  BOOL bKeyHooked = FALSE;
  BOOL bControl = FALSE;
  BOOL isDown = FALSE;
  BOOL isUp = FALSE;
  BOOL retVal;

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
          if( p->vkCode == VK_ESCAPE ||
              p->vkCode == VK_RETURN ||
              p->vkCode == VK_SPACE
              ){
            commandMode = 0;
          }
          if( p->vkCode == VK_SPACE ){
            // ENTER key down
            keybd_event(VK_RETURN, 0x9C, 0, 0);
            // ENTER key up
            keybd_event(VK_RETURN, 0x9C, KEYEVENTF_KEYUP, 0);
          }

          bCtrlG = (p->vkCode== 0x47 && ( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ); /* Ctrl+G */
          if(bCtrlG){
            prevTime = 0;
            commandMode = FALSE;
            PostMessage(GetForegroundWindow(), WM_CLOSE, 0, 0);
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

  retVal = ( (bKeyHooked && !bControl ) ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));

  if(bKeyHooked){
    ShellExecuteA( NULL, "open", "C:\\WINDOWS\\system32\\rundll32.exe", "shell32.dll,#61", NULL, SW_SHOWNORMAL );
    /* Sleep(1000); */

  // show msgbox to AllowSetForegroundWindow()
  HWND  hCurrWnd = GetForegroundWindow();
  MessageBox(hCurrWnd, NULL, NULL, MB_ICONWARNING );


HWND hwnd = FindWindow(NULL, "Everything");

FILE *f = fopen("d:\\hooklog.txt", "w");
if (f == NULL)
{
    printf("Error opening file!\n");
    exit(1);
}

/* print some text */
const char *text = "Test activate window";
fprintf(f, "Target: %s\n", text);

/* print integers and floats */
 fprintf(f, "Find Window: %d, %p\n", AllowSetForegroundWindow(GetCurrentProcessId()), GetCurrentProcessId());

fclose(f);

    SetForegroundWindow(hwnd);
    SetForegroundWindowInternal(hwnd);
exit(1);


    commandMode = 1;
  }

  return retVal;
}



LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
  BOOL bKeyHooked = FALSE;

  if (code == HC_ACTION)
    {
      bKeyHooked = (wParam == VK_RETURN);
    }

  return (bKeyHooked ? 1 : CallNextHookEx(NULL, code, wParam, lParam));
}
