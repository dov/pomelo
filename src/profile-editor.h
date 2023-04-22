//======================================================================
//  profile-editing.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Jun 23 19:52:41 2021
//----------------------------------------------------------------------
#ifndef PROFILE_EDITING_H
#define PROFILE_EDITING_H

#include <gtkmm/dialog.h>
#include <goocanvasmm.h>
#include "profile.h"

enum NodeType {
  NODE_CORNER,
  NODE_CURVE,
  NODE_CURVE_SYMMETRIC,
  NODE_END
};

class ProfileEditor;
class Layer;

class Node {
  public:
  Node(ProfileEditor *pe,
       Layer *layer,
       NodeType type, double x=0, double y=0)
    : type(type),
      xy(x,y),
      pe(pe),
      layer(layer) {}
  NodeType type = NODE_CORNER;
  bool selected = false;
  Vec2 dxym {0.,0.},xy {0.,0.},dxyp{0.,0.}; // Distance to control points and coordinate

  // The following are used for moving nodes. This can really be made
  // global as only one node can be dragged at a time.
  bool dragging = false;
  Vec2 drag_xy { 0,0 };

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
  
  ProfileEditor *pe = nullptr;
  Layer* layer = nullptr; 

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

  // Whether the node is positive directional
  bool is_positive_directional(const Node* prev_node) const;
};

class Layer : public std::vector<Node> {
  public:
  // constructor
  Layer();
  ~Layer();
  Glib::RefPtr<Goocanvas::Group> item_group; // a group for the layer
  Glib::RefPtr<Goocanvas::Path> item_path; // a group for the layer
  ProfileEditor *pe=nullptr; // back reference 
  bool is_baselayer = true;  // The first layer is a base layer it is
                           // different in that the first and the last nodes
                           // can't be moved.

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
  void clear_all_selected(Layer* except_layer=nullptr);
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

  // Get the current edited profile as an exported profile data
  ProfileData get_profile();
  
  // Set the profile from the external data
  void set_profile(const ProfileData& prof);

  // Query whether the profile is positive
  bool get_is_positive_monotone() const {
    return is_positive_monotone;
  }

  private:
  // A canvas item for the layers graphs
  Glib::RefPtr< Goocanvas::Group > m_layers_group;
  void on_add_node_clicked();
  void on_remove_node_clicked();
  void on_corner_node_clicked();
  void on_round_node_clicked();
  void on_round_symmetric_node_clicked();
  void on_remove_layer_clicked();
  void on_add_layer_clicked();

  std::vector<std::shared_ptr<Layer>> m_layers;
  double m_xmargin=40;
  double m_graph_xmargin=50; // distance from end of data to axis arrow
  double m_graph_ymargin=50;
  double m_ymargin=40;
  double m_canvas_width=800;
  double m_canvas_height=800;

  double m_profile_maxx = 30;
  double m_profile_maxy = 10;
  const int m_max_layers = 3;

  const double control_point_radius = 5;
  const double node_radius = 10;

  bool is_positive_monotone = true;
};

#endif /* PROFILE-EDITING */
