//======================================================================
//  profile-editor-window.h - A dialog for the profile editor
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jul 11 22:05:34 2021
//----------------------------------------------------------------------
#ifndef PROFILE_EDITOR_WINDOW_H
#define PROFILE_EDITOR_WINDOW_H

#include <gtkmm/dialog.h>
#include "profile-editor.h"
#include "profile.h"

class ProfileEditorWindow : public Gtk::Dialog {
  public:
  // contructor
  ProfileEditorWindow(Gtk::Window& parent);

  // Get a copy of of the profile
  ProfileData get_profile();
  
  // Set the profile from the external data
  void set_profile(const ProfileData& prof);

  // Get whether the profile is positive monotone
  bool get_is_positive_monotone() const;

  private:
  ProfileEditor *profile_editor=nullptr;
};


#endif /* PROFILE-EDITOR-WINDOW */
