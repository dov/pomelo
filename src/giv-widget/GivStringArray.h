#ifndef GIVSTRINGARRAY_H
#define GIVSTRINGARRAY_H

// A convience array of strings
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

void giv_string_array_free(GPtrArray *string_array);

/** 
 * Search for a string in a string array. If found return the index
 * otherwise return -1.
 * 
 * @param string_array 
 * @param string 
 * 
 * @return 
 */
int giv_string_array_find(GPtrArray* string_array,
                          const char *string);
void giv_string_array_replace(GPtrArray* string_array,
                              int pos,
                              char *s);

#ifdef __cplusplus
}
#endif

#endif /* GIVSTRINGARRAY */
