//======================================================================
//  main-input.h - The input values for the top widget
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:03:27 2020
//----------------------------------------------------------------------
#ifndef MAIN_INPUT_H
#define MAIN_INPUT_H

#include <gtkmm.h>

class MainInput : public Gtk::Frame
{
  public:
  MainInput();

  private:
  Gtk::Entry m_text;
  Gtk::FontButton m_fontPicker; 
  Gtk::SpinButton m_radius; // TBD make this a better one!
};

#endif /* MAIN-INPUT */
