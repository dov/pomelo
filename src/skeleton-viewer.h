//======================================================================
//  skeleton-viewer.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Jan  7 19:37:30 2021
//----------------------------------------------------------------------
#ifndef SKELETON_VIEWER_H
#define SKELETON_VIEWER_H

#include <gtkmm.h>
#include "giv-widget.h"

class SkeletonViewer : public Gtk::Dialog
{
  public:
  SkeletonViewer(Gtk::Window& parent);

  void set_giv_string(std::shared_ptr<std::string> giv_string);

  private:
  GtkWidget *w_giv_widget; // c-interface widget until I write a gtkmm wrapper
                           // for the giv widget.
};

#endif /* SKELETON-VIEWER */
