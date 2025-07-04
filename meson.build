project('pomelo','cpp','c',
        version : '0.0.9',
        default_options : ['cpp_std=c++2a'])

nanosvg_proj = subproject('nanosvg')
tinygltf_proj = subproject('tinygltf')
nlohmann_json_proj = subproject('nlohmann_json')
spdlog_proj = subproject('spdlog')
gob2_proj = subproject('gob2')
fmt_proj = subproject('fmt')
glm_proj = subproject('glm')

nanosvg_dep = nanosvg_proj.get_variable('nanosvg_dep')
tinygltf_dep = tinygltf_proj.get_variable('tinygltf_dep')
nhlohmann_json_dep = nlohmann_json_proj.get_variable('nlohmann_json_dep')
spdlog_dep = spdlog_proj.get_variable('spdlog_dep')
gob2 = gob2_proj.get_variable('gob2_exe')
fmt_dep = fmt_proj.get_variable('fmt_dep')
glm_dep = glm_proj.get_variable('glm_dep')

gnome = import('gnome')

name='pomelo'
name_cap = 'Pomelo'
version = '0.0.9'

r = run_command('git', 'rev-parse', 'HEAD', check: true)
if r.returncode() != 0
  # it failed
endif
sha1 = r.stdout().strip()
# meson version 0.56 will have substr()
sha1_short = sha1[0]+sha1[1]+sha1[2]+sha1[3]+sha1[4]+sha1[5]

r = run_command('git', 'log', '--pretty=%ci', '-n1', check: true)
if r.returncode() != 0
  # it failed
endif
commit_time=r.stdout().strip()

cpp_args = ['-DCOMMIT_ID="'+sha1+'"',
            '-DCOMMIT_TIME="'+commit_time+'"',
            '-DVERSION="'+version+'"',
            '-DSPDLOG_FMT_EXTERNAL',
            '-DFMT_HEADER_ONLY',
            '-flarge-source-files'
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
    '-DCOMMIT_TIME='+commit_time,
    '-DSHA1='+sha1,
    '-DNAME='+name,
    '-DNAME_CAP='+name_cap,
    '-DICON_NAME='+name + '_logo',
    '-DARCH='+arch,
    '-DSYSROOT=/usr/' + arch + '/sys-root/mingw',
    '-DHOST='+host,
    '-DVERSION='+'v'+ version,
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
