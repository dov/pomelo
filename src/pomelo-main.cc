#include "pomelo.h"
#include "fmt/core.h"
#include "pangocairo-to-contour.h"

using namespace std;
using namespace fmt;

int main(int argc, char *argv[])
{
  int argp = 1;
  string mesh_filename;

  while(argp < argc && argv[argp][0]=='-')
    {
      string S_ = argv[argp++];

      if (S_ == "--help")
        {
          print("pamelo - A 3D text generator\n"
                "\n"
                "Syntax:\n"
                "   pomelo [--mesh m]\n"
                "\n"
                "Options:\n"
                "   --mesh m   Mesh for testing\n"
                );
          exit(0);
        }
      if (S_ == "--mesh")
        {
          mesh_filename = argv[argp++];
          continue;
        }
    }

  auto app = Gtk::Application::create(); // argc, argv, "org.dov.pomelo");
  Pomelo pomelo;
  if (mesh_filename.size())
    pomelo.set_mesh(mesh_filename);

  return app->run(pomelo);
}

