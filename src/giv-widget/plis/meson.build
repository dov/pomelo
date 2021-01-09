plis_sources = [
'plis.cc'
]

pcre_dep = dependency('libpcre')
libplis = library('plis',
                  plis_sources,
                  dependencies : [pcre_dep],
                  install : false)
