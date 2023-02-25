//======================================================================
// The pomelo application file
//
// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2021-03-06 Sat
//----------------------------------------------------------------------
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "mesh-viewer.h"
#include "pomelo.h"
#include <iostream>
#include "dov-mm-macros.h"

using namespace std;
using namespace fmt;


Pomelo::Pomelo(shared_ptr<PomeloSettings> pomelo_settings)
  : m_main_input(*this),
    m_mesh_viewer(pomelo_settings),
    m_progress_dialog(*this,""),
    m_worker_skeleton(this, pomelo_settings),
    m_pomelo_settings(pomelo_settings)
{
  set_title("Pomelo");

  //set_icon(Gdk::Pixbuf::create_from_resource("/about/pomelo_logo.png", -1, 80, true));
  set_default_size(800, 1000);

  auto w_vbox = mmVBox;

  this->add(*w_vbox); 

  m_skeleton_viewer = Glib::RefPtr<SkeletonViewer>(new SkeletonViewer(*this));
  m_skeleton_viewer->signal_response().connect([=](int response_id) {
    m_skeleton_viewer->hide();
  });

  m_settings_dialog = Glib::RefPtr<SettingsDialog>(new SettingsDialog(*this,
                                                                      m_pomelo_settings));
  m_settings_dialog->signal_response().connect([=](int response_id)
  {
    if (response_id==1) // OK - Should make this an enum
      {
        m_settings_dialog->save_to_settings();
        m_pomelo_settings->save();
        m_main_input.set_skeleton_ready_state(false);
        m_mesh_viewer.refresh_from_settings();
      }
    else
      m_settings_dialog->load_from_settings();
      
    m_settings_dialog->hide();
  });

  //Define the actions:
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refActionGroup->add_action("export_stl",
    sigc::mem_fun(*this, &Pomelo::on_action_file_export_stl) );

  m_refActionGroup->add_action("export_gltf",
    sigc::mem_fun(*this, &Pomelo::on_action_file_export_gltf) );

  m_refActionGroup->add_action("load_svg",
    sigc::mem_fun(*this, &Pomelo::on_action_load_svg) );

  m_refActionGroup->add_action("quit",
    sigc::mem_fun(*this, &Pomelo::on_action_file_quit) );

  m_refActionGroup->add_action("about",
    sigc::mem_fun(*this, &Pomelo::on_action_help_about) );

  m_refActionGroup->add_action("reset_3d_view",
    sigc::mem_fun(*this, &Pomelo::on_action_reset_3d_view) );

  m_refActionGroup->add_action("view_skeleton",
    sigc::mem_fun(*this, &Pomelo::on_action_view_skeleton) );

  bool init_orthonormal = m_pomelo_settings->get_int_default("orthonormal",0);
  m_mesh_viewer.set_orthonormal(init_orthonormal);
  m_ref_orthonormal_toggle = m_refActionGroup->add_action_bool("orthonormal_camera",
    sigc::mem_fun(*this, &Pomelo::on_action_orthonormal), init_orthonormal );

  bool init_show_edge = m_pomelo_settings->get_int_default("show_edge",0);
  m_mesh_viewer.set_show_edge(init_show_edge);
  m_ref_show_edge_toggle = m_refActionGroup->add_action_bool("show_edge",
    sigc::mem_fun(*this, &Pomelo::on_action_show_edge), init_show_edge );

  bool init_show_matcap = m_pomelo_settings->get_int_default("show_matcap",0);
  m_mesh_viewer.set_show_matcap(init_show_matcap);
  m_ref_show_matcap_toggle = m_refActionGroup->add_action_bool("show_matcap",
    sigc::mem_fun(*this, &Pomelo::on_action_show_matcap), init_show_matcap );

  m_ref_layer0_toggle = m_refActionGroup->add_action_bool("layer0",
    sigc::mem_fun(*this, &Pomelo::on_action_view_layer0), true );
  m_ref_layer1_toggle = m_refActionGroup->add_action_bool("layer1",
    sigc::mem_fun(*this, &Pomelo::on_action_view_layer1), true );
  m_ref_layer2_toggle = m_refActionGroup->add_action_bool("layer2",
    sigc::mem_fun(*this, &Pomelo::on_action_view_layer2), true );

  m_refActionGroup->add_action("settings",
    sigc::mem_fun(*this, &Pomelo::on_action_view_settings) );

  insert_action_group("pomelo", m_refActionGroup);
  
  //Layout the actions in a menubar and toolbar:
  const char* ui_info =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label'>_Export STL</attribute>"
    "          <attribute name='action'>pomelo.export_stl</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;s</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Export GLTF</attribute>"
    "          <attribute name='action'>pomelo.export_gltf</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;g</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Load SVG</attribute>"
    "          <attribute name='action'>pomelo.load_svg</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;s</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Quit</attribute>"
    "          <attribute name='action'>pomelo.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label'>_View</attribute>"
    "      <item>"
    "        <attribute name='label'>_Orthonormal camera</attribute>"
    "        <attribute name='action'>pomelo.orthonormal_camera</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>_Reset 3D View</attribute>"
    "        <attribute name='action'>pomelo.reset_3d_view</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>Show mesh edges</attribute>"
    "        <attribute name='action'>pomelo.show_edge</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>Show mesh matcap</attribute>"
    "        <attribute name='action'>pomelo.show_matcap</attribute>"
    "      </item>"
    "      <submenu>"
    "        <attribute name='label'>_Levels</attribute>"
    "      <item>"
    "        <attribute name='label'>BaseLayer</attribute>"
    "        <attribute name='action'>pomelo.layer0</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>Layer 1</attribute>"
    "        <attribute name='action'>pomelo.layer1</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>Layer 2</attribute>"
    "        <attribute name='action'>pomelo.layer2</attribute>"
    "      </item>"
    "      </submenu>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label'>_Tools</attribute>"
    "      <item>"
    "        <attribute name='label'>_View Skeleton</attribute>"
    "        <attribute name='action'>pomelo.view_skeleton</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label'>Settings</attribute>"
    "        <attribute name='action'>pomelo.settings</attribute>"
    "      </item>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label'>_Help</attribute>"
    "      <item>"
    "        <attribute name='label'>_About</attribute>"
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
  m_main_input.signal_text_edited().connect( sigc::mem_fun(*this,
     &Pomelo::on_input_text_edited) );
  m_main_input.signal_profile_edited().connect( sigc::mem_fun(*this,
     &Pomelo::on_input_profile_edited) );

  w_vbox->pack_start(m_main_input, false, true);

  m_main_input.set_profile(m_pomelo_settings->get_string_default("profile"));

  w_vbox->pack_start(m_mesh_viewer, true, true);
  w_vbox->pack_start(m_statusbar, false, false);
  m_statusbar.push("Welcome to Pomelo");

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

  auto dialog = Gtk::FileChooserNative::create("STL filename",
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

    auto meshes = m_worker_skeleton.get_meshes();
    for (size_t i=0; i<meshes.size(); i++)
      {
        string mesh_fn = mesh_filename;

        if (meshes.size()>1)
          {
            size_t found = mesh_filename.rfind('.');
            if (found==std::string::npos)
              found = mesh_filename.size();
            mesh_fn = mesh_filename.substr(0,found) + format("-{:02d}", i) + mesh_filename.substr(found);
          }

       save_stl(meshes[i], mesh_fn);
       set_status(format("Saved mesh with {} vertices to {}",
                         meshes[i]->vertices.size(),
                         mesh_fn));
      }

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
void Pomelo::on_action_file_export_gltf()
{
  string mesh_filename;

  auto dialog = Gtk::FileChooserNative::create("GLTF filename",
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
    auto meshes = m_worker_skeleton.get_meshes();
    for (size_t i=0; i<meshes.size(); i++)
      {
        auto& mesh = meshes[i];
        auto msh_fn = format("mesh-{:02d}.gltf", i);
        // TBD - create a multi mesh gltf file
        save_gltf(meshes[i], mesh_filename);
        set_status(format("Saved mesh with {} vertices to {}",
                          mesh->vertices.size(),
                          mesh_filename));
      }

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled GLTF export");
    break;

  default:
    break;
  }
}

//Signal handlers:
void Pomelo::on_action_load_svg()
{
  m_svg_filename = "";
  auto dialog = Gtk::FileChooserNative::create("SVG filename",
                                               *this,
                                               Gtk::FILE_CHOOSER_ACTION_OPEN);
  
  if (m_last_selected_file.size())
    dialog->select_filename(m_last_selected_file);

  // Show the dialog and wait for a user response:
  const int result = dialog->run();

  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    m_svg_filename = dialog->get_filename();
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(m_svg_filename);
    string basename = file->get_basename();
    m_last_selected_file = m_svg_filename;
    
    set_status(format("Loading path from svg file {}",basename));
    m_main_input.set_text_edit_info_string(basename);
    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled svg import");
    break;

  default:
    break;
  }
}


//Signal handlers:
void Pomelo::on_action_file_quit()
{
  // hide(); // Close the main window to stop app->run(). Does not work!

  exit(0);
}

void Pomelo::on_action_help_about()
{
  Gtk::AboutDialog Dialog;
  Dialog.set_logo(Gdk::Pixbuf::create_from_resource("/about/pomelo_logo.png", -1, 80, true));

  Dialog.set_version(VERSION);
  Dialog.set_copyright("Dov Grobgeld <dov.grobgeld@gmail.com>");
  Dialog.set_comments(format(
                        "A program for generating 3D meshes of text\n\n"
                        "Commit-Id: {}\n"
                        "Commit-time: {}\n",
                        COMMIT_ID,
                        COMMIT_TIME));

  Glib::RefPtr<const Glib::Bytes> copying_bytes = Gio::Resource::lookup_data_global("/about/COPYING");

  gsize len=0;
  const char* copying_txt = (const char*)copying_bytes->get_data(len);

  Dialog.set_license(copying_txt);
  Dialog.set_license_type(Gtk::LICENSE_GPL_3_0);
  Dialog.set_program_name("Pomelo 3D");
  Dialog.set_website("http://github.com/dov/pomelo");
  Dialog.set_website_label("Pomelo 3D website");
  Dialog.set_transient_for(*this);

  std::vector<Glib::ustring> list_authors;
  list_authors.push_back("Dov Grobgeld");
  Dialog.set_authors(list_authors);
  Dialog.run();
}

void Pomelo::on_action_view_skeleton()
{
  m_skeleton_viewer->show();
}

void Pomelo::on_action_orthonormal()
{
  bool active = false;
  m_ref_orthonormal_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_orthonormal_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("orthonormal", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_orthonormal(active);
}

void Pomelo::on_action_show_edge()
{
  bool active = false;
  m_ref_show_edge_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_show_edge_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("show_edge", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_show_edge(active);
}

void Pomelo::on_action_show_matcap()
{
  bool active = false;
  m_ref_show_matcap_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_show_matcap_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("show_matcap", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_show_matcap(active);
}

void Pomelo::on_action_reset_3d_view()
{
  m_mesh_viewer.reset_view();
}

void Pomelo::on_action_view_settings()
{
  m_settings_dialog->show();
}

#if 0
void Pomelo::on_button_clicked()
{
  m_statusbar.push("That felt good!");

  Glib::signal_timeout()
    .connect_once([this]() { m_statusbar.remove_all_messages(); }, 1000);
}
#endif

void Pomelo::set_mesh(const string& mesh_filename)
{
  m_mesh_viewer.set_mesh_file(mesh_filename);
}

void Pomelo::set_debug_dir(const string& debug_dir)
{
  m_debug_dir = debug_dir;
  m_worker_skeleton.set_debug_dir(debug_dir);
  m_main_input.set_debug_dir(debug_dir);
}

void Pomelo::on_build_skeleton(Glib::ustring text_string,
                               double linear_limit,
                               Pango::FontDescription font_description
                               )
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
                                         text_string,
                                         m_svg_filename);
    });
  
}

void Pomelo::on_build_profile(bool use_profile_data,
                              double radius,
                              double round_max_angle,
                              int num_radius_steps,
                              double zdepth,
                              ProfileData profile_data)
{
  m_progress_dialog.set_title("Build Profile");
  m_progress_dialog.show();

  // Set parameters
  // Start a new worker thread.
  m_worker_action = ACTION_PROFILE;
  m_worker_skeleton_thread = make_unique<std::thread>(
    [=] {
      m_worker_skeleton.do_work_profile(use_profile_data,
                                        radius,
                                        round_max_angle,
                                        num_radius_steps,
                                        zdepth,
                                        profile_data);
    });

  m_main_input.set_profile_ready_state(true);
}

void Pomelo::on_input_text_edited()
{
  m_svg_filename = ""; 
}

void Pomelo::on_input_profile_edited()
{
  printf("on_input_profile_edited\n");
  m_pomelo_settings->set_string("profile",
                                m_main_input.get_profile_string());
  m_pomelo_settings->save();
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
              auto meshes = m_worker_skeleton.get_meshes();
              m_mesh_viewer.set_meshes(meshes);
              m_mesh_viewer.redraw();
            }

          // show the 2D skeleton result in the skeleton viewer
          if (m_skeleton_viewer)
            m_skeleton_viewer->set_giv_string(m_worker_skeleton.get_giv_string());
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

void Pomelo::on_action_view_layer0()
{
  bool active = false;
  m_ref_layer0_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_layer0_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("show_layer0", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_show_layer(0, active);
}

void Pomelo::on_action_view_layer1()
{
  bool active = false;
  m_ref_layer1_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_layer1_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("show_layer1", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_show_layer(1, active);
}

void Pomelo::on_action_view_layer2()
{
  bool active = false;
  m_ref_layer2_toggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_ref_layer2_toggle->change_state(active);

  // Store the new state
  m_pomelo_settings->set_int("show_layer2", int(active));
  m_pomelo_settings->save();

  m_mesh_viewer.set_show_layer(2, active);
}
