;TA3D Setup Script
;--------------------------------

!ifndef VERSION
  !define VERSION '0.6.0'
!endif

!define PRODUCT_NAME "TA3D" 
!define PRODUCT_VERSION '${VERSION}'
!define PRODUCT_PUBLISHER "The TA3D Team"
!define PRODUCT_WEB_SITE "http://www.ta3d.org"
!define PRODUCT_DIR_REGKEY "Software\TA3D"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
 
!define PRODUCT_ID "{ac266b10-4916-11dd-ae16-0800200c9a66}"


!define TA3D_BIN "ta3d.exe"

;Uninstaller log file management
;--------------------------------

!define UninstLog "uninstall.log"
Var UninstLog
 
; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"
 
; AddItem macro
!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; Copy files macro
!macro CopyFiles SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"
 
; Rename macro
!macro Rename SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"
 
; CreateDirectory macro
!macro CreateDirectory Path
 CreateDirectory "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
; WriteUninstaller macro
!macro WriteUninstaller Path
 WriteUninstaller "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"
 
Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

;--------------------------------
;Configuration

!ifdef OUTFILE
  OutFile "${OUTFILE}"
!else
  OutFile ta3d-${PRODUCT_VERSION}-setup.exe
!endif

SetCompressor /SOLID lzma

InstallDir $PROGRAMFILES\TA3D
InstallDirRegKey HKLM ${PRODUCT_DIR_REGKEY} ""

ShowInstDetails show
ShowUnInstDetails show

InstType "Full (Recommended)"
InstType "Lite (without the default TA3D Mod)"

RequestExecutionLevel user

;--------------------------------
;Header Files

!include "MUI2.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "Memento.nsh"
!include "WordFunc.nsh"

;--------------------------------
;Functions

!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  !insertmacro VersionCompare
!endif

;--------------------------------
;Definitions

!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0

;--------------------------------
;Configuration

;Names
Name "TA3D - A Remake of Total Annhilation"
Caption "TA3D ${VERSION} Setup"

;Memento Settings
!define MEMENTO_REGISTRY_ROOT HKLM
!define MEMENTO_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"

;Interface Settings
!define MUI_ABORTWARNING

; MUI Settings / Icons
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"



!define MUI_HEADERIMAGE
; MUI Settings / Wizard
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"



!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TITLE "Welcome to the TA3D ${VERSION} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of TA3D (A remake of Total Annhilation) ${VERSION}.$\r$\n$\r$\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the TA3D site for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"

!define MUI_FINISHPAGE_RUN "$INSTDIR\ta3d.exe"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show release notes"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReleaseNotes


!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

 # These indented statements modify settings for MUI_PAGE_FINISH
    !define MUI_FINISHPAGE_NOAUTOCLOSE
  !insertmacro MUI_PAGE_FINISH


Function LaunchLink
  ExecShell "" "$INSTDIR\ta3d.lnk"
FunctionEnd

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Russian"

!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections


${MementoSection} "TA3D Core Files [mingw] (required)" SecCore

  SetDetailsPrint textonly
  DetailPrint "Installing TA3D Core Files..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 4 RO
  ${SetOutPath} $INSTDIR

  SetOverwrite on
  ${File} "" "3dmeditor.ico"
  ${File} "" "ta3d.ico"
  ${File} "..\..\" "3dmeditor.exe"
  ${File} "..\..\" "${TA3D_BIN}"
  ${File} "..\..\" "install.bat"
  ${File} "..\..\" "AUTHORS"
  ${File} "..\..\" "COPYING"
  ${File} "..\..\src\tools\win32\mingw32\libs\" "fmodex.dll"
  ${File} "..\..\src\tools\win32\mingw32\libs\" "alleg42.dll"
  ${File} "..\..\src\tools\win32\mingw32\libs\" "nl.dll"
  ${File} "..\..\src\tools\win32\mingw32\libs\" "zlib1.dll"

${MementoSectionEnd}

${MementoSection} "Resources (required)" SecResources

  SetDetailsPrint textonly
  DetailPrint "Installing Resources..."
  SetDetailsPrint listonly

  SetOverwrite on
  SectionIn 1 2 3 4 RO
  
  ${SetOutPath} "$INSTDIR\Resources\"
  ${File} "..\..\resources\" "ta3d.res"
  ${File} "..\..\resources\" "3dmeditor.res"
  ${SetOutPath} "$INSTDIR\Resources\Languages"
  ${SetOutPath} "$INSTDIR\Resources\Intro"
  ${File} "..\..\resources\intro\" "*.txt"
  
  ${SetOutPath} "$INSTDIR\Gfx\"
  ${File} "..\..\gfx\" "*.tga"
  ${File} "..\..\gfx\" "*.jpg"
  ${SetOutPath} "$INSTDIR\Gfx\default_skin\"
  ${File} "..\..\gfx\default_skin\" "*.tga"
  ${SetOutPath} "$INSTDIR\Gfx\mdrn_skin\"
  ${File} "..\..\gfx\mdrn_skin\" "*.tga"
  ${SetOutPath} "$INSTDIR\Gfx\mdrn_teams\"
  ${File} "..\..\gfx\mdrn_teams\" "*.tga"
  ${SetOutPath} "$INSTDIR\Gfx\Sky\"
  ${File} "..\..\gfx\sky\" "*.jpg"
  ${SetOutPath} "$INSTDIR\Gfx\Tactical Icons\"
  ${File} "..\..\gfx\tactical_icons\" "*.tga"
  ${SetOutPath} "$INSTDIR\Gfx\Teams\"
  ${File} "..\..\gfx\teams\" "*.tga"


  ${SetOutPath} "$INSTDIR\Ai\"
  ${File} "..\..\ai\" "*.ai"

  ${SetOutPath} "$INSTDIR\Gui"
  ${File} "..\..\gui\" "*.tdf"
  ${File} "..\..\gui\" "*.area"
  ${File} "..\..\gui\" "*.skn"

  ${SetOutPath} "$INSTDIR\Music\"
  #${File} "..\..\music\" "*"

  ${SetOutPath} "$INSTDIR\Cache\"
  #${File} "..\..\cache\" "*"

  ${SetOutPath} "$INSTDIR\Scripts\"
  ${File} "..\..\scripts\" "*.lua"
  ${File} "..\..\scripts\" "*.h"

  ${SetOutPath} "$INSTDIR\Shaders\"
  ${File} "..\..\shaders\" "*.vert"
  ${File} "..\..\shaders\" "*.frag"

  ${SetOutPath} "$INSTDIR\Sky\"
  ${File} "..\..\sky\" "*.tdf"

  ${SetOutPath} "$INSTDIR\Objects3D\"
  ${File} "..\..\objects3d\" "*.3dm"
  ${File} "..\..\objects3d\" "*.3do"

${MementoSectionEnd}

${MementoSection} "Documentation" SecDocs

  SetDetailsPrint textonly
  DetailPrint "Installing Documentation files..."
  SetDetailsPrint listonly

  SectionIn 1 2 

  SetOverwrite on
  ${SetOutPath} "$INSTDIR\Docs"
  ${File} "..\..\docs\user\" "*.html"
  ${File} "..\..\docs\user\" "*.jpg"
  ${SetOutPath} "$INSTDIR\Docs\res"
  ${File} "..\..\docs\user\res\" "*.jpg"

${MementoSectionEnd}

${MementoSection} "Default Mod" SecDefaultMod

  SetDetailsPrint textonly
  DetailPrint "Installing the TA3D default mod..."
  SetDetailsPrint listonly

  SectionIn 1 

  SetOverwrite on
  ${AddItem} "$INSTDIR\Mods\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\"
  ${File} "..\..\mods\ta3d\" "*.sh"
  ${File} "..\..\mods\ta3d\" "*.txt"
  ${File} "..\..\mods\ta3d\" "TODO"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Anims\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Download\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\acid\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\all\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\archi\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\corpses\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\crystal\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\desert\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\green\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\ice\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\lava\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\lush\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\mars\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\metal\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\moon\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\slate\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\urban\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\water\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\wetdesert\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Features\worlds\"
  
  ${SetOutPath} "$INSTDIR\Mods\TA3D\GameData\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Guis\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Objects3D\"
  ${File} "..\..\mods\ta3d\objects3d\" "*.3dm"
  ${File} "..\..\mods\ta3d\objects3d\" "*.3do"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Scripts\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Sounds\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Textures\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\UnitPics\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Units\"
  ${SetOutPath} "$INSTDIR\Mods\TA3D\Weapons\"

${MementoSectionEnd}


${MementoSection} "Desktop Shortcut" SecDekstopShortcuts

  SetDetailsPrint textonly
  DetailPrint "Installing Desktop shortcut..."
  SetDetailsPrint listonly
  
CreateShortCut "$DESKTOP\TA3D.lnk" "$INSTDIR\ta3d.exe" "" "$INSTDIR\ta3d.ico"

${MementoSectionEnd}

${MementoSection} "Menu Shortcuts" SecMenuShortcuts

  SetDetailsPrint textonly
  DetailPrint "Installing Menu shortcuts..."
  SetDetailsPrint listonly
  
  SectionIn 1 2 
  
  CreateDirectory "$SMPROGRAMS\TA3D"
  CreateDirectory "$SMPROGRAMS\TA3D\Documentation"
  ${SetOutPath} "$INSTDIR\"
  CreateShortCut "$SMPROGRAMS\TA3D\TA3D.lnk" "$INSTDIR\ta3d.exe" "" "$INSTDIR\ta3d.ico"
  CreateShortCut "$SMPROGRAMS\TA3D\3DMEditor.lnk" "$INSTDIR\3dmeditor.exe" "" "$INSTDIR\3dmeditor.ico"
  CreateShortCut "$SMPROGRAMS\TA3D\Documentation\User Guide.lnk" "$INSTDIR\readme.html"
  CreateShortCut "$SMPROGRAMS\TA3D\Documentation\User Guide (Francais).lnk" "$INSTDIR\readme-fr.html"

${MementoSectionEnd}


${MementoSectionDone}


Section -post

  ${WriteUninstaller} $INSTDIR\uninstall.exe

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "InstallDir" $INSTDIR
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Version" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${TA3D_BIN}"
  
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
     "DisplayName" "$(^Name)" 
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
     "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
     "DisplayIcon" "$INSTDIR\${TA3D_BIN}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
     "DisplayVersion" "${PRODUCT_VERSION}" 
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "Publisher" "${PRODUCT_PUBLISHER}"
 


SectionEnd
#SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecTA3DGroup} "TA3D - Remake of Total Annhilation"
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The core files required to use TA3D"
!insertmacro MUI_DESCRIPTION_TEXT ${SecResources} "Resources used by TA3D"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDocs} "Documentation related to TA3D"
!insertmacro MUI_DESCRIPTION_TEXT ${SecMenuShortcuts} "Menu shortcuts"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDekstopShortcuts} "Desktop shortcut"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDefaultMod} "Default MOD for TA3D"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Installer Functions

Function .onInit

  ${MementoSectionRestore}

FunctionEnd

!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD

# TODO Make a reInstall function

!endif # VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD



Function ShowReleaseNotes
   ExecShell "open" "${PRODUCT_WEB_SITE}/home-en.php"
FunctionEnd

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd

;--------------------------------
;Uninstaller Section
 
Section Uninstall
 
  SetDetailsPrint textonly
  DetailPrint "Uninstalling TA3D..."
  SetDetailsPrint listonly

 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort

  SetDetailsPrint textonly
  DetailPrint "Deleting Registry Keys..."
  SetDetailsPrint listonly

  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  SetDetailsPrint textonly
  DetailPrint "Deleting Files..."
  SetDetailsPrint listonly
 
 Push $R0
 Push $R1
 Push $R2
 SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
 FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
 StrCpy $R1 0
 
 GetLineCount:
  ClearErrors
   FileRead $UninstLog $R0
   IntOp $R1 $R1 + 1
   IfErrors 0 GetLineCount
 
 LoopRead:
  FileSeek $UninstLog 0 SET
  StrCpy $R2 0
  FindLine:
   FileRead $UninstLog $R0
   IntOp $R2 $R2 + 1
   StrCmp $R1 $R2 0 FindLine
 
   StrCpy $R0 $R0 -2
   IfFileExists "$R0\*.*" 0 +3
    RMDir $R0  #is dir
   Goto +3
   IfFileExists $R0 0 +2
    Delete $R0 #is file
 
  IntOp $R1 $R1 - 1
  StrCmp $R1 0 LoopDone
  Goto LoopRead
 LoopDone:
 FileClose $UninstLog
 Delete "$INSTDIR\${UninstLog}"
 RMDir "$INSTDIR"
 Pop $R2
 Pop $R1
 Pop $R0

  RMDir /r "$INSTDIR\Cache"
  RMDir /r "$SMPROGRAMS\TA3D"
  delete "$DESKTOP\TA3D.lnk"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
 
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  SetDetailsPrint both
SectionEnd
