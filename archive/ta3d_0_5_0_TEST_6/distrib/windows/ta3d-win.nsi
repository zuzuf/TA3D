;TA3D Setup Script
;--------------------------------

!ifndef VERSION
  !define VERSION '0.5.1'
!endif

!define PRODUCT_NAME "TA3D" 
!define PRODUCT_VERSION '${VERSION}'
!define PRODUCT_PUBLISHER "The TA3D Team"
!define PRODUCT_WEB_SITE "http://www.ta3d.org"
!define PRODUCT_DIR_REGKEY "Software\TA3D"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
 
!define PRODUCT_ID "{ac266b10-4916-11dd-ae16-0800200c9a66}"


!define TA3D_BIN "ta3d-bin.exe"


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

!define MUI_FINISHPAGE_RUN "$INSTDIR\${TA3D_BIN}"
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


${MementoSection} "TA3D Core Files (required)" SecCore

  SetDetailsPrint textonly
  DetailPrint "Installing TA3D Core Files..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 4 RO
  SetOutPath $INSTDIR

  SetOverwrite on
  File ..\..\3dmeditor.exe
  File ..\..\${TA3D_BIN}
  File ..\..\ta3d.bat
  File ..\..\AUTHORS
  File ..\..\src\tools\win32\fmodex.dll
  File ..\..\src\tools\win32\alleg42.dll

${MementoSectionEnd}

${MementoSection} "Resources (required)" SecResources

  SetDetailsPrint textonly
  DetailPrint "Installing Resources..."
  SetDetailsPrint listonly

  SetOverwrite on
  SectionIn 1 2 3 4 RO
  SetOutPath $INSTDIR\Resources\Gfx
  File /r ..\..\gfx\*

  SetOutPath $INSTDIR\Resources\Ai
  File /r ..\..\ai\*

  SetOutPath $INSTDIR\Resources\Gui
  File /r ..\..\gui\*

  SetOutPath $INSTDIR\Resources\Music
  File /r ..\..\music\*

  SetOutPath $INSTDIR\Resources\Cache
  File /r ..\..\cache\*

  SetOutPath $INSTDIR\Resources\Scripts
  File /r ..\..\scripts\*

  SetOutPath $INSTDIR\Resources\Shaders
  File /r ..\..\shaders\*

  SetOutPath $INSTDIR\Resources\Sky
  File /r ..\..\sky\*

  SetOutPath $INSTDIR\Resources\Pictures
  File /r ..\..\pictures\*

  SetOutPath $INSTDIR\Resources\Objects3D
  File /r ..\..\objects3d\*

${MementoSectionEnd}

${MementoSection} "Documentation" SecDocs

  SetDetailsPrint textonly
  DetailPrint "Installing Documentation files..."
  SetDetailsPrint listonly

  SectionIn 1 2 

  SetOverwrite on
  SetOutPath $INSTDIR\Docs
  File /r ..\..\docs\user\*

${MementoSectionEnd}

${MementoSection} "Default Mod" SecDefaultMod

  SetDetailsPrint textonly
  DetailPrint "Installing the TA3D default mod..."
  SetDetailsPrint listonly

  SectionIn 1 

  SetOverwrite on
  SetOutPath $INSTDIR\Resources\Mods\TA3D
  File /r ..\..\mods\ta3d\*

${MementoSectionEnd}


${MementoSection} "Desktop Shortcut" SecDekstopShortcuts

  SetDetailsPrint textonly
  DetailPrint "Installing Desktop shortcut..."
  SetDetailsPrint listonly
  
CreateShortCut "$DESKTOP\TA3D.lnk" "$INSTDIR\${TA3D_BIN}"

${MementoSectionEnd}

${MementoSection} "Menu Shortcuts" SecMenuShortcuts

  SetDetailsPrint textonly
  DetailPrint "Installing Menu shortcuts..."
  SetDetailsPrint listonly
  
  SectionIn 1 2 
  
  CreateDirectory "$SMPROGRAMS\TA3D"
  CreateDirectory "$SMPROGRAMS\TA3D\Documentation"
  CreateShortCut "$SMPROGRAMS\TA3D\TA3D.lnk" "$INSTDIR\${TA3D_BIN}"
  CreateShortCut "$SMPROGRAMS\TA3D\3DMEditor.lnk" "$INSTDIR\3dmeditor.exe"
  CreateShortCut "$SMPROGRAMS\TA3D\Documentation\User Guide.lnk" "$INSTDIR\readme.html"
  CreateShortCut "$SMPROGRAMS\TA3D\Documentation\User Guide (Français).lnk" "$INSTDIR\readme-fr.html"

${MementoSectionEnd}


${MementoSectionDone}


Section -post

  WriteUninstaller $INSTDIR\uninstall.exe

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


;--------------------------------
;Uninstaller Section

Section uninstall

  SetDetailsPrint textonly
  DetailPrint "Uninstalling TA3D..."
  SetDetailsPrint listonly
  
  SetDetailsPrint textonly
  DetailPrint "Deleting Registry Keys..."
  SetDetailsPrint listonly

  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  SetDetailsPrint textonly
  DetailPrint "Deleting Files..."
  SetDetailsPrint listonly

  RMDir /r $INSTDIR
  RMDir /r "$SMPROGRAMS\TA3D"
  delete "$DESKTOP\TA3D.lnk"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
 
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  SetDetailsPrint both

SectionEnd
