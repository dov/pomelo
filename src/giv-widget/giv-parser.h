//======================================================================
//  GivParser.h - A giv parser class
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Nov  4 22:52:41 2007
//----------------------------------------------------------------------

#ifndef GIV_PARSER_H
#define GIV_PARSER_H

#include <glib.h>
#include "giv-data.h"

typedef enum {
    GIV_PARSER_ORIENTATION_UNDEF,
    GIV_PARSER_ORIENTATION_UNFLIP,
    GIV_PARSER_ORIENTATION_FLIP
} giv_parser_orientation_t;

typedef void (*giv_parser_file_reference_t)(const char *filename, gpointer data);
typedef void (*giv_parser_set_orientation_t)(giv_parser_orientation_t horint,
                                             giv_parser_orientation_t vorint,
                                             gpointer data);

typedef void (*giv_parser_set_vlock_t)(gboolean vlock, gpointer data);

typedef struct {
    GPtrArray* giv_datasets;
    GHashTable *style_hash;
    giv_parser_file_reference_t cb_file_reference;
    gpointer cb_file_reference_data;
    giv_parser_set_orientation_t cb_set_orientation;
    gpointer cb_set_orientation_data;
    giv_parser_set_vlock_t cb_set_vlock;
    gpointer cb_set_vlock_data;

    // Bounding box of data
    double global_mark_max_x;
    double global_mark_max_y;
    double global_mark_min_x;
    double global_mark_min_y;

    double quiver_scale;
}  GivParser;

GivParser *giv_parser_new();
void giv_parser_free(GivParser *giv_parser);
void giv_parser_set_reference_callback(GivParser *giv_parser,
                                       giv_parser_file_reference_t fr,
                                       gpointer user_data);
void giv_parser_set_orientation_callback(GivParser *giv_parser,
                                         giv_parser_set_orientation_t fr,
                                         gpointer user_data);
void giv_parser_set_vlock_callback(GivParser *giv_parser,
                                   giv_parser_set_vlock_t vlock,
                                   gpointer user_data);
int giv_parser_parse_file(GivParser *giv_parser,
                          const char *filename);
int giv_parser_parse_string(GivParser *giv_parser,
                            const char *giv_string);
int giv_parser_add_svgfile(GivParser *giv_parser,
                            const char *filename);

void giv_parser_giv_style_add_string(GivParser *giv_parser,
                                     const char* style_name,
                                     const char* style_string);
void giv_parser_giv_set_props_from_style(GivParser *giv_parser,
                                         giv_dataset_t *marks,
                                         const char *style_name);
void giv_parser_clear(GivParser *giv_parser);
void giv_parser_get_data_bbox(GivParser *giv_parser,
                              // output
                              double* min_x, double* min_y,
                              double* max_x, double* max_y);
void giv_parser_set_quiver_scale(GivParser *giv_parser,
                                 double quiver_scale);
double giv_parser_get_quiver_scale(GivParser *giv_parser);
int giv_parser_giv_marks_data_add_line(GivParser *giv_parser,
                                       giv_dataset_t *marks,
                                       const char *line,
                                       const char *filename,
                                       int linenum);
GPtrArray* giv_parser_get_giv_datasets(GivParser *giv_parser);
int giv_parser_count_marks(GivParser *giv_parser,
                           double x0, double y0,
                           double x1, double y1);

#endif
