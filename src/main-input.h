//======================================================================
//  main-input.h - The input values for the top widget
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:03:27 2020
//----------------------------------------------------------------------
#ifndef MAIN_INPUT_H
#define MAIN_INPUT_H

#include <gtkmm/textview.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/notebook.h>
#include <gtkmm/grid.h>
#include "profile-editor-window.h"

class MainInput : public Gtk::Box
{
  public:
  MainInput(Gtk::Window& window);

  //signal accessor:
  using type_signal_build_skeleton = sigc::signal<void(
    Glib::ustring,  // Text string
    double,         // Linear limit
    Pango::FontDescription font_decription   // Font
    )>;
  type_signal_build_skeleton signal_build_skeleton();

  using type_signal_build_profile = sigc::signal<void(
    bool,           // use profile_data
    double,         // Radius
    double,         // Round max angle in radians
    int,            // Num radius steps
    double,         // ZDepth
    ProfileData     // profile
    )>;
  type_signal_build_profile signal_build_profile();

  // When the text is edited, the following signal is sent
  using type_signal_text_edited = sigc::signal<void()>;
  type_signal_text_edited signal_text_edited();

  // When the profile has been edited, the following signal is sent
  using type_signal_profile_edited = sigc::signal<void()>;
  type_signal_profile_edited signal_profile_edited();

  // Setup UI for readiness
  void set_skeleton_ready_state(bool is_ready);
  void set_profile_ready_state(bool is_ready);

  // Populate from external sources
  void set_text_edit_string(const Glib::ustring& info_string,
                            bool is_info);
  std::string get_text_edit_string();
  void set_font_name(const Glib::ustring& font_string);
  std::string get_font_name();
  void set_profile_option(int profile_option); // 0 round, 1 is curve
  int get_profile_option();
  void set_round_profile(double radius,
                         int num_radius_steps,
                         double round_max_angle);
  void get_round_profile_params(double& radius,
                                int& num_radius_steps,
                                double& round_max_angle);
  void set_zdepth(double zdepth);
  double get_zdepth();
  
  // Get the profile for storing in a external storage
  // Get a string representation of the current profile
  Glib::ustring get_profile_string();

  // set the profile
  void set_profile(const Glib::ustring& profile_string);

  void set_debug_dir(const std::string& debug_dir) {
    m_debug_dir = debug_dir;
  }

  private:
  Gtk::TextView m_text;
  Glib::RefPtr<Gtk::TextBuffer> m_text_buffer;
  Gtk::FontButton m_font_picker; 
  Gtk::SpinButton m_radius; 
  Gtk::SpinButton m_num_radius_steps; 
  Gtk::SpinButton m_round_max_angle_in_degrees; 
  Gtk::SpinButton m_zdepth; 
  Gtk::SpinButton m_linear_limit;
  Gtk::Label m_skeleton_status_label;
  Gtk::Label m_profile_status_label;
  Gtk::Button m_skeleton_button;
  Gtk::Button m_profile_button;
  Gtk::Button m_profile_edit_button;
  Gtk::ComboBoxText m_type_chooser;
  Gtk::Notebook m_profile_type_notebook;
  ProfileEditorWindow m_profile_editor_window;
  std::string m_debug_dir;

  // If the text was set externally then this flag tells the
  // widget that on the next edit, we should clean and send a signal
  // about it.
  bool m_clean_on_edit = false;
  bool use_profile_data = false; // Type of profiling round or profile data

  // Signals
  type_signal_build_skeleton m_signal_build_skeleton;
  type_signal_build_profile m_signal_build_profile;
  type_signal_text_edited m_signal_text_edited;
  type_signal_profile_edited m_signal_profile_edited;

  void on_button_skeleton_clicked();
  void on_button_profile_clicked();
  void on_button_profile_edit_clicked();
  void on_skeleton_input_changed();
  void on_profile_input_changed();
  //  void on_text_insert_at_cursor(const Glib::ustring& str);
  bool on_text_focus_in(GdkEventFocus*);
  void on_combo_type_chooser_changed();
};

#endif /* MAIN-INPUT */
