//======================================================================
//  dov-mm-macros.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Dec 25 08:21:41 2020
//----------------------------------------------------------------------
#ifndef DOV_MM_MACROS_H
#define DOV_MM_MACROS_H

// Convenient shortcuts
#define mmVBox Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL)
#define mmHBox Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL)
#define mmLabel(label) Gtk::make_managed<Gtk::Label>(label)


#endif /* DOV-MM-MACROS */
