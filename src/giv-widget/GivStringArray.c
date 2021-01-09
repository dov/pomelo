//======================================================================
//  EggStringArray.c - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jul 10 07:57:05 2009
//----------------------------------------------------------------------

#include "GivStringArray.h"

void giv_string_array_free(GPtrArray *string_array)
{
    g_ptr_array_foreach(string_array,
                        (GFunc)g_free,
                        NULL);
    g_ptr_array_free(string_array, TRUE);
}

int giv_string_array_find(GPtrArray* string_array,
                          const char *pattern)
{
    int i=0;
    for (i =0; i<string_array->len; i++) {
        if (g_regex_match_simple(pattern,
                                 (gchar*)g_ptr_array_index(string_array, i),
                                 G_REGEX_CASELESS,
                                 (GRegexMatchFlags)0)) {
            return i;
        }
    }
    return -1;
}

void giv_string_array_replace(GPtrArray* string_array,
                             int pos,
                             char *s)
{
    // Use the fact that remove_fast puts the last element on the
    // i'th position.
    g_ptr_array_add(string_array, s);
    g_free(g_ptr_array_index(string_array, pos));
    g_ptr_array_remove_index_fast(string_array, pos);
}
