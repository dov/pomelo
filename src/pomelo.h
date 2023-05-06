//======================================================================
//  pomelo.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Dec  6 22:36:33 2020
//----------------------------------------------------------------------
#ifndef POMELO_H
#define POMELO_H

#include <gtkmm/statusbar.h>
#include <gtkmm/builder.h>
#include <giomm/simpleactiongroup.h>
#include "main-input.h"
#include "mesh-viewer.h"
#include "textrusion.h"
#include "worker-skeleton.h"
#include "progress-dialog.h"
#include "skeleton-viewer.h"
#include "settings-dialog.h"
#include "profile.h"

class Pomelo;

class PomeloUpdater : public Updater
{
  public:
  PomeloUpdater(Pomelo *pomelo) : m_pomelo(pomelo)
  {}
  virtual ~PomeloUpdater() {}
    
  // update_progress returns the progress. If the function returns
  // UPDATER_ABORT then the underlying process should abort.
  ContinueStatus info(const std::string& context, double progress) override;

  Pomelo *m_pomelo;    
};

class Pomelo : public Gtk::Window
{

public:
  Pomelo(std::shared_ptr<PomeloSettings> pomelo_settings);
  virtual ~Pomelo();

#if 0
  void set_mesh(const std::string& mesh_filename);
#endif
  void set_status(const std::string& message);
  void set_debug_dir(const std::string& debug_dir);

  // Called from the worker thread.
  void notify();
  void load_project(const std::string& filename);
  void save_project(const std::string& filename);

private:
  // What is the worker doing? Should we ask this from the worker?
  typedef enum {
    ACTION_SKELETON,
    ACTION_PROFILE
  } WorkerAction;

  //Signal handlers:
  void on_button_clicked();
  void on_action_open_project();
  void on_action_save_project();
  void on_action_save_as_project();
  void on_action_import_profile();
  void on_action_export_profile();
  void on_action_file_quit();
  void on_action_file_export_stl();
  void on_action_file_export_gltf();
  void on_action_help_about();
  void on_action_view_skeleton();
  void on_action_orthonormal();
  void on_action_show_edge();
  void on_action_show_matcap();
  void on_action_reset_3d_view();
  void on_action_view_settings();
  void on_action_load_svg();
  void on_action_view_layer0();
  void on_action_view_layer1();
  void on_action_view_layer2();

  // If the svg_filename is non-nil, use the svg filename,
  // otherwise use the text_string with the given font
  // decription
  void on_build_skeleton(Glib::ustring text_string,
                         double linear_limit,
                         Pango::FontDescription font_description);
  void on_build_profile(bool use_profile_data,
                        double radius,
                        double round_max_angle,
                        int num_radius_steps,
                        double zdepth,
                        ProfileData profile_data);

  // Info that the text was edited. This indicates that the svg
  // file is invalid.
  void on_input_text_edited();

  // When the profile was changed
  void on_input_profile_edited();

  // Dispatcher handler.
  void on_notification_from_skeleton_worker_thread();


  //Member widgets:
  MainInput m_main_input;
  MeshViewer m_mesh_viewer;
  ProgressDialog m_progress_dialog;
  Glib::RefPtr<SkeletonViewer> m_skeleton_viewer;
  Glib::RefPtr<SettingsDialog> m_settings_dialog;

  Gtk::Statusbar m_statusbar;
  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
  std::shared_ptr<PomeloUpdater> m_updator;
  Glib::Dispatcher m_dispatcher;
  Glib::ustring m_last_selected_file;
  Glib::ustring m_last_save_as_filename;
  Glib::ustring m_svg_filename;
  WorkerSkeleton m_worker_skeleton;
  bool project_has_changed = false;
  std::unique_ptr<std::thread> m_worker_skeleton_thread;
  WorkerAction m_worker_action;  // What is our worker doing
  std::shared_ptr<PomeloSettings> m_pomelo_settings;
  Glib::RefPtr<Gio::SimpleAction> m_ref_show_edge_toggle;
  Glib::RefPtr<Gio::SimpleAction> m_ref_show_matcap_toggle;
  Glib::RefPtr<Gio::SimpleAction> m_ref_orthonormal_toggle;
  Glib::RefPtr<Gio::SimpleAction> m_ref_layer0_toggle;
  Glib::RefPtr<Gio::SimpleAction> m_ref_layer1_toggle;
  Glib::RefPtr<Gio::SimpleAction> m_ref_layer2_toggle;
  std::string m_debug_dir;
};

#endif /* HELLO-WORLD */
