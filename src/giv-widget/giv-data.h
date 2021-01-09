/**
 * @file   giv_data.h
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Thu Apr 14 11:45:26 2005
 * 
 * @brief  
 * 
 * 
 */
#ifndef GIV_DATA_H
#define GIV_DATA_H

#include <gtk/gtk.h>
#include "agg/agg_svg_path_renderer.h"

enum
{
  OP_MOVE = 0,
  OP_DRAW = 1,
  OP_TEXT = 2,
  OP_QUIVER = 3,
  OP_ELLIPSE = 4,
  OP_CONT = 5,           // Misc continue operation for ops with more than two cordinates
  OP_CLOSE_PATH = 6
};

/* Mark types */
enum GivMarkType
{
  MARK_TYPE_FCIRCLE = 1,
  MARK_TYPE_FSQUARE = 2,
  MARK_TYPE_CIRCLE = 3,
  MARK_TYPE_SQUARE = 4,
  MARK_TYPE_PIXEL = 5,
};

enum GivArrowType
{
    ARROW_TYPE_NONE = 0,
    ARROW_TYPE_START = 1,
    ARROW_TYPE_END = 2,
    ARROW_TYPE_BOTH = 3,
};

enum GivTextStyle
{
  TEXT_STYLE_NORMAL,
  TEXT_STYLE_DROP_SHADOW
};

typedef struct {
    guint16 red;
    guint16 green;
    guint16 blue;
    guint16 alpha;
} GivColor;
    
typedef struct 
{
    GivColor color;
    GivColor outline_color;
    GivColor quiver_color;
    GivColor shadow_color;
    gdouble line_width;
    gdouble quiver_scale;
    gint line_style;
    gint mark_type;
    gdouble text_size;
    gboolean do_scale_fonts;
    gboolean do_pango_markup;
    enum GivTextStyle text_style;
    gint num_dashes;
    gdouble mark_size;
    gboolean do_scale_marks;
    gboolean do_draw_marks;
    gboolean do_draw_lines;
    gboolean do_draw_polygon;
    gboolean do_draw_polygon_outline;
    gboolean quiver_head;
    gboolean has_quiver;
    GArray *points;
    gchar *path_name;
    gchar *file_name;
    gchar *tree_path_string;
    gboolean is_visible;
    enum GivArrowType arrow_type;
    char *set_name;
    GString *balloon_string;
    char *font_name;
    gdouble *dashes;
    gdouble shadow_offset_x;
    gdouble shadow_offset_y;
    agg::svg::path_renderer *svg = NULL;  // svg path if data has svg
    agg::svg::path_renderer *svg_mark = NULL;  // marks drawn by svg
} giv_dataset_t;

typedef struct
{
    char *string;
    double size;
    int text_align; // A number between 1 and 9 like the numeric keypad. Default is 1.
} text_mark_t;

typedef struct
{
  gint op;
  gdouble x, y;
  text_mark_t *text_object;
} point_t;

giv_dataset_t *new_giv_dataset(int num_datasets);

void
free_giv_data_set(giv_dataset_t *dataset_p);

#endif /* GIV_DATA */
