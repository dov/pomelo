//======================================================================
// The pomelo application file
//
// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2021-03-06 Sat
//----------------------------------------------------------------------
#include <filesystem>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/menubar.h>
#include <gtkmm/filechoosernative.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include "mesh-viewer.h"
#include "pomelo.h"
#include <iostream>
#include "dov-mm-macros.h"
#include <spdlog/spdlog.h>
#include "utils.h"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;


Pomelo::Pomelo(shared_ptr<PomeloSettings> pomelo_settings)
  : m_main_input(*this),
    m_mesh_viewer(pomelo_settings),
    m_progress_dialog(*this,""),
    m_worker_skeleton(this, pomelo_settings),
    m_pomelo_settings(pomelo_settings)
{
  spdlog::info("Creating pomelo");

  set_title("Pomelo");

  //set_icon(Gdk::Pixbuf::create_from_resource("/about/pomelo_logo.png", -1, 80, true));
  set_default_size(800, 1000);

  auto w_vbox = mmVBox;

  this->add(*w_vbox); 

  m_skeleton_viewer = Glib::RefPtr<SkeletonViewer>(new SkeletonViewer(*this));
  m_skeleton_viewer->signal_response().connect([this](int response_id) {
    m_skeleton_viewer->hide();
  });

  m_mesh_viewer.signal_fatal_error().connect(
    [this](const string& message) {
        Gtk::MessageDialog dialog(*this, "Fatal Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_secondary_text("A critical error occurred and the application cannot continue.\n\nDetails: " + message);
        spdlog::info("Running error message dialog");
        dialog.run();
    
        // 2. Close the window. Because this window was passed to app->run(),
        //    closing it will cause app->run() to return, terminating the application.
        spdlog::info("Closing the pomelo window");
        this->close();
    });
    
  m_settings_dialog = Glib::RefPtr<SettingsDialog>(new SettingsDialog(*this,
                                                                      m_pomelo_settings));
  m_settings_dialog->signal_response().connect([this](int response_id)
  {
    if (response_id==1) // OK - Should make this an enum
      {
        if (m_settings_dialog->skeleton_params_have_changed())
          m_main_input.set_skeleton_ready_state(false);

        m_settings_dialog->save_to_settings();
        m_pomelo_settings->save();
        m_mesh_viewer.refresh_from_settings();
      }
    else
      m_settings_dialog->load_from_settings();
      
    m_settings_dialog->hide();
  });

  //Define the actions:
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refActionGroup->add_action("open_project",
    sigc::mem_fun(*this, &Pomelo::on_action_open_project) );

  m_refActionGroup->add_action("save_project",
    sigc::mem_fun(*this, &Pomelo::on_action_save_project) );

  m_refActionGroup->add_action("save_as_project",
    sigc::mem_fun(*this, &Pomelo::on_action_save_as_project) );

  m_refActionGroup->add_action("import_profile",
    sigc::mem_fun(*this, &Pomelo::on_action_import_profile) );

  m_refActionGroup->add_action("export_profile",
    sigc::mem_fun(*this, &Pomelo::on_action_export_profile) );

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
    "          <attribute name='label'>_Open project</attribute>"
    "          <attribute name='action'>pomelo.open_project</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;o</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Save project</attribute>"
    "          <attribute name='action'>pomelo.save_project</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;s</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>Save project _As</attribute>"
    "          <attribute name='action'>pomelo.save_as_project</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;a</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Import Profile</attribute>"
    "          <attribute name='action'>pomelo.import_profile</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;p</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Export Profile</attribute>"
    "          <attribute name='action'>pomelo.export_profile</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;r</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Export STL</attribute>"
    "          <attribute name='action'>pomelo.export_stl</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;e</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Export GLTF</attribute>"
    "          <attribute name='action'>pomelo.export_gltf</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;g</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label'>_Load SVG</attribute>"
    "          <attribute name='action'>pomelo.load_svg</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;v</attribute>"
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
  spdlog::info("Done creating the pomelo interface");
}

Pomelo::~Pomelo()
{
  spdlog::info("Destroying pomelo");
}

// g++ still doesn't have std::ends_with...
static bool ends_with(const string& subject, const string& query)
{
  // Check length
  if (subject.length() < query.length())
    return false;

  return (subject.substr(subject.length() - query.length(),
                         query.length()) == query);
}

// g++ still doesn't have std::ends_with...
static string tolower(const string& subject)
{
  string ret;

  for (const auto& ch : subject)
    ret += tolower(ch);
  
  return ret;
}

//Signal handlers:
void Pomelo::on_action_open_project()
{
  auto dialog = Gtk::FileChooserNative::create("Pomelo project",
                                               *this,
                                               Gtk::FILE_CHOOSER_ACTION_OPEN);

  auto filter = Gtk::FileFilter::create();
  filter->add_pattern("*.po3d");
  dialog->add_filter(filter);
  if (m_last_save_as_filename.size())
    dialog->select_filename(m_last_save_as_filename);

  // Show the dialog and wait for a user response:
  const int result = dialog->run();

  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    Glib::ustring filename = dialog->get_filename();
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
    string basename = file->get_basename();
    m_last_save_as_filename = filename;
    m_last_selected_file = filename;

    set_status(fmt::format("Loading project from file {}",basename));
    load_project(filename);

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled open project");
    break;

  default:
    break;
  }

}

void Pomelo::on_action_save_project()
{
  if (m_last_save_as_filename.size())
    save_project(m_last_save_as_filename);
  else
    on_action_save_as_project();
}

void Pomelo::on_action_save_as_project()
{
  auto dialog = Gtk::FileChooserNative::create("Pomelo project",
                                               *this,
                                               Gtk::FILE_CHOOSER_ACTION_SAVE);
  
  if (m_last_selected_file.size())
    dialog->select_filename(m_last_selected_file);

  // Show the dialog and wait for a user response:
  const int result = dialog->run();

  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    string filename = dialog->get_filename();

    if (!ends_with(tolower(filename), ".po3d"))
      filename += ".po3d";

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
    string basename = file->get_basename();
    m_last_selected_file = filename;

    set_status(fmt::format("Saving project to file {}",basename));
    save_project(filename);

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    spdlog::info("Canceled save project");
    set_status("Canceled save project");
    break;

  default:
    break;
  }
}

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
    string filenames;
    for (size_t i=0; i<meshes->size(); i++)
      {
        string mesh_fn = mesh_filename;

        if (!ends_with(tolower(mesh_filename), ".stl"))
          mesh_filename += ".stl";

        if (meshes->size()>1)
          {
            size_t found = mesh_filename.rfind('.');
            if (found==std::string::npos)
              found = mesh_filename.size();
            mesh_fn = mesh_filename.substr(0,found) + fmt::format("-{:02d}", i) + mesh_filename.substr(found);
          }

       filenames += mesh_fn + " ";

       save_stl((*meshes)[i], mesh_fn);
       spdlog::info("Saved mesh with {} vertices to {}",
                    (*meshes)[i].vertices.size(),
                    mesh_fn);
      }
    set_status(fmt::format("Saved file(s): {}", filenames));
  

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled STL export");
    break;

  default:
    break;
  }
}

void Pomelo::on_action_import_profile()
{
  auto dialog = Gtk::FileChooserNative::create("Export profile",
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
    string filename = dialog->get_filename();
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
    string basename = file->get_basename();
    m_last_selected_file = filename;

    spdlog::info("Importing profile from {}", filename);
    set_status(fmt::format("Importing profile from file {}",basename));
    string profile_string;

    try {
      profile_string = load_string_from_file(filename);
    }
    catch(...)
    {
      spdlog::error("Failed importing profile from file {}",basename);
      set_status(fmt::format("Failed importing profile from file {}",basename));
      break;
    }

    try {
      m_main_input.set_profile(profile_string);
    }
    catch(...)
    {
      spdlog::error("Failed setting profile from file {}. json error?", filename);
    }
    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled import profile");
    break;

  default:
    break;
  }

}

void Pomelo::on_action_export_profile()
{
  string filename;

  auto dialog = Gtk::FileChooserNative::create("Profile name",
                                               *this,
                                               Gtk::FILE_CHOOSER_ACTION_SAVE);
  
  // Show the dialog and wait for a user response:
  const int result = dialog->run();

  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    filename = dialog->get_filename();
    if (!ends_with(tolower(filename), ".prof3d"))
      filename += ".prof3d";

    spdlog::info("Exporting profile to {}", filename);

    string profile = m_main_input.get_profile_string();

    try {
      string_to_file(profile, filename);
    }
    catch(...)
    {
      spdlog::error("Failed saving profile to {}", filename);
      set_status("Failed saving profile!");
      return;
    }

    set_status(fmt::format("Saved profile file to {}", filename));

    break;
  }

  case Gtk::RESPONSE_CANCEL:
    set_status("Canceled export profile");
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

  auto meshes = m_worker_skeleton.get_meshes();
  vector<glm::vec3> mesh_colors;

  for (size_t i=0; i<meshes->size(); i++)
  {
    Gdk::RGBA color;

    if (i==0)
      color = Gdk::RGBA(m_pomelo_settings->get_string_default("mesh_color",
                                                              "#c0c0c0"));
    else if (i==1)
      color = Gdk::RGBA(m_pomelo_settings->get_string_default("mesh_level1_color",
                                                              "#c0c0c0"));
    else if (i==2)
      color = Gdk::RGBA(m_pomelo_settings->get_string_default("mesh_level2_color",
                                                              "#c0c0c0"));
    mesh_colors.push_back(
      {color.get_red(),
       color.get_green(),
       color.get_blue()});
  }


  // Handle the response:
  switch (result)
  {
  case Gtk::RESPONSE_ACCEPT:
  {
    mesh_filename = dialog->get_filename();

    meshes->save_gltf(mesh_filename);
    set_status(fmt::format("Saved meshes to {}", mesh_filename));

#if 0
    for (size_t i=0; i<meshes.size(); i++)
      {
        auto& mesh = meshes[i];
        auto msh_fn = fmt::format("mesh-{:02d}.gltf", i);
        // TBD - create a multi mesh gltf file
      }

#endif
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
    
    set_status(fmt::format("Loading path from svg file {}",basename));
    m_main_input.set_text_edit_string(basename, true);
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
  // TBD - Check if the project has changed!
  Gtk::MessageDialog dialog(
    *static_cast<Gtk::Window*>(this->get_toplevel()),
    "Save project?",
    false, // use_markup
    Gtk::MESSAGE_INFO,
    Gtk::BUTTONS_NONE,
    true); // modal
  dialog.add_button("Yes", Gtk::RESPONSE_OK);
  dialog.add_button("No", Gtk::RESPONSE_NO);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  int res = dialog.run();
  
  // Handle the response:
  switch (res)
  {
  case Gtk::RESPONSE_OK:
  {
    on_action_save_project();
    exit(0);
    break;
  }

  case Gtk::RESPONSE_CANCEL:
    spdlog::info("Quit canceled");
    set_status("Canceled svg import");
    break;

  case Gtk::RESPONSE_NO:
    exit(0);
    break;
  default:
    break;
  }
}

void Pomelo::on_action_help_about()
{
  spdlog::info("on_action_help_about");
  Gtk::AboutDialog Dialog;
  Dialog.set_logo(Gdk::Pixbuf::create_from_resource("/about/pomelo_logo.png", -1, 80, true));

  Dialog.set_version(VERSION);
  Dialog.set_copyright("Dov Grobgeld <dov.grobgeld@gmail.com>");
  Dialog.set_comments(fmt::format(
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
  spdlog::info("on_action_view_skeleton");
  m_skeleton_viewer->show();
}

void Pomelo::on_action_orthonormal()
{
  spdlog::info("on_action_orthonormal");
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
  spdlog::info("on_action_show_edge");
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
  spdlog::info("on_action_show_matcap");
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

#if 0
void Pomelo::set_mesh(const string& mesh_filename)
{
  m_mesh_viewer.set_mesh_file(mesh_filename);
}
#endif

void Pomelo::set_debug_dir(const string& debug_dir)
{
  if (!fs::exists(debug_dir))
    fs::create_directory(debug_dir);

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
    [=,this] {
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
    [=, this] {
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
  spdlog::info("Set status: {}", message);
  m_statusbar.remove_all_messages();
  m_statusbar.push(message);
}

Updater::ContinueStatus PomeloUpdater::info(const std::string& context, double progress)
{
  auto lambda = [this,context,progress]()->bool {
    m_pomelo->set_status(context + fmt::format("{}", progress));
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
              m_mesh_viewer.refresh_from_settings(false);
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
  
  
      set_status(fmt::format("Progress {:.0f}%: {}", fraction_done*100, message.c_str()));
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

void Pomelo::load_project(const std::string& filename)
{
  spdlog::info("Loading project from {}", filename);

  if (!fs::exists(filename))
  {
    spdlog::error("Project file {} does not exist!", filename);
    return;
  }

  // Open filename as json file
  string js = load_string_from_file(filename);

  json j;

  try {
    j = json::parse(js);
  }
  catch(const json::parse_error& err)
  {
    spdlog::error("Failed parsing json from {}!", filename);
    fmt::print("Failed parsing json\n");
    return;
  }

  // Populate different fields
  m_main_input.set_text_edit_string(j.value("text", "Pomelo").c_str(),
                                    false);
#ifdef WIN32
  string default_font = "Arial Black Heavy 48";
#else
  string default_font = "Sans Bold 48";
#endif

  m_main_input.set_font_name(j.value("font_name", default_font).c_str());

  // Should the project contain the mesh and the skeleton? Currently not...

  // Rest of fields
  m_main_input.set_profile_option(j.value("profile_option", 0));
  m_main_input.set_round_profile(j.value("radius", 5),
                                 j.value("num_radius_steps", 10),
                                 j.value("round_max_angle", 90)
                                 );
  m_main_input.set_zdepth(j.value("zdepth", 5));

  // Curve profile
  string curve_profile = j.value("curve_profile", "");
  if (curve_profile.size())
    m_main_input.set_profile(curve_profile);

  // Settings
  
  bool ortho_camera = j.value("ortho_camera", false);
  m_ref_orthonormal_toggle->change_state(ortho_camera);
  m_mesh_viewer.set_orthonormal(ortho_camera);
  bool show_edges = j.value("show_edges", false);
  m_ref_show_edge_toggle->change_state(show_edges);
  m_mesh_viewer.set_show_edge(show_edges);
  bool show_matcap = j.value("show_matcap", false);
  m_ref_show_matcap_toggle->change_state(show_matcap);
  m_mesh_viewer.set_show_matcap(show_matcap);

  // Settings override
  vector<string> settings = {
    "background_color",
    "mesh_color",
    "mesh_level1_color",
    "mesh_level2_color"
  };
  for (int i=0; i<(int)settings.size(); i++)
  {
    string key = settings[i];
    if (!j.contains(key))
      continue;
    m_pomelo_settings->set_string(key, j.value(key,""));
  }
  m_mesh_viewer.refresh_from_settings(false);
  m_settings_dialog->load_from_settings();

  // visible layers
  vector<Glib::RefPtr<Gio::SimpleAction>> toggles = {
    m_ref_layer0_toggle,
    m_ref_layer1_toggle,
    m_ref_layer2_toggle };
  for (int i=0; i<3; i++)
  {
    string key = fmt::format("layer{}", i);
    if (!j.contains(key))
      continue;
    bool show_layer = j.value(key, false);
    toggles[i]->change_state(show_layer);
    m_mesh_viewer.set_show_layer(i, show_layer);
  }
  m_last_save_as_filename = filename;
}

void Pomelo::save_project(const string& filename)
{
  spdlog::info("Saving project to {}", filename);
  nlohmann::json j;
  j["text"] = m_main_input.get_text_edit_string();
  j["font_name"] = m_main_input.get_font_name();
  j["profile_option"] = m_main_input.get_profile_option();
  double radius, round_max_angle;
  int num_radius_steps;
  m_main_input.get_round_profile_params(radius,
                                        num_radius_steps,
                                        round_max_angle);
  j["radius"] = radius;
  j["num_radius_steps"] = num_radius_steps;
  j["round_max_angle"] = round_max_angle;
  j["zdepth"] = m_main_input.get_zdepth();
  j["curve_profile"] = m_main_input.get_profile_string().c_str();
  bool active;
  m_ref_orthonormal_toggle->get_state(active);
  j["ortho_camera" ] = active;
  m_ref_show_edge_toggle->get_state(active);
  j["show_edges" ] = active;
  m_ref_show_matcap_toggle->get_state(active);
  j["show_matcap" ] = active;
  
  // From settings override - This should be shared!
  vector<string> settings = {
    "background_color",
    "mesh_color",
    "mesh_level1_color",
    "mesh_level2_color"
  };
  for (int i=0; i<(int)settings.size(); i++)
  {
    string key = settings[i];
    j[key] = m_pomelo_settings->get_string_default(key,"");
  }

  try {
    string_to_file(j.dump(2), filename);
    spdlog::info("Successfully saved project to {}", filename);
    set_status(fmt::format("Successfully saved project to {}", filename));
  }
  catch(...)
  {
    Gtk::MessageDialog dialog(
      *static_cast<Gtk::Window*>(this->get_toplevel()),
      fmt::format("Failed saving project to <b>{}</b>", filename),
      true, // use_markup
      Gtk::MESSAGE_ERROR,
      Gtk::BUTTONS_OK,
      true); // modal
    dialog.run();
    
    spdlog::error("Failed saving project to {}!", filename);
    set_status(fmt::format("Failed saving project to {}!", filename));
    
    return;
  }
  m_last_save_as_filename = filename;
}
