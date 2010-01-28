@echo off
TITLE Total Annihilation files installer

goto :program

:copy_file
FOR %%l IN (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z) DO FOR %%f IN (%%l:\%1*) DO @copy /Y %%f resources\
goto :eof

:copy_file_from_hpi
FOR %%l IN (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z) DO FOR %%f IN (%%l:\%1*) DO @ta3d --quiet --install %%f %2
goto :eof

:header
cls
echo *************************************************************
echo *                                                           *
echo *             Total Annihilation files installer            *
echo *                                                           *
echo *  Installateur de ressources Total Annihilation pour TA3D  *
echo *                                                           *
echo *************************************************************
echo.
goto :eof

:program

call :header
echo You will be asked for your CDs during the install process.
echo.
echo NB: If an error message shows up just click continue, this is due
echo     to drive detection
echo.
pause

# OTA CD 1
call :header
echo please mount/insert your TA cdrom now (TA CD 1)
echo.
pause
echo searching files
call :copy_file totala1.hpi
call :copy_file totala2.hpi
call :copy_file totala4.hpi
call :copy_file_from_hpi totala3.hpi install\totala1.hpi

# OTA CD 2
call :header
echo please mount/insert your TA cdrom now (TA CD 2)
echo.
pause
echo searching files
call :copy_file totala1.hpi
call :copy_file totala2.hpi
call :copy_file totala4.hpi
call :copy_file_from_hpi totala3.hpi install\totala1.hpi

# OTA Battle Tactics CD
call :header
echo please mount/insert your TA:Battle Tactics cdrom now (TA:BT CD) if you have it
echo.
pause
echo searching files
call :copy_file bt\btdata.ccx
call :copy_file bt\btmaps.ccx
call :copy_file bt\tactics1.hpi
call :copy_file bt\tactics2.hpi
call :copy_file bt\tactics3.hpi
call :copy_file bt\tactics4.hpi
call :copy_file bt\tactics5.hpi
call :copy_file bt\tactics6.hpi
call :copy_file bt\tactics7.hpi
call :copy_file bt\tactics8.hpi

# OTA Core Contingency CD
call :header
echo please mount/insert your TA:Core Contingency cdrom now (TA:CC CD) if you have it
echo.
pause
echo searching files
call :copy_file CC\ccdata.ccx
call :copy_file CC\ccmaps.ccx
call :copy_file CC\ccmiss.ccx

:end
cls
echo ************************************************************
echo *                                                          *
echo *    Installation is finished you can now play the game    *
echo *                                                          *
echo ************************************************************
echo.
pause

TITLE cmd
:eof