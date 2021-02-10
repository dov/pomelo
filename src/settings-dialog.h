//======================================================================
//  settings-dialog.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 21:07:03 2021
//----------------------------------------------------------------------
#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <gtkmm.h>
#include "pomelo-settings.h"

class SettingsDialog : public Gtk::Dialog
{
  public:
  SettingsDialog(Gtk::Window& parent,
                 std::shared_ptr<PomeloSettings> pomelo_settings);
  ~SettingsDialog(){}
  void load_from_settings();
  void save_to_settings();

  private:
  Gtk::Notebook *m_notebook;
  Gtk::Box *m_skeleton_page;
  Gtk::CheckButton *m_sharp_angles_checkbutton;
  Gtk::SpinButton *m_smooth_angle_max;
  std::shared_ptr<PomeloSettings> m_pomelo_settings;
};


#endif /* SETTINGS-DIALOG */
