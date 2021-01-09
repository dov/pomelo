#ifndef GIV_MARKUP_H
#define GIV_MARKUP_H

#include <gtk/gtk.h>

void
giv_text_buffer_insert_markup (GtkTextBuffer *buffer,
                               GtkTextIter   *textiter,
                               const gchar   *markup);

#endif /* GIV-MARKUP */
