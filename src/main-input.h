//======================================================================
//  main-input.h - The input values for the top widget
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:03:27 2020
//----------------------------------------------------------------------
#ifndef MAIN_INPUT_H
#define MAIN_INPUT_H

#include <gtkmm.h>

class MainInput : public Gtk::Box
{
  public:
  MainInput();

  //signal accessor:
  using type_signal_build_skeleton = sigc::signal<void(
    Glib::ustring,  // Text string
    double,         // Linear limit
    Pango::FontDescription font_decription   // Font
    )>;
  type_signal_build_skeleton signal_build_skeleton();

  using type_signal_build_profile = sigc::signal<void(
    double,         // Radius
    double,         // Round max angle in radians
    int,            // Num radius steps
    double          // ZDepth
    )>;
  type_signal_build_profile signal_build_profile();

  // Setup UI for readiness
  void set_skeleton_ready_state(bool is_ready);
  void set_profile_ready_state(bool is_ready);

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
  Gtk::Button m_profile_button;

  // Signals
  type_signal_build_skeleton m_signal_build_skeleton;
  type_signal_build_profile m_signal_build_profile;

  void on_button_skeleton_clicked();
  void on_button_profile_clicked();
  void on_skeleton_input_changed();
  void on_profile_input_changed();

};

#endif /* MAIN-INPUT */
