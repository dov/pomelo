//======================================================================
// The pomelo cli application 
//
// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2021-03-06 Sat
//----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include "cairo-flatten-by-bitmap.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "textrusion.h"
#include "giv-debug-utils.h"
#include "utils.h"


using namespace std;
using fmt::print;

template <typename... Args>
static void die(fmt::format_string<Args...> FormatStr, Args &&... args)
{
  string msg = fmt::format(FormatStr, std::forward<Args>(args)...);
  if (msg[msg.size()-1] != '\n')
    msg += "\n";
  fmt::print(stderr, "{}", msg);
  exit(-1);
}

#define CASE(s) if (s == S_)

// task creating the mesh
void create_mesh(bool do_rtl,
                 string font_description_string,
                 double linear_limit,
                 string markup,
                 string debug_dir,
                 bool use_profile_data,
                 double zdepth,
                 double radius,
                 double round_max_angle,
                 int num_radius_steps,
                 const ProfileData& profile_data) 
{
  double resolution = 100; // will be automatically reduced if needed

  auto textrusion = make_shared<TeXtrusion>();
  textrusion->do_rtl = do_rtl;
  textrusion->font_description = Pango::FontDescription(font_description_string);
  textrusion->linear_limit = linear_limit;

  // tbd - support svg
  cairo_surface_t *rec_surface = cairo_recording_surface_create(
    CAIRO_CONTENT_ALPHA,
    nullptr // unlimited extens
  );

  Cairo::RefPtr<Cairo::Surface> surface = Cairo::RefPtr<Cairo::Surface>(new Cairo::Surface(rec_surface));
  textrusion->markup_to_context(surface, markup);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

  // Flatten by a bitmap and store the result in cr
  FlattenByBitmap fb(cr->cobj());
  fb.set_debug_dir(debug_dir);
  fb.flatten_by_bitmap(surface->cobj(),
                       resolution);
  
  auto polys = textrusion->cairo_path_to_polygons(cr);
  fmt::print("{} polys found\n", polys.size());
  auto polys_with_holes = textrusion->polys_to_polys_with_holes(polys);

  string giv_filename = fmt::format("{}/polys_with_holes.giv", debug_dir);
  string outer_header = fmt::format(
    "$color red\n"
    "$path orig/outer\n"
    "$marks fcircle\n");
  string hole_header = fmt::format(
    "$color green\n"
    "$path orig/hole\n"
    "$marks fcircle\n");

  if (debug_dir.size())
  {
    spdlog::info("Saving polys with holes to {}", giv_filename);
    polys_with_holes_to_giv(giv_filename,
                            outer_header,
                            hole_header,
                            polys_with_holes,
                            false);
  }

  // tbd - add support for smooth_sharp_angles

  if (debug_dir.size())
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
  
  string giv_string;
  auto phole_infos = textrusion->skeletonize(polys_with_holes,
                                               // output
                                               giv_string);
  if (debug_dir.size())
  {
    string giv_filename = fmt::format("{}/skeleton.giv", debug_dir);
    fmt::print("Saving to {}\n", giv_filename);
    spdlog::info("Saving to {}", giv_filename);
    string_to_file(giv_string, giv_filename);
  }

  // second stage of algo, turn into mesh
  textrusion->use_profile_data = use_profile_data;
  textrusion->zdepth = zdepth; 
  textrusion->profile_radius = radius; 
  textrusion->profile_round_max_angle = round_max_angle; 
  textrusion->profile_num_radius_steps = num_radius_steps;
  textrusion->profile_data = profile_data;

  // Do the time consuming tasks
  auto meshes = textrusion->skeleton_to_mesh(phole_infos,
                                             // output
                                             giv_string);

  if (debug_dir.size())
  {
    string giv_filename = fmt::format("{}/mesh_giv_file.giv", debug_dir);
    spdlog::info("Saved {}", giv_filename);
    string_to_file(giv_string, giv_filename);

    save_stl(meshes[0], fmt::format("{}/mesh.stl", debug_dir));
  }

}

int main(int argc, char **argv)
{
  int argp = 1;
  string mesh_filename;
  string config_dir;
  string log_filename;
  string debug_dir;
  vector<string> args;
  bool do_log_stdout = false;
  bool do_log_and_exit = false;
  double do_rtl = false;
  string font_description = "Sans 24";
  double linear_limit = 500;
  string markup = ".";
  int num_radius_steps = 5;

  for (int i=0; i<argc; i++)
    args.push_back(argv[i]);

  while(argp < argc && argv[argp][0] == '-')
  {
    const string& S_ = argv[argp++];

    CASE("--help")
    {
      fmt::print("pomelo-cli - A cli interface to pomelo\n\n"
                 "Syntax:\n"
                 "    pomelo-cli [] markup\n"
                 "\n"
                 "Options:\n"
                 "   --log-file log_file    Log pomelo debug to the given log file\n"
                 "   --debug_dir debug_dir  Set debug dir for temporary files\n"
                 "   --log-stdout           Log to stdout\n"
                 "   --num-radius-steps n   Set number radius steps\n"
                 );
      exit(0);
    }
    CASE("--log-file")
    {
      log_filename = argv[argp++];
      continue;
    }
    CASE("--debug_dir")
    {
      debug_dir = argv[argp++];
      continue;
    }
    CASE("--log-stdout")
    {
      do_log_stdout = true;
      continue;
    }
    CASE("--num-radius-steps")
    {
      num_radius_steps = atoi(argv[argp++]);
      continue;
    }
    die("Unknown option {}!", S_);
  }

  // Setup the logger
#if _WIN32
  auto color_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else
  auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif

  vector<spdlog::sink_ptr> log_sinks;
  if (log_filename.size())
  {
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, 1024*1024*10, 3);
    log_sinks.push_back(rotating_sink);

  }
  if (do_log_stdout)
    log_sinks.push_back(color_sink); 

  auto logger = std::make_shared<spdlog::logger>("pomelo logger", log_sinks.begin(), log_sinks.end());
  spdlog::set_default_logger(logger);
  logger->set_pattern("[%H:%M:%S.%e] [%l] %v");
  logger->flush_on(spdlog::level::info); 

  spdlog::info("======================================================");
  spdlog::info("Starting Pomelo on {}",
               fmt::format("{:%Y-%m-%d %H:%M:%S %z}",
                           std::chrono::system_clock::now()));
  spdlog::info("CommitID: {}", COMMIT_ID);
  spdlog::info("CommitTime: {}", COMMIT_TIME);
  spdlog::info("Version: {}", VERSION);
  spdlog::info("Command line: {}", fmt::format("{}", fmt::join(args," ")));
  if (do_log_and_exit)
  {
    spdlog::info("exiting");
    exit(0);
  }

  bool use_profile_data = false;
  double zdepth = 10;
  double radius = 3;
  double round_max_angle = 1;
  ProfileData profile_data;

  create_mesh(do_rtl,
              font_description,
              linear_limit,
              markup,
              debug_dir,
              use_profile_data,
              zdepth,
              radius,
              round_max_angle,
              num_radius_steps,
              profile_data);

  exit(0);
  return(0);
}
