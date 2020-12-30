// A template application.
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "mesh-viewer.h"
#include "pomelo.h"
#include <iostream>
#include "dov-mm-macros.h"

using namespace std;



Pomelo::Pomelo()
{
  set_title("Pomelo");
  set_default_size(800, 1000);

  auto w_vbox = mmVBox;

  this->add(*w_vbox); 

  //Define the actions:
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refActionGroup->add_action("quit",
    sigc::mem_fun(*this, &Pomelo::on_action_file_quit) );

  m_refActionGroup->add_action("about",
    sigc::mem_fun(*this, &Pomelo::on_action_help_about) );

  insert_action_group("pomelo", m_refActionGroup);
  
  //Layout the actions in a menubar and toolbar:
  const char* ui_info =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Quit</attribute>"
    "          <attribute name='action'>pomelo.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Help</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>_About</attribute>"
    "        <attribute name='action'>pomelo.about</attribute>"
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
    w_vbox->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  }

  w_vbox->pack_start(m_mainInput, false, true);


  // A box for button widgets. Not worth putting this into a widget
  auto w_hbox = mmHBox;
  auto w_button = mm<Gtk::Button>("Build");
  w_button->signal_clicked().connect( sigc::mem_fun(*this,
     &Pomelo::on_button_clicked) );

  w_hbox->pack_start(*w_button, false, false);
  w_vbox->pack_start(*w_hbox, false, true);
  
  

  w_vbox->pack_start(m_meshViewer, true, true);
  w_vbox->pack_start(m_statusbar, false, false);
  m_statusbar.push("Welcome to Hello World");

  show_all_children();
}

Pomelo::~Pomelo()
{
}

//Signal handlers:
void Pomelo::on_action_file_quit()
{
  hide(); // Close the main window to stop app->run().
}

void Pomelo::on_action_help_about()
{
  Gtk::AboutDialog Dialog;
  Dialog.set_version("0.0.1");
  Dialog.set_copyright("Dov Grobgeld");
  Dialog.set_program_name("Pomelo");
  Dialog.set_transient_for(*this);

  Dialog.run();
}

void Pomelo::on_button_clicked()
{
  m_statusbar.push("That felt good!");

  Glib::signal_timeout()
    .connect_once([this]() { m_statusbar.remove_all_messages(); }, 1000);
}

void Pomelo::set_mesh(const std::string& mesh_filename)
{
  m_meshViewer.set_mesh_file(mesh_filename);
}
