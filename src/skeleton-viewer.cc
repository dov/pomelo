// This is giv viewer for the skeleton

#include <gtkmm/scrolledwindow.h>
#include "dov-mm-macros.h"
#include "skeleton-viewer.h"

using namespace std;

SkeletonViewer::SkeletonViewer(Gtk::Window& parent)
    : Gtk::Dialog("Skeleton Viewer", parent)
{
    set_default_size(800, 600);

    auto w_scrolled_win = Gtk::make_managed<Gtk::ScrolledWindow>();
    w_giv_widget = giv_widget_new(NULL);
    giv_widget_add_giv_from_string(GIV_WIDGET(w_giv_widget),
                                   "$color red\n"
                                   "m 0 0\n"
                                   "100 0\n"
                                   "100 100\n"
                                   "0 100\n"
                                   "z\n"
                                   "\n"
                                   "$color black\n"
                                   "T5 50 50 Skeleton Viewer\n"
                                   );
    w_scrolled_win->add(*Glib::wrap(w_giv_widget));    
    get_content_area()->pack_start(*w_scrolled_win);

    get_content_area()->show_all();

    add_button("Close", 0);
}

void SkeletonViewer::set_giv_string(std::shared_ptr<std::string> giv_string)
{
  giv_widget_clear_giv(GIV_WIDGET(w_giv_widget));
  giv_widget_add_giv_from_string(GIV_WIDGET(w_giv_widget),
                                 giv_string->c_str());
  gtk_image_viewer_redraw(GTK_IMAGE_VIEWER(w_giv_widget),TRUE);
}

