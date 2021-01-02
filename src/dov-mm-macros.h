//======================================================================
//  dov-mm-macros.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:21:41 2020
//----------------------------------------------------------------------
#ifndef DOV_MM_MACROS_H
#define DOV_MM_MACROS_H

// Convenient shortcuts
#define mm Gtk::make_managed
#define mmVBox mm<Gtk::Box>(Gtk::ORIENTATION_VERTICAL)
#define mmHBox mm<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL)
#define mmLabel(label) mm<Gtk::Label>(label)


#endif /* DOV-MM-MACROS */
