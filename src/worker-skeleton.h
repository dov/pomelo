//======================================================================
//  worker-skeleton.h - A worker thread doing the skeletonization
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jan  1 07:12:33 2021
//----------------------------------------------------------------------
#ifndef WORKER_SKELETON_H
#define WORKER_SKELETON_H

#include <gtkmm.h>
#include <thread>
#include <mutex>
#include "textrusion.h"
#include "mesh.h"
#include "pomelo-settings.h"

class Pomelo;
class WorkerSkeleton;

class SkeletonUpdater : public Updater
{
  public:
  SkeletonUpdater(WorkerSkeleton *worker_skeleton) : m_worker_skeleton(worker_skeleton)
  {}
  virtual ~SkeletonUpdater() {}
    
  // update_progress returns the progress. If the function returns
  // UPDATER_ABORT then the underlying process should abort.
  ContinueStatus info(const std::string& context, double progress) override;

  WorkerSkeleton *m_worker_skeleton;    
};
  
class WorkerSkeleton
{
public:
  WorkerSkeleton(Pomelo* caller,
                 std::shared_ptr<PomeloSettings> pomelo_settings);

  // Thread functions

  // Do the skeleton part of the shaping
  void do_work_skeleton(bool do_rtl,
                        Pango::FontDescription font_description,
                        double linear_limit,
                        Glib::ustring markup,
                        Glib::ustring svg_filename);

  // Do the profile part of the shaping. The interface allows
  // using one out of two method, either by radius or by the profile
  // data in which case the radius isn't used.
  void do_work_profile(bool use_profile_data,
                       double radius,
                       double round_max_angle,
                       int num_radius_steps,
                       double zdepth,
                       ProfileData profile_data);

  // Get update on progress
  void get_progress(// output
                    double& fraction_done,
                    Glib::ustring& message) const;
  // Request to stop
  void stop_work();

  // Are we finished?
  bool has_stopped() const;

  // Did we finish due to an error? This resets the error message if there
  // is one.
  std::string get_error_message();

  // Used by the updater to inform the parent thread
  void notify_context_and_progress(const std::string& context,
                                   double progress);

  bool get_shall_stop() { return m_shall_stop; };
  bool get_finished_successfully() { return m_finished_successfully; };
  std::vector<std::shared_ptr<Mesh>> get_meshes() {
    return m_meshes;
  }
  std::shared_ptr<std::string> get_giv_string() {
    return m_giv_string;
  }

  void set_debug_dir(const std::string& debug_dir);

private:
  // Synchronizes access to member data.
  mutable std::mutex m_mutex;

  // Data used by both GUI thread and worker thread.
  bool m_shall_stop { false };
  bool m_has_stopped { true };
  bool m_finished_successfully { false };
  double m_fraction_done { 0 };
  Glib::ustring m_message;
  std::shared_ptr<SkeletonUpdater> m_skeleton_updater;
  std::vector<PHoleInfo> m_phole_infos;
  std::vector<std::shared_ptr<Mesh>> m_meshes; // The result of the work
  std::shared_ptr<std::string> m_giv_string; // The result of the work
  std::shared_ptr<std::string> m_mesh_giv_string; // The result of the work
  std::shared_ptr<TeXtrusion> m_textrusion;
  std::string m_error_message;
  Pomelo* m_caller; // Used for notifications
  std::shared_ptr<PomeloSettings> m_pomelo_settings;
  std::string m_debug_dir;
};



#endif /* WORKER-SKELETON */
