gob2 = find_program('gob2')
patch_src = find_program('patch_src.py')
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

gen_src = custom_target('gen-output',
  output : ['gtk-image-viewer.h','gtk-image-viewer-private.h','gtk-image-viewer.c'],
  input : 'gtk-image-viewer.gob',
  command : [gob2, '-o', '@OUTDIR@', '@INPUT@'],
  )

patch_src = custom_target('patch-output',
  output : ['gtk-image-viewer-fixed.c'],
  input  : gen_src[2],
  command : [patch_src, '@INPUT@','@OUTPUT@'],
)

gtkimageviewer_sources = [ patch_src ]

gtk_dep = dependency('gtk+-3.0')
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

libgtkimageviewer = library(
  'gtk_image_viewer',
  sources: gtkimageviewer_sources,
  install: false,
  dependencies: [gtk_dep,
                m_dep],
)

