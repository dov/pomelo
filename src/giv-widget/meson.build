gob2 = find_program('gob2')

giv_widget_src = custom_target('gen-output',
  output : ['giv-widget.h','giv-widget-private.h','giv-widget.cc'],
  input : 'giv-widget.gob',
  command : [gob2, '-o', '@OUTDIR@', '--for-cpp', '@INPUT@'],
  )

giv_widget_sources = [
  giv_widget_src[2],
  'colormaps.cc',
  'WordBoundaries.cc',
  'giv-parser.cc',
  'GivPainterAgg.cc',
  'GivPainterCairo.cc',
  'GivPainterCairoPixbuf.cc',
  'GivRenderer.cc',
  'GivStringArray.c',
  'giv-data.cc',
  'giv-markup.cc',
  'giv_agg_arrowhead.cc',
  'clipper.cpp'
]

subdir('agg')
subdir('plis')
subdir('gtk-image-viewer')

gtk_dep = dependency('gtk+-3.0')
incagg = include_directories('agg')
incgtkimageviewer = include_directories('agg')

libgivwidget = library(
  'giv-widget',
  giv_widget_sources,
  include_directories : [
    incagg,
    incgtkimageviewer
  ],
  link_with : [
    libagg,
    libplis,
    libgtkimageviewer
  ],
  dependencies : [gtk_dep],
  install : false)

