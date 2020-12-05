#include "hello-world.h"

int main(int argc, char *argv[])
{
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.hello_world");

  HelloWorld helloworld;

  return app->run(helloworld);
}

