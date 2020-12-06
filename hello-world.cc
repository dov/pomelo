// A template application.
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "hello-world.h"

using namespace std;

HelloWorld::HelloWorld()
  : m_box(Gtk::ORIENTATION_VERTICAL)
{
  set_title("hello world example");
  set_default_size(400, 400);

  add(m_box); 

  //Define the actions:
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refActionGroup->add_action("quit",
    sigc::mem_fun(*this, &HelloWorld::on_action_file_quit) );

  m_refActionGroup->add_action("about",
    sigc::mem_fun(*this, &HelloWorld::on_action_help_about) );

  insert_action_group("helloworld", m_refActionGroup);
  
  //Layout the actions in a menubar and toolbar:
  const char* ui_info =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Quit</attribute>"
    "          <attribute name='action'>helloworld.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Help</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>_About</attribute>"
    "        <attribute name='action'>helloworld.about</attribute>"
    "      </item>"
    "    </submenu>"
    "  </menu>"
    "</interface>";

  m_refBuilder = Gtk::Builder::create();
  m_refBuilder->add_from_string(ui_info);

  // Get the menubar:. Must downcast objects that are got from the refbuilder
  auto object = m_refBuilder->get_object("menubar");
  auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if (!gmenu)
    g_warning("GMenu not found");
  else
  {
    auto pMenuBar = Gtk::make_managed<Gtk::MenuBar>(gmenu);

    //Add the MenuBar to the window:
    m_box.pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  }

  m_scrolledWindow.add(m_textView);
  m_box.pack_start(m_scrolledWindow, true, true);

  m_box.pack_start(m_statusbar, false, false);
  m_statusbar.push("Welcome to Hello World");

  show_all_children();
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_button_clicked()
{
  std::cout << "Hello world\n";
}

//Signal handlers:
void HelloWorld::on_action_file_quit()
{
  hide(); // Close the main window to stop app->run().
}

void HelloWorld::on_action_help_about()
{
  Gtk::AboutDialog Dialog;
  Dialog.set_version("0.0.1");
  Dialog.set_copyright("Joe Doe");
  Dialog.set_program_name("HelloWorld");
  Dialog.set_transient_for(*this);

  Dialog.run();
}

