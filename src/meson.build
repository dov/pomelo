engine_sources = ['textrusion.cc',
                  'smooth-sharp-angles.cc',
                  'svgpath-to-cairo.cc',
                  'pango-to-cairo.cc',
                  'profile.cc',
                  'bezier-intersect.cc',
                  'cairo-flatten-by-bitmap.cc',
                  'image-tracer-bw.cc',
                  'utils.cc',
                  'giv-debug-utils.cc',
                  'mesh.cc',
                  'mesh-utils.cc',
                 ]

sources = ['pomelo-main.cc',
           'pomelo.cc',
           'mesh-viewer.cc',
           'trackball.cc',
           'main-input.cc',
           'worker-skeleton.cc',
           'progress-dialog.cc',
           'skeleton-viewer.cc',
           'settings-dialog.cc',
           'pomelo-widget-utils.cc',
           'pomelo-settings.cc',
           'profile-editor.cc',
           'profile-editor-window.cc',
          ]

link_args = []
cpp_args += ['-DBOOST_ALLOW_DEPRECATED_HEADERS']

if host_machine.system() == 'windows' 
  cpp_args += ['-Wa,-mbig-obj',
               '-Wno-attributes',
               '-DBOOST_BIND_GLOBAL_PLACEHOLDERS',
              ]
  if get_option('optimization')=='0'
    cpp_args += ['-g','-O1']
  endif
  link_args += ['-Wa,-mbig-obj']
  mod_rescomp = import('windows')
  sources += mod_rescomp.compile_resources('pomelo.rc')
else

endif

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
pthread_dep = cc.find_library('pthread', required : false)
glibmm_dep = dependency('glibmm-2.4')
pangomm_dep = dependency('pangomm-1.4')
gtkmm_dep = dependency('gtkmm-3.0')
pangoft_dep = dependency('pangoft2')
cairo_dep = dependency('cairo')
epoxy_dep = dependency('epoxy')
fmt_dep = dependency('fmt')
glm_dep = dependency('glm')
gmp_dep = dependency('gmp')
goo_dep = dependency('goocanvasmm-2.0')
mpfr_dep = dependency('mpfr')

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
        nanosvg_dep,
        tinygltf_dep,
        nhlohmann_json_dep,
        goo_dep,
        spdlog_dep]

if host_machine.system() == 'windows'
  lib_engine = shared_library('engine',
                              engine_sources,
                              dependencies : deps,
                              install :true,
                              cpp_args : cpp_args,
                              link_args : link_args
                             )
else
  lib_engine = static_library('engine',
                              engine_sources,
                              dependencies : deps,
                              cpp_args : cpp_args,
                              link_args : link_args
                             )
endif

incdir = ['giv-widget']

exe = executable(name,
                 sources,
                 resources,
                 dependencies: deps,
                 win_subsystem : 'windows',  # true to hide console
                 cpp_args : cpp_args,
                 link_args : link_args,
                 include_directories : incdir,
                 link_with : [libgivwidget,
                              libgtkimageviewer,
                              lib_engine
                              ],
                 install :true
                )


executable('pomelo-cli',
           ['pomelo-cli.cc'],
           link_with : [lib_engine],
           cpp_args : cpp_args,
           dependencies : [pangoft_dep,
                           fmt_dep,
                           glm_dep,
                           gmp_dep,
                           mpfr_dep,
                           m_dep,
                           nanosvg_dep,
                           tinygltf_dep,
                           nhlohmann_json_dep,
                           spdlog_dep,
                           glibmm_dep,
                           pangomm_dep],
           install : true)

# An example for converting an svg path to cairo
executable('test-svgpath-to-cairo',
           ['test-svgpath-to-cairo.cc',
           ],
           cpp_args : cpp_args,
           link_with : [lib_engine],
           dependencies : deps)

# An example for converting an svg path to cairo
executable('test-svg-to-skeleton',
           ['test-svg-to-skeleton.cc'],
           cpp_args : cpp_args,
           link_with : [lib_engine],
           dependencies : deps)

executable('test-pango-to-cairo',
           ['test-pango-to-cairo.cc'],
           cpp_args : cpp_args,
           link_with : [lib_engine],
           dependencies : deps)

executable('test-profile-editor',
           ['test-profile-editor.cc',
            'profile-editor.cc',
            'profile.cc',
            'bezier-intersect.cc',
           ],
           resources,
           link_with : [lib_engine],
           cpp_args : cpp_args,
           dependencies : deps)

executable('test-merge-by-distance',
           ['test-merge-by-distance.cc',
           ],
           resources,
           link_with : [lib_engine],
           cpp_args : cpp_args,
           dependencies : deps)

#executable('test-profile-editor-window',
#           ['test-profile-editor-window.cc',
#            'profile-editor-window.cc',
#            'profile-editor.cc',
#            'profile.cc',
#            'bezier-intersect.cc',
#           ],
#           resources,
#           link_with : [lib_engine],
#           cpp_args : cpp_args,
#           dependencies : deps)


