//======================================================================
//  settings-dialog.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 21:07:03 2021
//----------------------------------------------------------------------
#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/grid.h>
#include "pomelo-settings.h"

class SettingsDialog : public Gtk::Dialog
{
  public:
  SettingsDialog(Gtk::Window& parent,
                 std::shared_ptr<PomeloSettings> pomelo_settings);
  ~SettingsDialog(){}
  void load_from_settings();
  void save_to_settings();
  void set_color(int level, const std::string& color);
  // Whether the skeleton parameters have changed
  bool skeleton_params_have_changed();

  private:
  Gtk::Notebook *m_notebook;
  Gtk::Box *m_skeleton_page;
  Gtk::CheckButton *m_sharp_angles_checkbutton;
  Gtk::SpinButton *m_smooth_angle_max;
  std::shared_ptr<PomeloSettings> m_pomelo_settings;
  Gtk::ColorButton *m_background_chooser;
  Gtk::ColorButton *m_mesh_color_chooser;
  Gtk::ColorButton *m_mesh_color_level1_chooser;
  Gtk::ColorButton *m_mesh_color_level2_chooser;
  Gtk::FileChooserButton *m_matcap_chooser;
};


#endif /* SETTINGS-DIALOG */
