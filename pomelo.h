//======================================================================
//  pomelo.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Dec  6 22:36:33 2020
//----------------------------------------------------------------------
#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

#include <gtkmm.h>
#include "main-input.h"
#include "mesh-viewer.h"

class Pomelo : public Gtk::Window
{

public:
  Pomelo();
  virtual ~Pomelo();

  void set_mesh(const std::string& mesh_filename);

private:
  //Signal handlers:
  void on_button_clicked();

  //Signal handlers:
  void on_action_file_quit();
  void on_action_help_about();

  //Member widgets:
  MainInput m_mainInput;
  MeshViewer m_meshViewer;
  Gtk::Statusbar m_statusbar;
  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
};

#endif /* HELLO-WORLD */
