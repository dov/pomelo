sources = ['pomelo-main.cc',
           'pomelo.cc',
           'mesh-viewer.cc',
           'trackball.cc',
           'main-input.cc',
           'mesh.cc',
           'pangocairo-to-contour.cc',
           'worker-skeleton.cc',
           'progress-dialog.cc',
           'skeleton-viewer.cc',
          ]

if host_machine.system() == 'windows'
  cpp_args += ['-Wa,-mbig-obj']
  mod_rescomp = import('windows')
  sources += mod_rescomp.compile_resources('pomelo.rc')
else

endif

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


subdir('giv-widget')

resources = gnome.compile_resources(
  'resources',
  'resources.xml',
  c_name :'resources',
  source_dir : ['.',
                '..',
                '../images',]
)
incdir = ['giv-widget']
exe = executable(name_cap,
                 sources,
                 resources,
                 dependencies: deps,
                 gui_app : true,
                 cpp_args : cpp_args,
                 include_directories : incdir,
                 link_with : [libgivwidget,
                              libgtkimageviewer],
                 install :true
                )


