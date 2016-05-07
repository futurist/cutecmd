@rem using TinyCC as compiler
@rem First import the definitions in shell32.dll as below:
@rem -----------------------------------------------------------
@rem       C:\tcc>tiny_impdef.exe c:\WINDOWS\system32\shell32.dll
@rem       move shell32.def lib\
@rem       tcc keyhook.c -lshell32


tcc keyhook.c -lshell32
