#include "main-input.h"
#include "dov-mm-macros.h"
#include "pomelo-widget-utils.h"
#include <math.h>
#include <fmt/core.h>

using namespace fmt;

constexpr double DEG2RAD = M_PI/180;

MainInput::MainInput()
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
  auto w_frame = mmFrameWithBoldLabel("Skeleton");
  this->pack_start(*w_frame, true,true);
  auto w_vbox = mmVBox; // Main window vbox
  auto w_grid = mm<Gtk::Grid>();

  // Configure the widgets
  m_text.set_hexpand(true);
  m_text.set_focus_on_click(true);
  m_text.signal_focus_in_event().connect( sigc::mem_fun(*this, &MainInput::on_text_focus_in));
  m_text_buffer = m_text.get_buffer();
  m_text_buffer->set_text("Pomelo");
  m_text_buffer->signal_changed().connect( sigc::mem_fun(*this,
    &MainInput::on_skeleton_input_changed));

  // Setup the linear limit button
  m_linear_limit.set_digits(3);
  m_linear_limit.set_range(0.001,20);
  m_linear_limit.set_increments(0.01,1);
  m_linear_limit.set_value(5.0);
  m_linear_limit.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_skeleton_input_changed));

  m_font_picker.set_show_size(false);
#ifdef WIN32
  m_font_picker.set_font_name("Arial Black Heavy 48");
#else
  m_font_picker.set_font_name("Sans Bold 48");
#endif
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
  m_num_radius_steps.set_range(1,50);
  m_num_radius_steps.set_increments(1,10);
  m_num_radius_steps.set_value(10);
  m_num_radius_steps.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_profile_input_changed));

  // Setup the round max angle steps
  m_round_max_angle_in_degrees.set_digits(0);
  m_round_max_angle_in_degrees.set_range(0,180);
  m_round_max_angle_in_degrees.set_increments(1,10);
  m_round_max_angle_in_degrees.set_value(90);
  m_round_max_angle_in_degrees.signal_value_changed().connect(sigc::mem_fun(*this,
    &MainInput::on_profile_input_changed));

  // The zdepth spin button
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
  m_skeleton_button.set_label("Build");
  w_hbox->pack_start(m_skeleton_button, false,false);
  m_skeleton_status_label.set_markup("Status: <span foreground=\"red\">❌</span>");
  w_hbox->pack_end(m_skeleton_status_label, false,false);

  m_skeleton_button.signal_clicked().connect( sigc::mem_fun(*this,
     &MainInput::on_button_skeleton_clicked) );
  
  // Lower frame
  w_frame = mmFrameWithBoldLabel("3D Profile");
  this->pack_start(*w_frame, true,true);

  w_vbox = mmVBox; // Main window vbox
  w_frame->add(*w_vbox);

  w_hbox = mmHBox;
  m_type_chooser.append("Round");
  m_type_chooser.append("Curve");
  m_type_chooser.set_active(0);
  m_type_chooser.signal_changed().connect(
     sigc::mem_fun(*this,
                   &MainInput::on_combo_type_chooser_changed) );  
  w_hbox->pack_start(*mmLabel("Profile option: "), false,false);
  w_hbox->pack_start(m_type_chooser, false,false);
  w_vbox->pack_start(*w_hbox, false,false);

  m_profile_type_notebook.set_show_tabs(false); // Will be controlled by the type chooser
  w_vbox->pack_start(m_profile_type_notebook, true,true);

  w_grid = mm<Gtk::Grid>();
  row = 0;

  w_grid->attach(*mmLabelRight("Radius:"), 0,row);
  w_grid->attach(m_radius,               1,row);
  row++;
  w_grid->attach(*mmLabelRight("Num radius steps:"), 0,row);
  w_grid->attach(m_num_radius_steps,              1,row);
  row++;
  w_grid->attach(*mmLabelRight("Round max angle:"), 0,row);
  w_grid->attach(m_round_max_angle_in_degrees, 1,row);
  w_grid->attach(*mmLabelLeft("[Degrees]"), 2, row);
  row++;
  m_profile_type_notebook.append_page(*w_grid, "");

  w_grid = mm<Gtk::Grid>();
  row = 0;

  m_profile_edit_button.set_label("Edit");
  m_profile_edit_button.signal_clicked().connect( sigc::mem_fun(*this,
     &MainInput::on_button_profile_edit_clicked) );

  w_grid->attach(*mmLabelRight("Profile:"), 0,row);
  auto w_prof_chooser = mm<Gtk::ComboBoxText>();
  w_prof_chooser->append("Prof1");
  w_prof_chooser->append("Prof2");
  w_prof_chooser->set_active(0);
  w_grid->attach(*w_prof_chooser,       1,row);
  w_grid->attach(m_profile_edit_button, 2,row);
  m_profile_type_notebook.append_page(*w_grid, "");
  row++;
  
  // Common options
  w_grid = mm<Gtk::Grid>();
  row = 0;
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
  m_profile_status_label.set_markup("Status: <span foreground=\"red\">❌</span>");

  w_hbox->pack_end(m_profile_status_label, false,false);

  // Create tags or the text buffer
  Glib::RefPtr<Gtk::TextBuffer::Tag> refTag;

  refTag = m_text_buffer->create_tag("info");
  refTag->property_foreground() = "#808080";
}

void MainInput::on_button_skeleton_clicked()
{
  auto text = m_text_buffer->get_text();
  double linear_limit = m_linear_limit.get_value();
  auto font_name = m_font_picker.get_font_name();
  print("font_name = {}\n", font_name.c_str());
  Pango::FontDescription font_description(font_name);
  m_signal_build_skeleton(text,linear_limit,font_description);
}

void MainInput::on_button_profile_clicked()
{
  m_signal_build_profile(m_radius.get_value(),
                         m_round_max_angle_in_degrees.get_value()*DEG2RAD,
                         int(m_num_radius_steps.get_value()),
                         m_zdepth.get_value());
}

void MainInput::on_button_profile_edit_clicked()
{
#if 0
  m_profile_editor_window.show();
  m_profile_editor_window.set_profile(profile_row);
#endif
  print("TBD");
}

void MainInput::set_skeleton_ready_state(bool is_ready)
{
  if (is_ready)
    {
      m_skeleton_status_label.set_markup("Status: <span foreground=\"green\">✅</span>");
      m_profile_button.set_sensitive(true);
    }
  else
    {
      m_skeleton_status_label.set_markup("Status: <span foreground=\"red\">❌</span>");

      m_profile_status_label.set_markup("Status: <span foreground=\"red\">❌</span>");
      m_profile_button.set_sensitive(false);
    }
}

void MainInput::set_profile_ready_state(bool is_ready)
{
  if (is_ready)
    m_profile_status_label.set_markup("Status: <span foreground=\"green\">✅</span>");
  else
    m_profile_status_label.set_markup("Status: <span foreground=\"red\">❌</span>");
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

//void MainInput::on_text_insert_at_cursor(const Glib::ustring& str)
bool MainInput::on_text_focus_in(GdkEventFocus*)
{
  if (m_clean_on_edit)
    {
      m_clean_on_edit = false;
      m_text_buffer->set_text("");
      m_signal_text_edited();
    }
  return true;
}

MainInput::type_signal_build_skeleton MainInput::signal_build_skeleton()
{
  return m_signal_build_skeleton;
}

MainInput::type_signal_build_profile MainInput::signal_build_profile()
{
  return m_signal_build_profile;
}

MainInput::type_signal_text_edited MainInput::signal_text_edited()
{
  return m_signal_text_edited;
}

void MainInput::set_text_edit_info_string(const Glib::ustring& info_string)
{
  m_skeleton_button.grab_focus();
  m_text_buffer->set_text("");
  auto iter = m_text_buffer->begin();
  m_text_buffer->insert_with_tag(iter,info_string,"info");
  m_clean_on_edit = true;
  
}

void MainInput::on_combo_type_chooser_changed()
{
  int active_page = m_type_chooser.get_active_row_number();
  m_profile_type_notebook.set_current_page(active_page);
}
