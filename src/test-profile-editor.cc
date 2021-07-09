#include <gtkmm.h>
#include "profile-editor.h"

class TestProfileEditor : public Gtk::Window
{
public:
  TestProfileEditor();

protected:
  //Member widgets:
  ProfileEditor *profile_editor;
};

TestProfileEditor::TestProfileEditor()
  : profile_editor(Gtk::make_managed<ProfileEditor>())
{
  // Sets the border width of the window.
  set_border_width(10);
  set_can_focus(true);
  profile_editor->set_size_request(400,400);

  add(*profile_editor);

  // The final step is to display this newly created widget...
  show_all_children();
  set_focus(*profile_editor);
  grab_focus();
}

int main(int argc, char *argv[])
{
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.test-profile-editor");

  TestProfileEditor mw;

  return app->run(mw);
}
