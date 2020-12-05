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

protected:
  //Signal handlers:
  void on_button_clicked();

  //Member widgets:
  Gtk::Button m_button;
};

#endif /* HELLO-WORLD */
