# Cross compilation for windows

## Windows

The windows installer is build by meson cross compilation. The compilation is done as follows:

      meson setup build_mingw64 --cross-file cross_mingw64.txt -Ddefault_library=shared -Dbuildtype=release
      ninja -C build_mingw64

To build the nsis installer do:

      ninja -C build_mingw64 installer
      
### Debug build

To build in debug mode do:

      meson setup build_mingw64_debug --cross-file cross_mingw64.txt -Ddefault_library=shared -Dbuildtype=debug
      ninja -C build_mingw64_debug

## Linux

The package goocanvasmm is not provided by default and needs to be loaded from source and may e.g. be installed in `/usr/local/lib`.

In order to avoid conflict with system libraries use the following command to create the dependent libraries as static.

      env PKG_CONFIG_PATH=/usr/local/lib/pkgconfig meson . build_release -Ddefault_library=static -Dbuildtype=release
      meson compile -C build_release
      meson install -C build_release --skip-subprojects
