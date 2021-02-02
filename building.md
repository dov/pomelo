# Cross compilation for windows

## Windows

The windows installer is build by meson cross compilation. The compilation is done as follows:

      meson . build_mingw64 --cross-file cross_mingw64.txt -Ddefault_library=shared -Dbuildtype=release
      ninja -C build_mingw64

To build the nsis installer do:

      ninja -C build_mingw64 installer
      
## Linux

In order to avoid conflict with system libraries use the following command to create the dependent libraries as static.

      meson . build -Ddefault_library=static -Dbuildtype=release
      ninja -C build
