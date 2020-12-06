//======================================================================
//  hello-world.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Dec  6 22:36:33 2020
//----------------------------------------------------------------------
#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <iostream>

class HelloWorld : public Gtk::Window
{

public:
  HelloWorld();
  virtual ~HelloWorld();

private:
  //Signal handlers:
  void on_button_clicked();

  //Signal handlers:
  void on_action_file_quit();
  void on_action_help_about();

  //Member widgets:
  Gtk::Box m_box;
  Gtk::ScrolledWindow m_scrolledWindow; // Main window contents
  Gtk::TextView m_textView;
  Gtk::Statusbar m_statusbar;
  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
};

#endif /* HELLO-WORLD */
