// Some convenience widgets for pomelo

#include "dov-mm-macros.h"
#include "pomelo-widget-utils.h"

Gtk::Widget* mmLabelAligned(const gchar *markup, double xAlign)
{
  auto w_label = mm<Gtk::Label>();
  w_label->set_markup(markup);
  w_label->set_alignment(xAlign,0.5);
  return w_label;
}

// Make a left aligned managed label with markup
Gtk::Widget* mmLabelLeft(const gchar *markup)
{
  return mmLabelAligned(markup, 0);
}

// Make a left aligned managed label with markup
Gtk::Widget* mmLabelRight(const gchar *markup)
{
  return mmLabelAligned(markup, 1);
}

Gtk::Frame* mmFrameWithBoldLabel(const gchar *label)
{
  auto w_frame = mm<Gtk::Frame>();
  auto w_frame_label = mm<Gtk::Label>();
  w_frame_label->set_markup(Glib::ustring("<b>") + Glib::ustring(label) + "</b>");
  w_frame->set_label_widget(*w_frame_label);
  return w_frame;
}

