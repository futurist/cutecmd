@rem Keep one frame: open existing frame or create new frame of Emacs
@rem see  https://www.reddit.com/r/emacs/comments/4hzhtf/how_to_keep_emacs_only_one_frame/

@d:\Emacs\bin\emacsclientw.exe -n --server-file "d:\Emacs\.emacs.d\server\server" --alternate-editor="d:\Emacs\bin\runemacs.exe" %1%
