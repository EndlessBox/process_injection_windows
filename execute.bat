rm -rf %CD%\a.exe
gcc main.c utils.c process.c  "C:\Windows\System32\psapi.dll"
%CD%/a.exe