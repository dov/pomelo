Name "${NAME}"
OutFile "${OUTDIR}\Install${NAME_CAP}-${HOST}-${VERSION}-${COMMITID_SHORT}.exe"

!include "FileFunc.nsh"
!include "DumpLog.nsh"
!insertmacro GetTime

SetCompress force ; (can be off or force)
CRCCheck on ; (can be off)

LicenseText "Please read and accept the following license:"
LicenseData "COPYING"

BrandingText "Pomelo Installer"

InstallDir "$PROGRAMFILES64\${NAME_CAP}"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}" ""

; The text to prompt the user to enter a directory
DirText "Choose a folder in which to install ${NAME_CAP}"

; Show details
ShowInstDetails show

Icon ${ICON_DIR}/${ICON_NAME}.ico
UninstallIcon ${ICON_DIR}/${ICON_NAME}.ico
XPStyle on

Section "Info"
  # Print version info into the installer window
  ${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
  DetailPrint "Running installer on $3 $2-$1-$0 $4:$5:$6"
  DetailPrint "Sha1=${SHA1}"
  DetailPrint "Version=${VERSION}"
  DetailPrint "CommitTime=${COMMIT_TIME}"
SectionEnd
  
; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${NAME_CAP}"
  CreateShortCut "$SMPROGRAMS\${NAME_CAP}\Uninstall.lnk" "$INSTDIR\bin\uninst.exe" "" "$INSTDIR\uninst.exe" 0
  CreateShortCut "$SMPROGRAMS\${NAME_CAP}\${NAME_CAP}.lnk" "$INSTDIR\bin\${NAME_CAP}.exe" "" "$INSTDIR\${NAME_CAP}.exe" 0
SectionEnd

Section "" ; (default section)

  SetOutPath $INSTDIR
  
  File /oname=COPYING.txt COPYING
  File README.md
  
  SetOutPath $INSTDIR\bin
  File ${SYSROOT}\bin\libintl-8.dll
  File ${SYSROOT}\bin\iconv.dll
  File ${SYSROOT}\bin\libpcre-1.dll
  File ${SYSROOT}\bin\libpcre2-8-0.dll
  File ${SYSROOT}\bin\libgtk-3-0.dll
  File ${SYSROOT}\bin\libgdk-3-0.dll
  File ${SYSROOT}\bin\libssp-0.dll
  File ${SYSROOT}\bin\libgdk_pixbuf-2.0-0.dll
  File ${SYSROOT}\bin\libpixman-1-0.dll
  File ${SYSROOT}\bin\libffi-8.dll
  File ${SYSROOT}\bin\libgio-2.0-0.dll
  File ${SYSROOT}\bin\libcairo-2.dll
  File ${SYSROOT}\bin\libcairo-gobject-2.dll
  File ${SYSROOT}\bin\zlib1.dll
  File ${SYSROOT}\bin\libglib-2.0-0.dll
  File ${SYSROOT}\bin\libatk-1.0-0.dll
  File ${SYSROOT}\bin\libgobject-2.0-0.dll
  File ${SYSROOT}\bin\libgmodule-2.0-0.dll
  File ${SYSROOT}\bin\libgthread-2.0-0.dll
  File ${SYSROOT}\bin\libpango-1.0-0.dll
  File ${SYSROOT}\bin\libfribidi-0.dll
  File ${SYSROOT}\bin\libpangocairo-1.0-0.dll
  File ${SYSROOT}\bin\libpangoft2-1.0-0.dll
  File ${SYSROOT}\bin\libpangowin32-1.0-0.dll
  File ${SYSROOT}\bin\libpng16-16.dll
  File ${SYSROOT}\bin\libtiff-5.dll
  File ${SYSROOT}\bin\libjpeg-62.dll
  File ${SYSROOT}\bin\libfontconfig-1.dll
  File ${SYSROOT}\bin\libxml2-2.dll
  File ${SYSROOT}\bin\libfreetype-6.dll
  File ${SYSROOT}\bin\gdk-pixbuf-query-loaders.exe
  File ${SYSROOT}\bin\${LIBGCCDLL}
  File ${SYSROOT}\bin\libstdc++-6.dll
  File ${SYSROOT}\bin\libwinpthread-1.dll
  File ${SYSROOT}\bin\libgfortran-*.dll
  File ${SYSROOT}\bin\librsvg-2-2.dll
  File ${SYSROOT}\bin\libcroco-0.6-3.dll
  File ${SYSROOT}\bin\libxml2-2.dll
  File ${SYSROOT}\bin\libquadmath-0.dll
  File ${SYSROOT}\bin\libopenjp2.dll
  File ${SYSROOT}\bin\libsqlite3-0.dll
  File ${SYSROOT}\bin\libexpat-1.dll
  File ${SYSROOT}\bin\libbz2-1.dll
  #File ${SYSROOT}\bin\libgdkglext-win32-1.0-0.dll
  #File ${SYSROOT}\bin\libgtkglext-win32-1.0-0.dll
  File ${SYSROOT}\bin\glew32.dll
  File ${SYSROOT}\bin\libzip-5.dll
  File ${SYSROOT}\bin\libharfbuzz-0.dll
  File ${SYSROOT}\bin\libharfbuzz-icu-0.dll
  File ${SYSROOT}\bin\libharfbuzz-subset-0.dll
  File ${SYSROOT}\bin\gspawn-win64-helper.exe
  File ${SYSROOT}\bin\libglibmm-2.4-1.dll
  File ${SYSROOT}\bin\libatkmm-1.6-1.dll
  File ${SYSROOT}\bin\libpangomm-1.4-1.dll
  File ${SYSROOT}\bin\libcairomm-1.0-1.dll
  File ${SYSROOT}\bin\libgtkmm-3.0-1.dll
  File ${SYSROOT}\bin\libgdkmm-3.0-1.dll
  File ${SYSROOT}\bin\libgiomm-2.4-1.dll
  File ${SYSROOT}\bin\libgoocanvas-2.0-9.dll
  File ${SYSROOT}\bin\gdbus.exe
  File ${SYSROOT}\bin\libsigc-2.0-0.dll
  File ${SYSROOT}\bin\libmpfr-6.dll
  # Libepoxy in official package is broken, so we use an older working version!
  # File ${SYSROOT}\bin\libepoxy-0.dll
  File \usr\local\mingw64\bin\libepoxy-0.dll
  File \usr\local\mingw64\bin\libfmt.dll
  File \usr\local\mingw64\bin\libgmp-10.dll
  File \usr\local\mingw64\bin\libgoocanvasmm-2.0-6.dll
  File ${OUTDIR}\src\${NAME}.exe
  File ${OUTDIR}\src\libengine.dll
  File ${OUTDIR}\src\giv-widget\libgiv-widget.dll
  File ${OUTDIR}\src\giv-widget\gtk-image-viewer\libgtk_image_viewer.dll
  File ${OUTDIR}\src\giv-widget\agg\libagg.dll
  File ${OUTDIR}\src\giv-widget\plis\libplis.dll
  
  
  SetOutPath "$INSTDIR"
  File /r ${SYSROOT}\etc
  SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0\loaders
  File ${SYSROOT}\lib\gdk-pixbuf-2.0\2.10.0\loaders\*
  SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0
  File ${SYSROOT}\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache
  #SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\engines
  #File ${SYSROOT}\lib\gtk-2.0\2.10.0\engines\*
  
  SetOutPath $INSTDIR\share
  File /r ${SYSROOT}\share\themes
  SetOutPath $INSTDIR\share\icons
  File /r ${SYSROOT}\share\icons\Adwaita
  
  SetOutPath $INSTDIR\share\glib-2.0
  File /r ${SYSROOT}\share\glib-2.0\schemas
  
  # Build the gdk-pixbuf.loaders file automatically
  ExpandEnvStrings $0 %COMSPEC%
  nsExec::ExecToStack '"$0" /C ""$INSTDIR\bin\gdk-pixbuf-query-loaders" > "$INSTDIR\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache""'
  
  # SetOutPath $INSTDIR\share
  # File /r ${SYSROOT}\share\themes
  # SetOutPath $INSTDIR\share
  # File /r ${SYSROOT}\share\icons
  # SetOutPath $INSTDIR\share\glib-2.0
  # File /r ${SYSROOT}\share\glib-2.0\schemas
  
  
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}" "" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME_CAP}" "DisplayName" "${NAME_CAP} (remove only)"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME_CAP}" "UninstallString" '"$INSTDIR\bin\uninst.exe"'
  ; write out uninstaller
  WriteUninstaller "$INSTDIR\bin\uninst.exe"

  # Copy the install file
  StrCpy $0 "$INSTDIR\install-pomelo.log"
  Push $0
  Call DumpLog
SectionEnd ; end of default section

; begin uninstall settings/section
UninstallText "This will uninstall ${NAME_CAP} from your system"

Section Uninstall
  ; add delete commands to delete whatever files/registry keys/etc you installed here.
  ReadRegStr $0 HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}" ""
  DetailPrint "Deleting from $0"
  
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME_CAP}"
  SetOutPath "$TEMP"
  RMDir /r "$0"
  
  RMDir /r "$SMPROGRAMS\${NAME_CAP}"
SectionEnd ; end of uninstall section

; eof
