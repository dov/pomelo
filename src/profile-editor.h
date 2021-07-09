//======================================================================
//  profile-editing.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Jun 23 19:52:41 2021
//----------------------------------------------------------------------
#ifndef PROFILE_EDITING_H
#define PROFILE_EDITING_H

#include <gtkmm.h>
#include <goocanvasmm.h>

enum NodeType {
  NODE_CORNER,
  NODE_CURVE,
  NODE_CURVE_SYMMETRIC,
  NODE_END
};

class ProfileEditor;
class Layer;

struct Vec2 : std::array<double,2> {
  Vec2(double x=0, double y=0) {
    (*this)[0] = x;
    (*this)[1] = y;
  }
  double& x() {
    return (*this)[0];
  }
  double& y() {
    return (*this)[1];
  }

  Vec2 operator+(Vec2 b)
  {
    return { this->x()+b.x(),this->y()+b.y() };
  }
  Vec2 operator-(Vec2 b)
  {
    return { this->x()-b.x(),this->y()-b.y() };
  }
};

class Node {
  public:
  Node(ProfileEditor *pe,
       Layer *layer,
       NodeType type, double x, double y)
    : type(type),
      xy(x,y),
      pe(pe),
      layer(layer) {}
  NodeType type = NODE_CORNER;
  bool selected = false;
  Vec2 dxym,xy,dxyp; // Distance to control points and coordinate

  // The following are used for moving nodes. This can really be made
  // global as only one node can be dragged at a time.
  bool dragging = false;
  Vec2 drag_xy;

  // The group containing the item and the controls
  Glib::RefPtr<Goocanvas::Group> item_group;

  // Only one of these will be used dependending on the type. 
  Glib::RefPtr<Goocanvas::Polyline> item_poly;
  Glib::RefPtr<Goocanvas::Ellipse> item_circle;
  Glib::RefPtr<Goocanvas::Item> item;

  // Controls for bezier curves for drawing the cxm,cym and cxp,cyp
  Glib::RefPtr<Goocanvas::Group> item_control_group;
  Glib::RefPtr<Goocanvas::Ellipse> item_control_minus;
  Glib::RefPtr<Goocanvas::Ellipse> item_control_plus;
  Glib::RefPtr<Goocanvas::Polyline> item_control_line;
  
  ProfileEditor *pe;
  Layer *layer;

  bool on_button_press(const Glib::RefPtr<Goocanvas::Item>& item,
                       GdkEventButton* event);
  bool on_button_release(const Glib::RefPtr<Goocanvas::Item>& item,
                         GdkEventButton* event);
  bool on_motion_notify(const Glib::RefPtr<Goocanvas::Item>& item,
                        GdkEventMotion* event);

  void translate(double px, double py);
  void move_control_point(int control_point, double dpx, double dpy);

  // Show the node control points
  void set_show_control(bool show_control);
  void auto_show_control();
};

class Layer : public std::vector<Node> {
  public:
  Glib::RefPtr<Goocanvas::Group> item_group; // a group for the layer
  Glib::RefPtr<Goocanvas::Path> item_path; // a group for the layer
  ProfileEditor *pe;

  // Move all the selected nodes by the given amount
  void move_selected(double dpx, double dpy);

  // Insert a node between all pairs of selected nodes
  void insert_node();

  // Delete all selected nodes except the first and last that cannot
  // be deleted.
  void remove_selected_nodes();

  // Make selected nodes into corner nodes
  void corner_selected_nodes();

  // Make selected nodes into round
  void round_selected_nodes();

  // Make selected nodes into round
  void round_selected_symmetric_nodes();
};

class ProfileEditor : public Gtk::Box
{
  public:
  ProfileEditor();

  // Used by the "friends" in the layers
  void clear_all_selected(Layer *except_layer=nullptr);
  void populate_canvas_items();
  void draw_layers();

  // Convert from profile coordinates to canvas coordinates
  void profile_coord_to_canvas_coord(double px, double py,
                                       // output
                                       double &cx,double &cy);

  void canvas_delta_to_profile_delta(double dcx, double dcy,
                                     // output
                                     double &dpx,double &dpy);

  void profile_delta_to_canvas_delta(double dpx, double dpy,
                                     // output
                                     double &dcx,double &dcy);

  private:
  // A canvas item for the layers graphs
  Glib::RefPtr< Goocanvas::Group > m_layers_group;
  void on_add_node_clicked();
  void on_remove_node_clicked();
  void on_corner_node_clicked();
  void on_round_node_clicked();
  void on_round_symmetric_node_clicked();

  std::vector<Layer> m_layers;
  double m_xmargin=50;
  double m_graph_xmargin=40;
  double m_graph_ymargin=40;
  double m_ymargin=50;
  double m_canvas_width=800;
  double m_canvas_height=500;

  double m_profile_maxx = 100;
  double m_profile_maxy = 50;
};

#endif /* PROFILE-EDITING */
