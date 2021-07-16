//======================================================================
//  profile-editor-window.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jul 11 22:05:34 2021
//----------------------------------------------------------------------

#include "profile-editor-window.h"

ProfileEditorWindow::ProfileEditorWindow(Gtk::Window& parent)
  : Gtk::Dialog("Profile Editor",
                parent,
                false // modal
                ),
    profile_editor(Gtk::make_managed<ProfileEditor>())
{
  profile_editor->set_size_request(400,400);
  get_content_area()->pack_start(*profile_editor);
  get_content_area()->show_all();
  
  add_button("Save",0);
  add_button("Cancel",1);
}

// Get a copy of of the profile
ProfileData ProfileEditorWindow::get_profile()
{
  ProfileData prof = profile_editor->get_profile();

  return prof;
}

// Set the profile from the external data
void ProfileEditorWindow::set_profile(const ProfileData& prof)
{
  profile_editor->set_profile(prof);
}

