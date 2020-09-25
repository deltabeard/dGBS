ECHO OFF

IF "%VSCMD_VER%"=="" (
	ECHO You must run this within Visual Studio Native Tools Command Line
	EXIT /B 1
)

WHERE GNUMAKE.EXE >nul 2>nul
IF %ERRORLEVEL% NEQ 0 (
	ECHO GNUMAKE.EXE was not found
	EXIT /B 1
)

GNUMAKE.EXE CC="cl" OBJEXT="obj" SOUND="FILE" RM=del EXEOUT="/Fe" CFLAGS="/nologo /analyze /diagnostics:caret /O2 /GF /Zo- /fp:precise /W3 /DSOUND_FILE" %1
