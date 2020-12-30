#include "main-input.h"
#include "dov-mm-macros.h"

static Gtk::Widget* mmLabelAligned(const gchar *markup, double xAlign)
{
  auto w_label = mm<Gtk::Label>();
  w_label->set_markup(markup);
  w_label->set_alignment(xAlign,0.5);
  return w_label;
}

// Make a left aligned managed label with markup
static Gtk::Widget* mmLabelLeft(const gchar *markup)
{
  return mmLabelAligned(markup, 0);
}

// Make a left aligned managed label with markup
static Gtk::Widget* mmLabelRight(const gchar *markup)
{
  return mmLabelAligned(markup, 1);
}

MainInput::MainInput()
  : Gtk::Frame("Input")
{
  auto w_vbox = mmVBox; // Main window vbox
  auto w_grid = mm<Gtk::Grid>();
  m_text.set_hexpand(true);

  // Setup the radius button
  m_radius.set_digits(1);
  m_radius.set_range(0,1000);
  m_radius.set_increments(0.1,1);
  m_radius.set_value(5);

  int row=0;
  w_grid->attach(*mmLabelRight("Text:"), 0,row);
  w_grid->attach(m_text,                 1,row,2);
  row++;
  w_grid->attach(*mmLabelRight("Font:"), 0,row);
  w_grid->attach(m_fontPicker,           1,row); // TBD improve the widget
  row++;
  w_grid->attach(*mmLabelRight("Radius:"), 0,row);
  w_grid->attach(m_radius,               1,row);
  row++;

  w_vbox->pack_start(*w_grid, Gtk::PACK_SHRINK);

  this->add(*w_vbox);
}
