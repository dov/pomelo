//======================================================================
//  pomelo-widget-utils.h - Some convenience widgets
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 21:22:42 2021
//----------------------------------------------------------------------
#ifndef POMELO_WIDGET_UTILS_H
#define POMELO_WIDGET_UTILS_H

#include <gtkmm.h>

Gtk::Widget* mmLabelAligned(const gchar *markup, double xAlign);

// Make a left aligned managed label with markup
Gtk::Widget* mmLabelLeft(const gchar *markup);

// Make a left aligned managed label with markup
Gtk::Widget* mmLabelRight(const gchar *markup);

// Make a frame awith a bold label
Gtk::Frame* mmFrameWithBoldLabel(const gchar* label);

#endif /* POMELO-WIDGET-UTILS */
