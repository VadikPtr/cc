@echo off
Setlocal EnableDelayedExpansion
set COPYDST=%1

if not exist "%COPYDST%" (
  mkdir "%COPYDST%"
)

:loop
shift
if not "%1"=="" (
  set "COPYFILEDST=%COPYDST%\%~n1%~x1"
  @REM echo =======
  @REM echo COPY %1 TO
  @REM echo      !COPYFILEDST!
  if not exist "!COPYFILEDST!" (copy /Y "%1" "!COPYFILEDST!")
  goto :loop
)
