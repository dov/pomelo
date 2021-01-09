/**
 * @file   giv_data.cpp
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Thu Apr 14 11:48:38 2005
 * 
 * @brief  manipulate giv data
 * 
 * 
 */
#include "giv-data.h"
#include "plis/plis.h"

using namespace plis;

#define COLOR_NONE 0xfffe

static gboolean default_draw_lines = TRUE;
static gboolean default_draw_marks = FALSE;
static gboolean default_scale_marks = FALSE;
static gint default_mark_type = 1;
static gint default_render_type = -1;
static gdouble default_line_width = 0;
static gdouble default_mark_size = 7;

giv_dataset_t *new_giv_dataset(int num_datasets)
{
    giv_dataset_t *dataset_p = g_new0(giv_dataset_t, 1);
    GivColor color_of_black = { 0,0,0,0xffff};

    dataset_p->points = g_array_new (FALSE, FALSE, sizeof (point_t));
    dataset_p->do_draw_marks = FALSE;
    dataset_p->do_draw_lines = TRUE;
    dataset_p->do_draw_polygon = FALSE;
    dataset_p->quiver_head = TRUE;
    dataset_p->do_draw_polygon_outline = FALSE;
    dataset_p->do_scale_marks = default_scale_marks;
    dataset_p->has_quiver = FALSE;
    dataset_p->mark_type = default_mark_type;
    dataset_p->mark_size = default_mark_size;
    dataset_p->line_style = 0;
    dataset_p->line_width = 1;
    dataset_p->text_size = -1;
    dataset_p->do_scale_fonts = FALSE;
    dataset_p->do_pango_markup = FALSE;
    dataset_p->num_dashes = 0;
    dataset_p->dashes = NULL;
    dataset_p->set_name = NULL;
    dataset_p->balloon_string = NULL;
    dataset_p->font_name = NULL;
    dataset_p->file_name = NULL;
    dataset_p->color = color_of_black; 
    dataset_p->shadow_color = color_of_black;
    dataset_p->shadow_offset_x = 0.5; 
    dataset_p->shadow_offset_y = 0.5;
    dataset_p->text_style = TEXT_STYLE_NORMAL;
    dataset_p->outline_color = color_of_black;
    dataset_p->outline_color.alpha = 128*256;
    dataset_p->quiver_color = color_of_black;
    dataset_p->quiver_scale = 1.0;
#if 0
    if (prm_override_names)
        dataset_p->set_name = g_array_index (prm_override_names, char *, set_idx);
    if (!dataset_p->set_name)
        dataset_p->set_name = g_strdup (file_name);
#endif
    dataset_p->path_name = g_strdup_printf ("Dataset %d", num_datasets);
    dataset_p->tree_path_string = NULL;
    dataset_p->is_visible = TRUE;
    dataset_p->arrow_type = ARROW_TYPE_NONE;
    dataset_p->svg = NULL;

    return dataset_p;
}

void free_giv_data_set(giv_dataset_t *dataset_p)
{
    g_free(dataset_p->path_name);
    if (dataset_p->set_name)
        g_free(dataset_p->set_name);
    if (dataset_p->file_name)
        g_free(dataset_p->file_name);
    if (dataset_p->balloon_string)
        g_string_free(dataset_p->balloon_string, TRUE);
    if (dataset_p->font_name)
        g_free(dataset_p->font_name);
    for (int i=0; i<(int)dataset_p->points->len; i++)
        {
          point_t p = g_array_index(dataset_p->points, point_t, i);
          if (p.op == OP_TEXT) {
            g_free(p.text_object->string);
            g_free(p.text_object);
          }
          p.text_object=NULL;
        }
    g_array_free(dataset_p->points, TRUE);
    if (dataset_p->svg)
      delete dataset_p->svg;
    g_free(dataset_p);
}

