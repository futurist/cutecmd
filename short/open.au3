#include <Array.au3>

;;; This Au3 script need 3 param:

;; au3.au3 exeName.exe [title|class] Title_or_Class_to_activate

;; It will check for title or class, WinActivate the window when visible, else run exeName.exe


Local $title
Local $prev = "[REGEXPTITLE:^运行$]"
Local $args = $CmdLine
Local $argsLen = $args[0]

While BitAND(2, WinGetState ($prev) )
   WinClose($prev)
WEnd

If $argsLen<3 Then Exit

If $argsLen==3 Then _ArrayInsert($args, 2, "")

; MsgBox(0, _ArrayPop($args), _ArrayToString($args))

Local $keyword = _Arraypop($args)
Local $method = _Arraypop($args)

; MsgBox(0, "", WinGetState ("[REGEXPTITLE:^运行$]") )

If $method="title" Then
   $title = ('[REGEXPTITLE:' & $keyword & ']')
EndIf

If $method="class" Then
   $title = ('[CLASS:' & $keyword & ']')
EndIf

; MsgBox(0, $title, BitAND(2, WinGetState ($title) ) )

if BitAND(2, WinGetState ($title) ) Then

   WinActivate($title)

Else

   ShellExecute ($args[1], $args[2])

EndIf



