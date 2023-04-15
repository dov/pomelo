//======================================================================
//  profile-editor-window.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jul 11 22:05:34 2021
//----------------------------------------------------------------------

#include "profile-editor-window.h"
#include "fmt/core.h"
#include <spdlog/spdlog.h>

using namespace fmt;

ProfileEditorWindow::ProfileEditorWindow(Gtk::Window& parent)
  : Gtk::Dialog("Profile Editor",
                parent,
                false // modal
                ),
    profile_editor(Gtk::make_managed<ProfileEditor>())
{
  spdlog::info("Creating the profile editor window");
  profile_editor->set_size_request(800,800);
  get_content_area()->pack_start(*profile_editor, true,true,0);
  get_content_area()->show_all();
  
  add_button("Accept",0);
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
  if (profile_editor)
    profile_editor->set_profile(prof);
  else
    print("Oops\n");
}

