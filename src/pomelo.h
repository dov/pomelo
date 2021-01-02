//======================================================================
//  pomelo.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Dec  6 22:36:33 2020
//----------------------------------------------------------------------
#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

#include <gtkmm.h>
#include "main-input.h"
#include "mesh-viewer.h"
#include "pangocairo-to-contour.h"
#include "worker-skeleton.h"
#include "progress-dialog.h"

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
  Pomelo();
  virtual ~Pomelo();

  void set_mesh(const std::string& mesh_filename);
  void set_status(const std::string& message);

  // Called from the worker thread.
  void notify();

private:
  // What is the worker doing? Should we ask this from the worker?
  typedef enum {
    ACTION_SKELETON,
    ACTION_PROFILE
  } WorkerAction;

  //Signal handlers:
  void on_button_clicked();

  //Signal handlers:
  void on_action_file_quit();
  void on_action_file_export_stl();
  void on_action_help_about();

  void on_build_skeleton(Glib::ustring text_string,
                         double linear_limit,
                         Pango::FontDescription font_description);
  void on_build_profile(double radius,
                        int num_radius_steps,
                        double zdepth);

  // Dispatcher handler.
  void on_notification_from_skeleton_worker_thread();

  //Member widgets:
  MainInput m_main_input;
  MeshViewer m_mesh_viewer;
  ProgressDialog m_progress_dialog;

  Gtk::Statusbar m_statusbar;
  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
  std::shared_ptr<PomeloUpdater> m_updator;
  Glib::Dispatcher m_dispatcher;
  WorkerSkeleton m_worker_skeleton;
  std::unique_ptr<std::thread> m_worker_skeleton_thread;
  WorkerAction m_worker_action;  // What is our worker doing
};

#endif /* HELLO-WORLD */
