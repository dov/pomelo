project('gtkmm_example','cpp','c')

gnome = import('gnome')

name='pomelo'
name_cap = 'Pomelo'
version = '0.0.2'

r = run_command('git', 'rev-parse', 'HEAD')
if r.returncode() != 0
  # it failed
endif
sha1 = r.stdout().strip()
# meson version 0.56 will have substr()
sha1_short = sha1[0]+sha1[1]+sha1[2]+sha1[3]+sha1[4]+sha1[5]

r = run_command('git', 'log', '--pretty=%ci', '-n1')
if r.returncode() != 0
  # it failed
endif
commit_time=r.stdout().strip()

cpp_args = ['-DCOMMIT_ID="'+sha1+'"',
            '-DCOMMIT_TIME="'+commit_time+'"'
           ]

subdir('src')

fs = import('fs')
builddir = fs.name(meson.current_build_dir())
buildroot = fs.name(meson.current_source_dir())
if host_machine.system() == 'windows'
  arch = 'x86_64-w64-mingw32'
  host = 'w64'

  nsis_cmd = [
    find_program('makensis'),
    '-DOUTDIR='+builddir,
    '-DICON_DIR=.',
    '-DCOMMITID_SHORT='+sha1_short,
    '-DNAME='+name,
    '-DNAME_CAP='+name_cap,
    '-DICON_NAME='+name + '_logo',
    '-DARCH='+arch,
    '-DHOST='+host,
    '-DVERSION='+version,
    '-DLIBGCCDLL=libgcc_s_seh-1.dll',
    '@INPUT@',
  ]

  nsis = custom_target('nsis',
                       output: 'install-pomelo.exe',
                       input: files('pomelo.nsi'),
                       build_always_stale: true,
                       command: nsis_cmd,
                       depends: [exe]
                       )
  alias_target('installer', nsis)
endif
