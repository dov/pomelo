agg_sources = [
'agg_arc.cpp',
'agg_arrowhead.cpp',
'agg_bezier_arc.cpp',
'agg_bspline.cpp',
'agg_curves.cpp',
'agg_embedded_raster_fonts.cpp',
'agg_gsv_text.cpp',
'agg_image_filters.cpp',
'agg_line_aa_basics.cpp',
'agg_line_profile_aa.cpp',
'agg_rounded_rect.cpp',
'agg_sqrt_tables.cpp',
'agg_svg_gradient.cpp',
'agg_svg_parser.cpp',
'agg_svg_path_renderer.cpp',
'agg_svg_path_tokenizer.cpp',
'agg_trans_affine.cpp',
'agg_trans_double_path.cpp',
'agg_trans_single_path.cpp',
'agg_trans_warp_magnifier.cpp',
'agg_vcgen_bspline.cpp',
'agg_vcgen_contour.cpp',
'agg_vcgen_dash.cpp',
'agg_vcgen_markers_term.cpp',
'agg_vcgen_smooth_poly1.cpp',
'agg_vcgen_stroke.cpp',
'agg_vpgen_clip_polygon.cpp',
'agg_vpgen_clip_polyline.cpp',
'agg_vpgen_segmentator.cpp'
]

expat_dep = dependency('expat')

libagg = library('agg',
                 agg_sources,
                 dependencies : expat_dep,
                 install : false)
