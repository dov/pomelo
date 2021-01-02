// A template application.
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "mesh-viewer.h"
#include "pomelo.h"
#include <iostream>
#include "dov-mm-macros.h"

using namespace std;
using namespace fmt;


Pomelo::Pomelo()
  : m_progress_dialog(*this,""),
    m_worker_skeleton(this)
{
  set_title("Pomelo");
  set_default_size(800, 1000);

  auto w_vbox = mmVBox;

  this->add(*w_vbox); 

  //Define the actions:
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refActionGroup->add_action("export_stl",
    sigc::mem_fun(*this, &Pomelo::on_action_file_export_stl) );

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
    "          <attribute name='label' translatable='yes'>_Export STL</attribute>"
    "          <attribute name='action'>pomelo.export_stl</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;s</attribute>"
    "        </item>"
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

  m_main_input.signal_build_skeleton().connect( sigc::mem_fun(*this,
     &Pomelo::on_build_skeleton) );
  m_main_input.signal_build_profile().connect( sigc::mem_fun(*this,
     &Pomelo::on_build_profile) );

  w_vbox->pack_start(m_main_input, false, true);

  w_vbox->pack_start(m_mesh_viewer, true, true);
  w_vbox->pack_start(m_statusbar, false, false);
  m_statusbar.push("Welcome to Hello World");

  // Connect the handler to the dispatcher.
  m_dispatcher.connect(sigc::mem_fun(*this, &Pomelo::on_notification_from_skeleton_worker_thread));

  show_all_children();

  m_progress_dialog.signal_cancel().connect(
    [this]() {
      m_worker_skeleton.stop_work();
    });

  m_updator = make_shared<PomeloUpdater>(this);
}

Pomelo::~Pomelo()
{
}

//Signal handlers:
void Pomelo::on_action_file_export_stl()
{
  string mesh_filename;

  auto dialog = Gtk::FileChooserNative::create("Mesh filename",
                                               *this,
                                               Gtk::FILE_CHOOSER_ACTION_SAVE);
  
  // Show the dialog and wait for a user response:
  const int result = dialog->run();

  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    mesh_filename = dialog->get_filename();
    auto mesh = m_worker_skeleton.get_mesh();
    save_stl(mesh, mesh_filename);
    set_status(format("Saved mesh with {} vertices to {}\n",
                      mesh->vertices.size(),
                      mesh_filename));

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled STL export");
    break;

  default:
    break;
  }
}


//Signal handlers:
void Pomelo::on_action_file_quit()
{
  hide(); // Close the main window to stop app->run().
}

void Pomelo::on_action_help_about()
{
  Gtk::AboutDialog Dialog;
  Dialog.set_logo(Gdk::Pixbuf::create_from_resource("/about/pomelo_logo.png", -1, 80, true));

  Dialog.set_version("0.0.1");
  Dialog.set_copyright("Dov Grobgeld <dov.grobgeld@gmail.com>");
  Dialog.set_comments("A program for generating 3D meshes of text");

  Glib::RefPtr<const Glib::Bytes> copying_bytes = Gio::Resource::lookup_data_global("/about/COPYING");

  gsize len=0;
  const char* copying_txt = (const char*)copying_bytes->get_data(len);

  Dialog.set_license(copying_txt);
  Dialog.set_program_name("Pomelo");
  Dialog.set_website("http://github.com/dov/pomelo");
  Dialog.set_website_label("pomelo website");
  Dialog.set_transient_for(*this);

  std::vector<Glib::ustring> list_authors;
  list_authors.push_back("Dov Grobgeld");
  Dialog.set_authors(list_authors);
  Dialog.run();
}

#if 0
void Pomelo::on_button_clicked()
{
  m_statusbar.push("That felt good!");

  Glib::signal_timeout()
    .connect_once([this]() { m_statusbar.remove_all_messages(); }, 1000);
}
#endif

void Pomelo::set_mesh(const std::string& mesh_filename)
{
  m_mesh_viewer.set_mesh_file(mesh_filename);
}

void Pomelo::on_build_skeleton(Glib::ustring text_string,
                               double linear_limit,
                               Pango::FontDescription font_description)
{
  m_progress_dialog.set_title("Build Skeleton");
  m_progress_dialog.show();

  bool do_rtl = false;

  // Start a new worker thread.
  m_worker_action = ACTION_SKELETON;
  m_worker_skeleton_thread = make_unique<std::thread>(
    [=] {
      m_worker_skeleton.do_work_skeleton(
                                         do_rtl,
                                         font_description,
                                         linear_limit,
                                         text_string);
    });
  
}

void Pomelo::on_build_profile(double radius,
                              int num_radius_steps,
                              double zdepth)
{
  m_progress_dialog.set_title("Build Profile");
  m_progress_dialog.show();

  // Set parameters
  // Start a new worker thread.
  m_worker_action = ACTION_PROFILE;
  m_worker_skeleton_thread = make_unique<std::thread>(
    [=] {
      m_worker_skeleton.do_work_profile(radius,num_radius_steps,zdepth);
    });

  m_main_input.set_profile_ready_state(true);
}

void Pomelo::set_status(const string& message)
{
  m_statusbar.remove_all_messages();
  m_statusbar.push(message);
}

Updater::ContinueStatus PomeloUpdater::info(const std::string& context, double progress)
{
  auto lambda = [this,context,progress]()->bool {
    m_pomelo->set_status(context + format("{}", progress));
    return true;
  };
  Glib::signal_idle().connect (lambda);

  return Updater::UPDATER_OK;
}

// notify() is called from the worker thread. It is executed in the worker
// thread. It triggers a call to on_notification_from_worker_thread(), which is
// executed in the GUI thread.
void Pomelo::notify()
{
  m_dispatcher.emit();
}

// Dispatcher handler.
void Pomelo::on_notification_from_skeleton_worker_thread()
{
  if (m_worker_skeleton_thread && m_worker_skeleton.has_stopped())
    {
      // Work is done.
      if (m_worker_skeleton_thread->joinable())
        m_worker_skeleton_thread->join();
      m_worker_skeleton_thread.reset();

      if (m_worker_skeleton.get_finished_successfully())
        {
          if (m_worker_action == ACTION_SKELETON)
            m_main_input.set_skeleton_ready_state(true);
          else
            {
              m_main_input.set_profile_ready_state(true);

              // Set the mesh!
              auto mesh = m_worker_skeleton.get_mesh();
              m_mesh_viewer.set_mesh(mesh);
              m_mesh_viewer.redraw();
            }
        }
      else
        {
          auto error_message = m_worker_skeleton.get_error_message();
          if (error_message.size())
            {
              Gtk::MessageDialog dialog(*this,
                                        error_message,
                                        false, // use_markup
                                        Gtk::MESSAGE_ERROR,
                                        Gtk::BUTTONS_OK,
                                        true // modal
                                        );
              dialog.run();
            }
          set_status("Cancelled!");
        }
      m_progress_dialog.hide();
    }
  else
    {
      // update the gui
      double fraction_done;
      Glib::ustring message;
      m_worker_skeleton.get_progress(// output
                                     fraction_done, message);
      m_progress_dialog.update(message,fraction_done);
  
  
      set_status(format("Progress {:.0f}%: {}", fraction_done*100, message.c_str()));
    }
}
