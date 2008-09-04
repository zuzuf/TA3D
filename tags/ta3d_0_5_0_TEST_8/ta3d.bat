echo off

set param=

:restart

ta3d-bin.exe %param% %1 %2 %3 %4 %5 %6 %7 %8 %9

if errorlevel 3 goto restore
if errorlevel 2 goto quickrestart

goto end

:restore
set param=--quick-restart --restore
goto restart

:quickrestart
set param=--quick-restart
goto restart

:end
