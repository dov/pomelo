//======================================================================
//  main-input.h - The input values for the top widget
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:03:27 2020
//----------------------------------------------------------------------
#ifndef MAIN_INPUT_H
#define MAIN_INPUT_H

#include <gtkmm.h>
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

  // Setup UI for readiness
  void set_skeleton_ready_state(bool is_ready);
  void set_profile_ready_state(bool is_ready);

  // Change the text to a placeholder
  void set_text_edit_info_string(const Glib::ustring& info_string);

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

  // If the text was set externally then this flag tells the
  // widget that on the next edit, we should clean and send a signal
  // about it.
  bool m_clean_on_edit = false;
  bool use_profile_data = false; // Type of profiling round or profile data

  // Signals
  type_signal_build_skeleton m_signal_build_skeleton;
  type_signal_build_profile m_signal_build_profile;
  type_signal_text_edited m_signal_text_edited;

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
