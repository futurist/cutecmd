' See: http://superuser.com/questions/929225/how-to-run-a-program-as-an-administrator-at-startup-on-windows-10
' Windows 10: In order to use Administrative Privilege, put this script into:
' C:\Users\Administrator\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
' And set cutecmd.exe to run as Administrative Privilege

Set WshShell = CreateObject("WScript.Shell" )
WshShell.Run chr(34) & "C:\Windows\cutecmd.exe" & Chr(34), 0
Set WshShell = Nothing
