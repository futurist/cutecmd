;; Edit current files in Windows Explorer

; Windows Explorer on XP, Vista, 7, 8
$hExplorer = WinGetHandle( "[REGEXPCLASS:^(Cabinet|Explore)WClass$]" )
If Not $hExplorer Then Exit

; Shell object
$oShell = ObjCreate( "Shell.Application" )

; Find window
For $oWindow In $oShell.Windows()
  If $oWindow.HWND() = $hExplorer Then ExitLoop
Next

; Selected items

$HOME = EnvGet("HOME")

For $oItem In $oWindow.Document.SelectedItems()
    ; MsgBox(0, $oItem.path(), $oItem.isFolder )
    ; https://msdn.microsoft.com/en-us/library/windows/desktop/bb787810(v=vs.85).aspx
    If Not $oItem.isFolder Then ShellExecute($HOME & "\bin\emacsclientw.exe", " -n """ & $oItem.path() & """")
Next
