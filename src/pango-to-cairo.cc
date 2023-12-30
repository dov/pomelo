// Uses pango to turn a markup into cairo context
#include "pango-to-cairo.h"
#include <cairomm/cairomm.h>
#include <pangomm.h>
#include <pangomm/layout.h>
#include <pango/pangoft2.h>
#include <pango/pangocairo.h>
#include <fmt/core.h>

using namespace std;
using namespace Glib;

void pangomarkup_to_cairo(cairo_t *cr,       
                          const char *markup,
                          PangoFontDescription *desc)
{
  PangoLayout *layout = pango_cairo_create_layout(cr);
  PangoContext *context = pango_layout_get_context(layout);
  pango_context_set_language(context, pango_language_from_string("C"));
  pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
  pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER); // Make configurable!
  pango_layout_set_markup(layout, markup,-1);
  pango_layout_set_auto_dir(layout, true);

  pango_layout_set_font_description(layout, desc);

  cairo_translate(cr,0,0);
  //  cr->scale(0.1,0.1); // This scaling is done to prevent clipping in the
                      // cairo layout. The linear limit is changed in
                      // in correspondance.
  cairo_scale(cr, 2,2);
  cairo_move_to(cr,0,0);
  cairo_set_source_rgb(cr, 0,0,0);
  pango_cairo_update_layout(cr, layout);

  // Here we use low level pango functions to iterate over the runs
  // one at a time. This is currently not used but is a preparation
  // for the following two desired functionalities:
  //
  //  1. the direct converting of glyphs to polygons with holes.
  //  2. Caching of skeletons per glyph
  //
  // To do this properly we need to merge this routine with the
  // TeXtrusion::cairo_path_to_polygons() and
  // TeXtrusion::polys_to_polys_with_holes().

  cairo_save(cr);
  PangoLayoutIter *line_iter = pango_layout_get_iter(layout);
  float line_spacing = pango_layout_get_line_spacing(layout);
  
  fmt::print("line_count = {} line_spacing=line_spacing={}\n", pango_layout_get_line_count(layout), line_spacing);
  PangoRectangle layout_logical_rect;
  pango_layout_get_extents(layout,
                           NULL,
                           &layout_logical_rect);
  do {
    fmt::print("Layouting a line\n");
    PangoLayoutIter *run_iter = pango_layout_iter_copy(line_iter);
    int baseline = pango_layout_iter_get_baseline(line_iter);
    PangoRectangle line_logical_rect;
    pango_layout_iter_get_line_extents(line_iter,
                                       NULL,
                                       &line_logical_rect);

    cairo_move_to(cr, line_logical_rect.x/PANGO_SCALE, baseline/PANGO_SCALE);
    
    do {
      fmt::print("showing a glyph item\n");
      PangoGlyphItem *glyph_item = pango_layout_iter_get_run_readonly(run_iter);
      if (!glyph_item)
        break;
      
      // Add one logical cluster at a time. These should immediately
      // be turned into polygons with holes.
      for (int i=0; i<glyph_item->glyphs->num_glyphs; i++)
        {
          PangoGlyphInfo *glyph_info = glyph_item->glyphs->glyphs+i;
          PangoGlyphString gs = {1,
            glyph_info,
            glyph_item->glyphs->log_clusters + i};
          
          // Move to the position of the glyph
          cairo_rel_move_to(cr,
                            glyph_info->geometry.x_offset/PANGO_SCALE,
                            glyph_info->geometry.y_offset/PANGO_SCALE);
          pango_cairo_glyph_string_path(cr,
                                        glyph_item->item->analysis.font,
                                        &gs);
          // Forward to next glyph
          cairo_rel_move_to(cr,
                            glyph_info->geometry.width/PANGO_SCALE
                            -glyph_info->geometry.x_offset/PANGO_SCALE,
                            -glyph_info->geometry.y_offset/PANGO_SCALE);
        }
    } while(pango_layout_iter_next_run(run_iter));
    pango_layout_iter_free(run_iter);

  } while(pango_layout_iter_next_line(line_iter));

  pango_layout_iter_free(line_iter);

#if 0
  // The following much shorter version is long version above equivalent,
  // but I want to keep the long version as a preparation for optimizations
  // outlined.
  pango_cairo_layout_path(cr, layout);
#endif

  g_object_unref(layout);
}
