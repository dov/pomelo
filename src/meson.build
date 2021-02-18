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
cgal_core_dep = cc.find_library('CGAL_Core', required : false)
cgal_dep = cc.find_library('CGAL', required : false)
pthread_dep = cc.find_library('pthread', required : false)
gtkmm_dep = dependency('gtkmm-3.0')
pangoft_dep = dependency('pangoft2')
cairo_dep = dependency('cairo')
epoxy_dep = dependency('epoxy')
fmt_dep = dependency('fmt')
glm_dep = dependency('glm')
cgal_dep = dependency('CGAL')
boost_dep = dependency('boost')
boost_thread_dep = cc.find_library('boost_thread')

nanosvg_dep = dependency('nanosvg', fallback : ['nanosvg','nanosvg_dep'])

# Dependencies for old fedora
gmp_dep = cc.find_library('gmp')
mpfr_dep = cc.find_library('mpfr')

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
        cgal_core_dep,
        cgal_dep,
        boost_dep,
        boost_thread_dep,
        pangoft_dep,
        epoxy_dep,
        fmt_dep,
        glm_dep,
        gmp_dep,
        mpfr_dep,
        m_dep,
        pthread_dep,
        nanosvg_dep,
        tinygltf_dep]

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

