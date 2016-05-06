; #include <Array.au3>

;;; This Au3 script need 3 param:

;; au3.au3 exeName.exe [title|class] Title_or_Class_to_activate

;; It will check for title or class, WinActivate the window when visible, else run exeName.exe


Local $title
Local $prev = "[REGEXPTITLE:^运行$]"

While BitAND(2, WinGetState ($prev) )
   WinClose($prev)
WEnd

If $CmdLine[0]<3 Then Exit

; MsgBox(0, "", WinGetState ("[REGEXPTITLE:^运行$]") )

If $CmdLine[2]="title" Then
   $title = ('[REGEXPTITLE:' & $CmdLine[3] & ']')
EndIf

If $CmdLine[2]="class" Then
   $title = ('[CLASS:' & $CmdLine[3] & ']')
EndIf

; MsgBox(0, $title, BitAND(2, WinGetState ($title) ) )

if BitAND(2, WinGetState ($title) ) Then

   WinActivate($title)

Else

   ShellExecute ($CmdLine[1])

EndIf



