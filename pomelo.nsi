Name "${NAME}"
OutFile "${OUTDIR}\Install${NAME_CAP}-${HOST}-${VERSION}-${COMMITID_SHORT}.exe"

SetCompress force ; (can be off or force)
CRCCheck on ; (can be off)

LicenseText "Please read and accept the following license:"
LicenseData "COPYING"

InstallDir "$PROGRAMFILES64\${NAME_CAP}"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}" ""

; The text to prompt the user to enter a directory
DirText "Choose a folder in which to install ${NAME_CAP}"

; Show details
ShowInstDetails show

Icon ${ICON_DIR}/${ICON_NAME}.ico
UninstallIcon ${ICON_DIR}/${ICON_NAME}.ico
XPStyle on

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
File \usr\${ARCH}\sys-root\mingw\bin\libintl-8.dll
File \usr\${ARCH}\sys-root\mingw\bin\iconv.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpcre-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgtk-3-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgdk-3-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libssp-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgdk-win32-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgdk_pixbuf-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgtk-win32-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpixman-1-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libffi-6.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgio-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libcairo-2.dll
File \usr\${ARCH}\sys-root\mingw\bin\libcairo-gobject-2.dll
File \usr\${ARCH}\sys-root\mingw\bin\zlib1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libglib-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libatk-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgobject-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgmodule-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgthread-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpango-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libfribidi-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpangocairo-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpangoft2-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpangowin32-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpng16-16.dll
File \usr\${ARCH}\sys-root\mingw\bin\libtiff-5.dll
File \usr\${ARCH}\sys-root\mingw\bin\libjpeg-62.dll
File \usr\${ARCH}\sys-root\mingw\bin\libfontconfig-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libxml2-2.dll
File \usr\${ARCH}\sys-root\mingw\bin\libfreetype-6.dll
File \usr\${ARCH}\sys-root\mingw\bin\gdk-pixbuf-query-loaders.exe
File \usr\${ARCH}\sys-root\mingw\bin\${LIBGCCDLL}
File \usr\${ARCH}\sys-root\mingw\bin\libstdc++-6.dll
File \usr\${ARCH}\sys-root\mingw\bin\libwinpthread-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgfortran-*.dll
File \usr\${ARCH}\sys-root\mingw\bin\librsvg-2-2.dll
File \usr\${ARCH}\sys-root\mingw\bin\libcroco-0.6-3.dll
File \usr\${ARCH}\sys-root\mingw\bin\libxml2-2.dll
File \usr\${ARCH}\sys-root\mingw\bin\libquadmath-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libopenjp2.dll
File \usr\${ARCH}\sys-root\mingw\bin\libsqlite3-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libexpat-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libbz2-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgdkglext-win32-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgtkglext-win32-1.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\glew32.dll
File \usr\${ARCH}\sys-root\mingw\bin\libzip-5.dll
File \usr\${ARCH}\sys-root\mingw\bin\libharfbuzz-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libharfbuzz-icu-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libharfbuzz-subset-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\gspawn-win64-helper.exe
File \usr\${ARCH}\sys-root\mingw\bin\libglibmm-2.4-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libatkmm-1.6-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libpangomm-1.4-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libcairomm-1.0-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgtkmm-3.0-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgdkmm-3.0-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\libgiomm-2.4-1.dll
File \usr\${ARCH}\sys-root\mingw\bin\gdbus.exe
File \usr\${ARCH}\sys-root\mingw\bin\libsigc-2.0-0.dll
File \usr\${ARCH}\sys-root\mingw\bin\libmpfr-6.dll
File \usr\${ARCH}\sys-root\mingw\bin\libepoxy-0.dll
File \usr\local\mingw64\bin\libfmt.dll
File \usr\local\mingw64\bin\libgmp-10.dll
File ${OUTDIR}\src\${NAME}.exe
File ${OUTDIR}\src\libengine.dll
File ${OUTDIR}\src\giv-widget\libgiv-widget.dll
File ${OUTDIR}\src\giv-widget\gtk-image-viewer\libgtk_image_viewer.dll
File ${OUTDIR}\src\giv-widget\agg\libagg.dll
File ${OUTDIR}\src\giv-widget\plis\libplis.dll


SetOutPath "$INSTDIR"
File /r \usr\${ARCH}\sys-root\mingw\etc
SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0\loaders
File \usr\${ARCH}\sys-root\mingw\lib\gdk-pixbuf-2.0\2.10.0\loaders\*
SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0
File \usr\${ARCH}\sys-root\mingw\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache
SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\engines
File \usr\${ARCH}\sys-root\mingw\lib\gtk-2.0\2.10.0\engines\*
SetOutPath $INSTDIR\share
File /r \usr\${ARCH}\sys-root\mingw\share\themes
SetOutPath $INSTDIR\share
File /r \usr\${ARCH}\sys-root\mingw\share\icons
SetOutPath $INSTDIR\share\glib-2.0
File /r \usr\${ARCH}\sys-root\mingw\share\glib-2.0\schemas


WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\${NAME_CAP}" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME_CAP}" "DisplayName" "${NAME_CAP} (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME_CAP}" "UninstallString" '"$INSTDIR\bin\uninst.exe"'
; write out uninstaller
WriteUninstaller "$INSTDIR\bin\uninst.exe"

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
