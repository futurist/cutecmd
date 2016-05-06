#include <windows.h>

LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);

void KeyboardHook();
void LowLevelKeyboardHook(HINSTANCE);
void SetKeyboardHook(int, HOOKPROC, HINSTANCE, DWORD);



// source code from : https://github.com/derUnbekannt/keyboard-hook


HHOOK hhkKeyboard;


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
	//	TEXT("Enter Key is now disabled.\n")
	//	TEXT("Click \"Ok\" to terminate this application and re-enable that key."),
	//	TEXT("Disable Low-Level Keys"), MB_OK);

	// UnhookWindowsHookEx(hhkKeyboard);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL bKeyHooked = FALSE;

	if (nCode == HC_ACTION)
	{
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

		switch (wParam)
		{
		case WM_KEYDOWN:
		//case WM_KEYUP:
		case WM_SYSKEYDOWN:
		//case WM_SYSKEYUP:
		{
			//bKeyHooked = (p->vkCode == VK_RETURN);
			//winexec("c:\\windows\\notepad.exe");
			//bKeyHooked = (( p->vkCode == VK_SPACE ) &&
   //                        (( p->flags & LLKHF_ALTDOWN ) != 0 )) ||
   //                        (( p->vkCode == VK_ESCAPE ) &&
   //                        (( p->flags & LLKHF_ALTDOWN ) != 0 )) ||
   //                        (( p->vkCode == VK_ESCAPE ) &&
   //                        (( GetKeyState( VK_CONTROL ) & 0x8000) != 0 ));

			bKeyHooked = (( p->vkCode == VK_SPACE ) &&
                           (( GetKeyState( VK_LCONTROL ) & 0x8000) != 0 ) &&
						   ((GetKeyState( VK_LSHIFT ) & 0x8000) != 0)
						   );

			break;
		}
		}
	}
	
	if(bKeyHooked) ShellExecute( NULL, L"open", L"C:\\WINDOWS\\system32\\rundll32.exe", L"shell32.dll,#61", NULL, SW_SHOWNORMAL );

	return (bKeyHooked ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
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

