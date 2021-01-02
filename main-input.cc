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

static Gtk::Frame* mmFrameWithBoldLabel(Glib::ustring label)
{
  auto w_frame = mm<Gtk::Frame>();
  auto w_frame_label = mm<Gtk::Label>();
  w_frame_label->set_markup(Glib::ustring("<b>") + label + "</b>");
  w_frame->set_label_widget(*w_frame_label);
  return w_frame;
}

MainInput::MainInput()
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
  auto w_frame = mmFrameWithBoldLabel("Skeleton");
  this->pack_start(*w_frame, true,true);
  auto w_vbox = mmVBox; // Main window vbox
  auto w_grid = mm<Gtk::Grid>();

  // Configure the widgets
  m_text.set_hexpand(true);
  m_text_buffer = m_text.get_buffer();
  m_text_buffer->set_text("Pomelo");
  m_text_buffer->signal_changed().connect( sigc::mem_fun(*this,
    &MainInput::on_skeleton_input_changed));

  // Setup the linear limit button
  m_linear_limit.set_digits(3);
  m_linear_limit.set_range(0.001,1000);
  m_linear_limit.set_increments(0.01,1);
  m_linear_limit.set_value(5.0);
  m_linear_limit.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_skeleton_input_changed));

  m_font_picker.set_show_size(false);
  m_font_picker.set_font_name("Sans Bold 48");
  m_font_picker.signal_font_set().connect(sigc::mem_fun(*this,
    &MainInput::on_skeleton_input_changed));

  // Setup the radius button
  m_radius.set_digits(1);
  m_radius.set_range(0,1000);
  m_radius.set_increments(0.1,1);
  m_radius.set_value(5);
  m_radius.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_profile_input_changed));

  // Setup the radius num steps
  m_num_radius_steps.set_digits(0);
  m_num_radius_steps.set_range(2,50);
  m_num_radius_steps.set_increments(1,10);
  m_num_radius_steps.set_value(10);
  m_num_radius_steps.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_profile_input_changed));

  // Setup the radius button
  m_zdepth.set_digits(1);
  m_zdepth.set_range(0,1000);
  m_zdepth.set_increments(0.1,1);
  m_zdepth.set_value(5);
  m_zdepth.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_profile_input_changed));

  // Upper frame
  int row=0;
  w_grid->attach(*mmLabelRight("Text:"), 0,row);
  w_grid->attach(m_text,                 1,row,2);
  row++;
  w_grid->attach(*mmLabelRight("Linear limit:"), 0,row);
  w_grid->attach(m_linear_limit, 1,row,2);
  row++;
  w_grid->attach(*mmLabelRight("Font:"), 0,row);
  w_grid->attach(m_font_picker,          1,row); 
  row++;
  w_frame->add(*w_vbox);
  w_vbox->pack_start(*w_grid, Gtk::PACK_SHRINK);

  // Button box
  auto w_hbox = mmHBox;
  w_vbox->pack_start(*w_hbox, Gtk::PACK_SHRINK);
  auto w_button = mm<Gtk::Button>("Build");
  w_hbox->pack_start(*w_button, false,false);
  m_skeleton_status_label.set_text("Status: ❌");
  w_hbox->pack_end(m_skeleton_status_label, false,false);

  w_button->signal_clicked().connect( sigc::mem_fun(*this,
     &MainInput::on_button_skeleton_clicked) );
  
  // Lower frame
  w_frame = mmFrameWithBoldLabel("3D Profile");

  this->pack_start(*w_frame, true,true);
  w_grid = mm<Gtk::Grid>();
  row = 0;
  w_vbox = mmVBox; // Main window vbox
  w_frame->add(*w_vbox);

  w_grid->attach(*mmLabelRight("Radius:"), 0,row);
  w_grid->attach(m_radius,               1,row);
  row++;
  w_grid->attach(*mmLabelRight("Num radius steps:"), 0,row);
  w_grid->attach(m_num_radius_steps,              1,row);
  row++;
  w_grid->attach(*mmLabelRight("Z-depth:"), 0,row);
  w_grid->attach(m_zdepth,               1,row);
  row++;
  w_vbox->pack_start(*w_grid, Gtk::PACK_SHRINK);

  // Lower Button box
  w_hbox = mmHBox;
  w_vbox->pack_start(*w_hbox, Gtk::PACK_SHRINK);
  m_profile_button.set_label("Build");
  m_profile_button.set_sensitive(false);
  m_profile_button.signal_clicked().connect( sigc::mem_fun(*this,
     &MainInput::on_button_profile_clicked) );
  w_hbox->pack_start(m_profile_button, false,false);
  m_profile_status_label.set_label("Status: ❌");
  w_hbox->pack_end(m_profile_status_label, false,false);

}

void MainInput::on_button_skeleton_clicked()
{
  auto text = m_text_buffer->get_text();
  double linear_limit = m_linear_limit.get_value();
  auto font_name = m_font_picker.get_font_name();
  PangoFontDescription *p_font_description = pango_font_description_from_string(font_name.c_str());
  Pango::FontDescription font_description(p_font_description);
  m_signal_build_skeleton(text,linear_limit,font_description);
}

void MainInput::on_button_profile_clicked()
{
  m_signal_build_profile(m_radius.get_value(),
                         int(m_num_radius_steps.get_value()),
                         m_zdepth.get_value());
}

void MainInput::set_skeleton_ready_state(bool is_ready)
{
  if (is_ready)
    {
      m_skeleton_status_label.set_text("Status: ✅");
      m_profile_button.set_sensitive(true);
    }
  else
    {
      m_skeleton_status_label.set_text("Status: ❌");
      m_profile_status_label.set_text("Status: ❌");
      m_profile_button.set_sensitive(false);
    }
}

void MainInput::set_profile_ready_state(bool is_ready)
{
  if (is_ready)
    m_profile_status_label.set_text("Status: ✅");
  else
    m_profile_status_label.set_text("Status: ❌");
}

void MainInput::on_skeleton_input_changed()
{
  // Could compare with the last value, but this is good enough
  set_skeleton_ready_state(false);
}

void MainInput::on_profile_input_changed()
{
  // Could compare with the last value, but this is good enough
  set_profile_ready_state(false);
}

MainInput::type_signal_build_skeleton MainInput::signal_build_skeleton()
{
  return m_signal_build_skeleton;
}

MainInput::type_signal_build_profile MainInput::signal_build_profile()
{
  return m_signal_build_profile;
}
