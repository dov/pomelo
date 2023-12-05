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
#include "cairo-flatten-by-bitmap.h"
#include <spdlog/spdlog.h>
#include "utils.h"


using namespace std;

const double MY_PI=3.141592653589793;
const double DEG2RAD=MY_PI/180;

// These should be in a utility library
static void poly_to_giv(const string& filename,
                        const string& header,
                        const Polygon_2& poly,
                        bool append)
{
    auto flags = ofstream::out;
    if (append)
        flags |= ofstream::app;
    ofstream of(filename, flags);
    of << header;
    of << "m ";
    for (const auto& p  : poly)
        of << p.x() << " " << p.y() << "\n";
    of << "z\n\n";
    of.close();
}


static void polys_with_holes_to_giv(const string& filename,
                                    const string& outer_header,
                                    const string& hole_header,
                                    std::vector<Polygon_with_holes>& polys,
                                    bool append)
{
  for (const auto &ph : polys)
  {
    poly_to_giv(filename,
                outer_header,
                ph.outer_boundary(),
                append);
    append = true;
    for (const auto &h : ph.holes())
      poly_to_giv(filename,
                  hole_header,
                  h,
                  append);
      
  }
}

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
  spdlog::info("Creating the WorkerSkeleton");

  m_skeleton_updater = make_shared<SkeletonUpdater>(this);
  m_textrusion = make_shared<TeXtrusion>(m_skeleton_updater);
  if (m_meshes)
    m_meshes->clear();
  m_giv_string = make_shared<string>();
  m_mesh_giv_string = make_shared<string>();

  spdlog::info("Done creating the WorkerSkeleton");
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
  double resolution = 100; // will be automatically reduced if needed

  try {
    cairo_surface_t *rec_surface = cairo_recording_surface_create(
      CAIRO_CONTENT_ALPHA,
      nullptr // unlimited extens
    );
  
    Cairo::RefPtr<Cairo::Surface> surface = Cairo::RefPtr<Cairo::Surface>(new Cairo::Surface(rec_surface));
    // Cairo::RefPtr<Cairo::Surface> surface = Cairo::RecordingSurface::create();
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
    if (svg_filename.size()>0)
      m_textrusion->svg_filename_to_context(surface, svg_filename);
    else
      m_textrusion->markup_to_context(surface, markup);

    // Flatten by a bitmap and store the result in cr
    FlattenByBitmap fb(cr->cobj());
    fb.set_debug_dir(m_debug_dir);
    fb.flatten_by_bitmap(surface->cobj(),
                         resolution);
    
    auto polys = m_textrusion->cairo_path_to_polygons(cr);
    fmt::print("{} polys found\n", polys.size());
    auto polys_with_holes = m_textrusion->polys_to_polys_with_holes(polys);

    string giv_filename = fmt::format("{}/polys_with_holes.giv", m_debug_dir);
    string outer_header = fmt::format(
      "$color red\n"
      "$path orig/outer\n"
      "$marks fcircle\n");
    string hole_header = fmt::format(
      "$color green\n"
      "$path orig/hole\n"
      "$marks fcircle\n");

    if (m_debug_dir.size())
    {
      spdlog::info("Saving polys with holes to {}", giv_filename);
      polys_with_holes_to_giv(giv_filename,
                              outer_header,
                              hole_header,
                              polys_with_holes,
                              false);
    }

    if (m_pomelo_settings->get_int_default("smooth_sharp_angles",1))
      {
        double max_angle_to_smooth = m_pomelo_settings->get_double_default("smooth_max_angle", 160) * DEG2RAD;
        fmt::print("max_angle_to_smooth = {}\n", max_angle_to_smooth/DEG2RAD);
        polys_with_holes = smooth_acute_angles(0.5, max_angle_to_smooth, polys_with_holes, 16);
      }

    if (m_debug_dir.size())
    {
      spdlog::info("Saving smooth polys with holes to {}", giv_filename);

      outer_header = fmt::format(
        "$color purple\n"
        "$path smooth/outer\n"
        "$marks fcircle\n");
      hole_header = fmt::format(
        "$color seagreen\n"
        "$path smooth/hole\n"
        "$marks fcircle\n");


      polys_with_holes_to_giv(giv_filename,
                              outer_header,
                              hole_header,
                              polys_with_holes,
                              true);
    }


    m_phole_infos = m_textrusion->skeletonize(polys_with_holes,
                                              // output
                                              giv_string);
    if (m_debug_dir.size())
    {
      string giv_filename = fmt::format("{}/skeleton.giv", m_debug_dir);
      fmt::print("Saving to {}\n", giv_filename);
      spdlog::info("Saving to {}", giv_filename);
      string_to_file(giv_string, giv_filename);
    }

    finished_successfully=true;
  }
  catch(EAborted&) {
  }
  catch(const CGAL::Failure_exception& exc) {
    // We should still show errors!
    error_message = fmt::format("CGAL failure: {}", exc.message());
  }
  catch(const std::runtime_error& exc) {
    // We should still show errors!
    error_message = exc.what();
  }

  if (error_message.size())
    spdlog::error("Failed skeleton to mesh: {}", error_message);

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
  spdlog::info("do_work_profile use_profile_data={} radius={} round_max_angle={} num_radius_steps={} zdepth={}", use_profile_data, radius, round_max_angle, num_radius_steps, zdepth);

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
  MultiMesh meshes;
  string giv_string;
  try {
    meshes = m_textrusion->skeleton_to_mesh(m_phole_infos,
                                            // output
                                            giv_string);
    finished_successfully=true;
  }
  catch(const EAborted&) {
  }
  catch(const CGAL::Failure_exception& exc) {
    error_message = fmt::format("CGAL failure: {}", exc.message());
  }
  catch(const std::runtime_error& exc)
  {
    error_message = exc.what();
  }
  if (error_message.size())
    spdlog::error("Failed skeleton to mesh: {}", error_message);

  {
    lock_guard<mutex> lock(m_mutex);
    m_shall_stop = false;
    m_has_stopped = true;
    m_finished_successfully = finished_successfully;
    m_error_message = error_message;
    m_meshes = make_shared<MultiMesh>(meshes);

    *m_mesh_giv_string = giv_string;

    if (m_debug_dir.size())
    {
      string giv_filename = fmt::format("{}/mesh_giv_file.giv", m_debug_dir);
      spdlog::info("Saved {}", giv_filename);
      string_to_file(giv_string, giv_filename);
    }
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

void WorkerSkeleton::set_debug_dir(const std::string& debug_dir)
{
  m_debug_dir = debug_dir;
  m_textrusion->set_debug_dir(debug_dir);
}
