//======================================================================
//   main for Pomelo.
//
//   2023-02-25 Sat
//   Dov Grobgeld <dov.grobgeld@gmail.com>
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "pomelo.h"
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include "textrusion.h"
#include "pomelo-settings.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>


using namespace std;
namespace fs = std::filesystem;

static void die(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt); 
    
  spdlog::error("Dieing: {}", fmt); // tbd format this properly!
  vfprintf(stderr, fmt, ap);
  exit(-1);
}


#define CASE(s) if (s == S_)

int main(int argc, char *argv[])
{
  int argp = 1;
  string mesh_filename;
  string config_dir;
  string log_filename;
  string debug_dir;
  vector<string> args;
  bool do_log_stdout = false;
  bool do_log_and_exit = false;

  for (int i=0; i<argc; i++)
    args.push_back(argv[i]);

  while(argp < argc && argv[argp][0]=='-')
  {
    string S_ = argv[argp++];

    CASE("--help")
    {
      fmt::print("pomelo - A 3D text generator\n"
                 "\n"
                 "Syntax:\n"
                 "   pomelo [--mesh m]\n"
                 "\n"
                 "Options:\n"
                 "   --mesh m    Mesh for   testing\n"
                 "   --log-file log_file    Log pomelo debug to the given log file\n"
                 "   --debug_dir debug_dir  Set debug dir for temporary files\n"
                 "   --log-stdout           Log to stdout\n"
                 );
      do_log_and_exit=true;
      break;
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
    die("Unknown option: %s\n", S_.c_str());
  }

  // If we have another argument treat it as a project and load it
  string project_filename;
  if (argp < argc)
    project_filename = argv[argp++];
    
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

  if (!do_log_and_exit)
  {
    auto app = Gtk::Application::create(); // argc, argv, "org.dov.pomelo");
    auto pomelo_settings = make_shared<PomeloSettings>();
    Pomelo pomelo(pomelo_settings);
    if (debug_dir.size())
      pomelo.set_debug_dir(debug_dir);

    if (project_filename.size())
      pomelo.load_project(project_filename);
        
    return app->run(pomelo);
  }
  spdlog::info("Exited application");
}

