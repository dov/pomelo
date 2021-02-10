// The pomelo settings dialog

#include "settings-dialog.h"
#include "pomelo-widget-utils.h"
#include "dov-mm-macros.h"

using namespace std;

SettingsDialog::SettingsDialog(Gtk::Window& parent,
                               shared_ptr<PomeloSettings> pomelo_settings)
  : Gtk::Dialog("Pomelo Settings", parent),
    m_pomelo_settings(pomelo_settings)
{
  set_default_size(800, 600);

  m_notebook = mm<Gtk::Notebook>();
  get_content_area()->pack_start(*m_notebook);

  m_skeleton_page = mmVBox;
  m_notebook->append_page(*m_skeleton_page,
                          *mmLabel("Skeleton"));

  // Build the skeleton page
  auto w_frame = mmFrameWithBoldLabel("Skeleton settings");
  m_skeleton_page->pack_start(*w_frame, true,true);
  auto w_grid = mm<Gtk::Grid>();
  auto w_vbox = mmVBox; // Main window vbox
  w_vbox->pack_start(*w_grid, Gtk::PACK_SHRINK);
  w_frame->add(*w_vbox);

  // The smooth angle max setup
  m_smooth_angle_max = mm<Gtk::SpinButton>();
  m_smooth_angle_max->set_digits(0);
  m_smooth_angle_max->set_range(0,360);
  m_smooth_angle_max->set_increments(1,10);
  m_smooth_angle_max->set_value(135);

  m_sharp_angles_checkbutton = mm<Gtk::CheckButton>();

  int row=0;
  w_grid->attach(*mmLabelRight("Smooth sharp angles: "), 0,row);
  w_grid->attach(*m_sharp_angles_checkbutton,            1,row);

  row++;
  w_grid->attach(*mmLabelRight("Smooth angle max: "), 0,row);
  w_grid->attach(*m_smooth_angle_max,                 1,row);
  w_grid->attach(*mmLabelLeft("[°]"),                2,row);
  row++;

  m_notebook->show_all();

  load_from_settings();

  add_button("Cancel", 0);
  add_button("Ok", 1);
}

void SettingsDialog::load_from_settings()
{
  m_sharp_angles_checkbutton->set_active(m_pomelo_settings->get_int_default("smooth_sharp_angles", 1));
  m_smooth_angle_max->set_value(m_pomelo_settings->get_double_default("smooth_max_angle", 135));
}

void SettingsDialog::save_to_settings()
{
  m_pomelo_settings->set_int("smooth_sharp_angles",
                             m_sharp_angles_checkbutton->get_active());
  m_pomelo_settings->set_double("smooth_max_angle",
                                m_smooth_angle_max->get_value());
}
