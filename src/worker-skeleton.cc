/* A worker doing the skeletonization. Based on the
 * gtkmm example worker. Original copyright below 
 *
 * gtkmm example Copyright (C) 2013 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "worker-skeleton.h"
#include "pomelo.h"
#include <sstream>
#include <chrono>
#include <fmt/core.h>
#include "smooth-sharp-angles.h"


using namespace std;
using namespace fmt;

const double MY_PI=3.141592653589793;
const double DEG2RAD=MY_PI/180;

// constructor
WorkerSkeleton::WorkerSkeleton(Pomelo *caller,
                               shared_ptr<PomeloSettings> pomelo_settings
                               ) :
  m_mutex(),
  m_shall_stop(false),
  m_has_stopped(false),
  m_fraction_done(0.0),
  m_message(),
  m_caller(caller),
  m_pomelo_settings(pomelo_settings)
{
  m_skeleton_updater = make_shared<SkeletonUpdater>(this);
  m_textrusion = make_shared<TeXtrusion>(m_skeleton_updater);
  m_mesh = make_shared<Mesh>();
  m_giv_string = make_shared<string>();
  m_mesh_giv_string = make_shared<string>();
}

// Get current porgress
void WorkerSkeleton::get_progress(// output
                                  double& fraction_done,
                                  Glib::ustring& message) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  fraction_done = m_fraction_done;
  message = m_message;
}

void WorkerSkeleton::stop_work()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_shall_stop = true;
}

bool WorkerSkeleton::has_stopped() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_has_stopped;
}

string WorkerSkeleton::get_error_message() 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto error_message = m_error_message;
  m_error_message="";
  return error_message;
}

void WorkerSkeleton::do_work_skeleton(
  bool do_rtl,
  Pango::FontDescription font_description,
  double linear_limit,
  Glib::ustring markup,
  Glib::ustring svg_filename)
{
  {
    std::lock_guard<mutex> lock(m_mutex);
    m_has_stopped = false;
    m_shall_stop = false;
    m_fraction_done = 0.0;
    m_message = "";
  } 

  m_textrusion->do_rtl = do_rtl;
  m_textrusion->font_description = font_description;
  m_textrusion->linear_limit = linear_limit;

  // Do the time consuming tasks
  bool finished_successfully = false;
  string error_message;
  string giv_string;

  try {
    Cairo::RefPtr<Cairo::Context> cr;
    if (svg_filename.size()>0)
      cr = m_textrusion->svg_filename_to_context(svg_filename);
    else
      cr = m_textrusion->markup_to_context(markup);
    auto polys = m_textrusion->cairo_path_to_polygons(cr);
    auto polys_with_holes = m_textrusion->polys_to_polys_with_holes(polys);

    if (m_pomelo_settings->get_int_default("smooth_sharp_angles",1))
      {
        double max_angle_to_smooth = m_pomelo_settings->get_double_default("smooth_max_angle", 135) * DEG2RAD;
        print("max_angle_to_smooth = {}\n", max_angle_to_smooth/DEG2RAD);
        polys_with_holes = smooth_acute_angles(0.1, max_angle_to_smooth, polys_with_holes, 16);
      }
    
    m_phole_infos = m_textrusion->skeletonize(polys_with_holes,
                                              // output
                                              giv_string);
    finished_successfully=true;
  }
  catch(EAborted&) {
  }
  catch(CGAL::Failure_exception&) {
    // We should still show errors!
    error_message = "Illegal geometry found! Try another font!";
  }


  {
    lock_guard<mutex> lock(m_mutex);
    m_shall_stop = false;
    m_has_stopped = true;
    m_finished_successfully = finished_successfully;
    m_error_message = error_message;
    *m_giv_string = giv_string;
  }

  m_caller->notify();
}

void WorkerSkeleton::do_work_profile(
  bool use_profile_data,
  double radius,
  double round_max_angle,
  int num_radius_steps,
  double zdepth,
  ProfileData profile_data
)
{
  {
    std::lock_guard<mutex> lock(m_mutex);
    m_has_stopped = false;
    m_shall_stop = false;
    m_fraction_done = 0.0;
    m_message = "";
  } 

  m_textrusion->use_profile_data = use_profile_data;
  m_textrusion->zdepth = zdepth; 
  m_textrusion->profile_radius = radius; 
  m_textrusion->profile_round_max_angle = round_max_angle; 
  m_textrusion->profile_num_radius_steps = num_radius_steps;
  m_textrusion->profile_data = profile_data;

  // Do the time consuming tasks
  bool finished_successfully = false;
  string error_message;
  Mesh mesh;
  string giv_string;
  try {
    mesh = m_textrusion->skeleton_to_mesh(m_phole_infos,
                                          // output
                                          giv_string);
    finished_successfully=true;
  }
  catch(EAborted&) {
  }
  catch(CGAL::Failure_exception&) {
    error_message = "Illegal geometry found! Try another font!";
  }

  {
    lock_guard<mutex> lock(m_mutex);
    m_shall_stop = false;
    m_has_stopped = true;
    m_finished_successfully = finished_successfully;
    m_error_message = error_message;
    *m_mesh = mesh;
    *m_mesh_giv_string = giv_string;

#if 0
    ofstream of("path.giv");
    of << *m_mesh_giv_string;
    of.close();
#endif
  }

  m_caller->notify();
}

void WorkerSkeleton::notify_context_and_progress(const std::string& context,
                                                 double progress)
{
  m_message = context;
  m_fraction_done = progress;
  m_caller->notify();
}

Updater::ContinueStatus SkeletonUpdater::info(const std::string& context, double progress)
{
  if (m_worker_skeleton->get_shall_stop())
    return Updater::UPDATER_ABORT;

  // TBD - Notify to
  m_worker_skeleton->notify_context_and_progress(context,progress);
  
  return Updater::UPDATER_OK;
}
