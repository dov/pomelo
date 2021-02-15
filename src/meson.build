engine_sources = ['textrusion.cc',
                  'smooth-sharp-angles.cc',
                  'svgpath-to-cairo.cc',
                  'pango-to-cairo.cc']

sources = ['pomelo-main.cc',
           'pomelo.cc',
           'mesh-viewer.cc',
           'trackball.cc',
           'main-input.cc',
           'mesh.cc',
           'worker-skeleton.cc',
           'progress-dialog.cc',
           'skeleton-viewer.cc',
           'settings-dialog.cc',
           'pomelo-widget-utils.cc',
           'pomelo-settings.cc'
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
gtkmm_dep = dependency('gtkmm-3.0')
pangoft_dep = dependency('pangoft2')
cairo_dep = dependency('cairo')
epoxy_dep = dependency('epoxy')
fmt_dep = dependency('fmt')
glm_dep = dependency('glm')
gmp_dep = dependency('gmp')
mpfr_dep = dependency('mpfr')
nanosvg_dep = dependency('nanosvg')

subdir('giv-widget')

resources = gnome.compile_resources(
  'resources',
  'resources.xml',
  c_name :'resources',
  source_dir : ['.',
                '..',
                '../images',]
)

deps = [gtkmm_dep,
        pangoft_dep,
        epoxy_dep,
        fmt_dep,
        glm_dep,
        gmp_dep,
        mpfr_dep,
        m_dep,
        pthread_dep,
        nanosvg_dep]

if host_machine.system() == 'windows'
  lib_engine = shared_library('engine',
                              engine_sources,
                              dependencies : deps,
                              install :true
                             )
else
  lib_engine = static_library('engine',
                              engine_sources,
                              dependencies : deps)
endif

incdir = ['giv-widget']
exe = executable(name_cap,
                 sources,
                 resources,
                 dependencies: deps,
                 gui_app : true,
                 cpp_args : cpp_args,
                 include_directories : incdir,
                 link_with : [libgivwidget,
                              libgtkimageviewer,
                              lib_engine
                              ],
                 install :true
                )


# An example for converting an svg path to cairo
executable('test-svgpath-to-cairo',
           ['test-svgpath-to-cairo.cc'],
           link_with : [lib_engine],
           dependencies : deps)

# An example for converting an svg path to cairo
executable('test-svg-to-skeleton',
           ['test-svg-to-skeleton.cc'],
           link_with : [lib_engine],
           dependencies : deps)

executable('test-pango-to-cairo',
           ['test-pango-to-cairo.cc'],
           link_with : [lib_engine],
           dependencies : deps)

