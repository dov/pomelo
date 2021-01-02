project('gtkmm_example','cpp','c')

sources = ['src/pomelo-main.cc',
           'src/pomelo.cc',
           'src/mesh-viewer.cc',
           'src/trackball.cc',
           'src/main-input.cc',
           'src/mesh.cc',
           'src/pangocairo-to-contour.cc',
           'src/worker-skeleton.cc',
           'src/progress-dialog.cc'
          ]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
pthread_dep = cc.find_library('pthread', required : false)
gtkmmdep = dependency('gtkmm-3.0')
pangoftdep = dependency('pangoft2')
epoxydep = dependency('epoxy')
fmtdep = dependency('fmt')
glmdep = dependency('glm')
gmpdep = dependency('gmp')
mpfrdep = dependency('mpfr')
deps = [gtkmmdep,
        pangoftdep,
        epoxydep,
        fmtdep,
        glmdep,
        gmpdep,
        mpfrdep,
        m_dep,
        pthread_dep]

gnome = import('gnome')

resources = gnome.compile_resources(
  'resources',
  'src/resources.xml',
  c_name :'resources',
  source_dir : 'src'
)

name='pomelo'
name_cap = 'Pomelo'
version = '0.1.0'

exe = executable(name_cap,
                 sources,
                 resources,
                 dependencies: deps,
                 gui_app : true
                )

r = run_command('git', 'rev-parse', 'HEAD')
if r.returncode() != 0
  # it failed
endif

sha1 = r.stdout().strip()
# meson version 0.56 will have substr()
sha1_short = sha1[0]+sha1[1]+sha1[2]+sha1[3]+sha1[4]+sha1[5]

fs = import('fs')
builddir = fs.name(meson.current_build_dir())

if host_machine.system() == 'windows'
  arch = 'x86_64-w64-mingw32'
  host = 'w64'

  nsis_cmd = [
    find_program('makensis'),
    '-DOUTDIR='+builddir,
    '-DCOMMITID_SHORT='+sha1_short,
    '-DNAME='+name,
    '-DNAME_CAP='+name_cap,
    '-DICON_NAME='+name,
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
