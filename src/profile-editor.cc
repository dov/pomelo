// An editor of pomelo profiles
//
// Can't get the keyboard focus to work. :-(

#include "profile-editor.h"
#include "dov-mm-macros.h"
#include <gtkmm.h>
#include <goocanvasmm.h>
#include <fmt/core.h>

#include <string>

using namespace std;
using namespace fmt;
static constexpr double MY_TWO_PI = 2*3.141592653589793;

Gtk::Button *mmSvgButton(const std::string& filename)
{
  auto pixbuf = Gdk::Pixbuf::create_from_resource("/icons/" + filename,48,48);
  auto image = new Gtk::Image(pixbuf);
  auto button = mm<Gtk::Button>();
  button->set_image(*image);
  return button;
}


// Constructor
ProfileEditor::ProfileEditor()
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
  this->set_can_focus(true);
  this->set_focus_on_click(true);
  this->add_events(
    Gdk::EventMask(
      int(GDK_KEY_PRESS_MASK) |
      int(GDK_FOCUS_CHANGE_MASK) 
      )
    );


  // Upper buttons
  auto w_hbox = mmHBox;
  this->pack_start(*w_hbox, Gtk::PACK_SHRINK);
  w_hbox->show();
  {
    auto w_button = mmSvgButton("corner-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_corner_node_clicked));

    w_button = mmSvgButton("round-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_round_node_clicked));

    w_button = mmSvgButton("round-symmetric-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_round_symmetric_node_clicked));
    
    w_hbox->pack_start(*mm<Gtk::Separator>(), Gtk::PACK_SHRINK, 15);

    w_button = mmSvgButton("add-node-icon.svg");
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_add_node_clicked));
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button = mmSvgButton("remove-node-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_remove_node_clicked));


    w_hbox->pack_start(*mm<Gtk::Separator>(), Gtk::PACK_SHRINK, 15);

    w_button = mmSvgButton("add-layer-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_add_layer_clicked));
    w_button = mmSvgButton("remove-layer-icon.svg");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button->signal_clicked().connect(sigc::mem_fun(*this,
      &ProfileEditor::on_remove_layer_clicked));
  }

  // Canvas
  w_hbox = mmHBox;
  this->pack_start(*w_hbox, Gtk::PACK_SHRINK);
  auto w_canvas = mm<Goocanvas::Canvas>();
  w_hbox->pack_start(*w_canvas, Gtk::PACK_SHRINK);
  w_canvas->set_size_request(m_canvas_width,m_canvas_height);

  // Draw a graph just to make sure it works
  auto points = Goocanvas::Points(3);
  points.set_coordinate(0, m_xmargin, m_ymargin);
  points.set_coordinate(1, m_xmargin, m_canvas_height-m_ymargin);
  points.set_coordinate(2, m_canvas_width-m_xmargin, m_canvas_height-m_ymargin);
  auto line = Goocanvas::Polyline::create(false, points); 
  line->property_stroke_color() = "black" ;
  line->property_line_width() = 3.0 ;
  line->property_start_arrow() = true ;
  line->property_end_arrow() = true ;
  line->property_arrow_tip_length() = 6.0 ;
  line->property_arrow_length() = 10.0 ;
  line->property_arrow_width() = 10.0 ;

  auto root = w_canvas->get_root_item();
  root->add_child(line);

  // clear all selected items on button click
  w_canvas->signal_button_press_event().connect(
    [=](GdkEventButton*) -> bool{
      print("Got button press event\n");
      this->clear_all_selected();
      this->draw_layers();
      static_cast<Gtk::Widget*>(this)->grab_focus();
      return true;
    });
  w_canvas->add_events(
    Gdk::EventMask(
      int(GDK_KEY_PRESS_MASK) 
      )
    );
  static_cast<Gtk::Widget*>(w_canvas)->set_can_focus(true);
  static_cast<Gtk::Widget*>(w_canvas)->grab_focus();
  w_canvas->set_focus_on_click(true);
  w_canvas->signal_key_press_event().connect(
    [=](GdkEventKey*) -> bool{
      print("Got key press\n");
      return true;
    });
#if 0
  root->signal_key_press_event().connect(
    [=](const Glib::RefPtr<Goocanvas::Item>& item,
        GdkEventKey*) -> bool{
      print("Got key press\n");
      return true;
    });
  w_canvas->grab_focus(root);
#endif
  
  // Create a dummy layer
  m_layers.push_back(make_shared<Layer>());
  shared_ptr<Layer> layer = m_layers[m_layers.size()-1];
  layer->pe = this;
  layer->push_back(Node(this,layer.get(),NODE_CORNER, 0,0));
  layer->push_back(Node(this,layer.get(),NODE_CORNER, m_profile_maxx,m_profile_maxy));

  // Set control points for the first and the last points
  (*layer)[0].dxyp = {m_profile_maxx/10,m_profile_maxy/10};
  (*layer)[1].dxym = {-m_profile_maxx/10,-m_profile_maxy/10};

  // Create the group item for the layers
  m_layers_group = Goocanvas::Group::create();
  root->add_child(m_layers_group);

  populate_canvas_items();
  draw_layers();
  
#if 0
  // Lower buttons (a dialog?)
  w_hbox = mmHBox;
  this->pack_start(*w_hbox, Gtk::PACK_SHRINK);
  {
    auto w_button = mm<Gtk::Button>("Save");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button = mm<Gtk::Button>("Load");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
    w_button = mm<Gtk::Button>("Close");
    w_hbox->pack_start(*w_button, Gtk::PACK_SHRINK);
  }
#endif

  show_all_children();
}

// Rebuild the canvas items corresponding to the current canvas coordinates
void ProfileEditor::populate_canvas_items()
{
  string colors[2] = { "pink", "blue" }; // colors not selected and selected
  string color_per_layer[4] = {"red","green","orange","cyan" };


  // Remove the old layer drawing. Do from the end to not disturb the
  // enumeration
  for (int i=m_layers_group->get_n_children()-1; i>=0; i--)
    m_layers_group->remove_child(i);

  int layer_id = 0;
  for (auto layer : m_layers) {
    // Create group for the layer
    auto layer_group = Goocanvas::Group::create();
    layer->item_group = layer_group;

    // Create the bezier curve
    string path_string;

    for (size_t i=0; i<layer->size(); i++)
      {
        auto color = colors[int((*layer)[i].selected)];
        Glib::RefPtr<Goocanvas::Item> item;
  
        // Create the control points
        (*layer)[i].item_control_group = Goocanvas::Group::create();
        (*layer)[i].item_control_minus = Goocanvas::Ellipse::create(0,0,9,9);
        (*layer)[i].item_control_plus = Goocanvas::Ellipse::create(0,0,9,9);
        (*layer)[i].item_control_line = Goocanvas::Polyline::create(false,Goocanvas::Points(1));
  
#if 0
        // Set control points for the first and the last points
        if (i==0)
          (*layer)[i].dxyp = {m_profile_maxx/10,m_profile_maxy/10};
        if (i==layer->size()-1)
          (*layer)[i].dxym = {-m_profile_maxx/10,-m_profile_maxy/10};
#endif
        
        for (auto& it : { (*layer)[i].item_control_minus,
                         (*layer)[i].item_control_plus})
          {
            it->property_fill_color() = "cyan";
            it->property_stroke_color() = "black" ;
          }
  
        (*layer)[i].item_control_group->add_child((*layer)[i].item_control_line);
        if (i>0)
          (*layer)[i].item_control_group->add_child((*layer)[i].item_control_minus);
        if (i<layer->size()-1)
          (*layer)[i].item_control_group->add_child((*layer)[i].item_control_plus);
  
        // Add the control group, and it will be hidden immediately
        (*layer)[i].item_group = Goocanvas::Group::create();
        (*layer)[i].item_group->add_child((*layer)[i].item_control_group);
  
        if (!layer->is_baselayer || i<layer->size()-1)
          {
            auto circle = Goocanvas::Ellipse::create(0,0,15,15);
            circle->property_fill_color() = color ;
            circle->property_stroke_color() = "black" ;
            circle->property_line_width() = 1.5 ;
          
            (*layer)[i].item_circle = circle;
            item = circle;
  
          }
        else
          {
            // Create a triangle pointing in the direction
            auto points = Goocanvas::Points(3);
  
            auto poly = Goocanvas::Polyline::create(true, points); 
            poly->property_stroke_color() = "black" ;
            poly->property_fill_color() = color ;
            poly->property_line_width() = 1.5 ;
  
            item = poly;
            (*layer)[i].item_poly = poly;
          }
        (*layer)[i].item_group->add_child(item);
        layer_group->add_child((*layer)[i].item_group);
        (*layer)[i].item = item; // An abstract pointer
    
        // TBD - do this through on canvas creted like in
        //    https://github.com/GNOME/goocanvasmm/blob/goocanvasmm-2.0/examples/moving_shapes/window.cc
        for (auto it : vector<Glib::RefPtr<Goocanvas::Item>> {
            (*layer)[i].item,
            (*layer)[i].item_control_minus,
            (*layer)[i].item_control_plus } )
          {
            if (it)
              {
                it->signal_button_press_event().connect(
                  sigc::mem_fun((*layer)[i], &Node::on_button_press));
                it->signal_button_release_event().connect(
                  sigc::mem_fun((*layer)[i], &Node::on_button_release));
                it->signal_motion_notify_event().connect(
                  sigc::mem_fun((*layer)[i], &Node::on_motion_notify));
              }
          }
  
        auto path = Goocanvas::Path::create("");
        path->property_stroke_color() = color_per_layer[layer_id%4] ;
        path->property_line_width() = 3.0 ;
        layer_group->add_child(path);
        path->lower();
        layer->item_path = path;
    
        // Add the layer group
        m_layers_group->add_child(layer_group);
      }
    layer_id++;
  }
}

// Update the ites to match the layer coordinates
void ProfileEditor::draw_layers()
{
  string colors[2] = { "pink", "blue" }; // colors not selected and selected

  for (auto layer : m_layers) {
    // Create group for the layer
    auto layer_group = layer->item_group;

    // Create the bezier curve
    string path_string;

    for (size_t i=0; i<layer->size(); i++) {
      auto& n = (*layer)[i];
      auto color = colors[int((*layer)[i].selected)];

      double cx, cy;

      profile_coord_to_canvas_coord((*layer)[i].xy.x, (*layer)[i].xy.y,
                                    // output
                                    cx,cy);
      if (i==0)
        path_string += format("M {} {} ", cx,cy);
      else
        {
          double cp1x, cp1y, cp2x, cp2y;
          profile_coord_to_canvas_coord((*layer)[i-1].xy.x+(*layer)[i-1].dxyp.x,
                                        (*layer)[i-1].xy.y+(*layer)[i-1].dxyp.y,
                                        // output
                                        cp1x,cp1y);

          profile_coord_to_canvas_coord((*layer)[i].xy.x+(*layer)[i].dxym.x,
                                        (*layer)[i].xy.y+(*layer)[i].dxym.y,
                                        // output
                                        cp2x,cp2y);
          path_string += format("C {} {} {} {} {} {} ",
                                cp1x,cp1y,
                                cp2x,cp2y,
                                cx,cy);
        }
  
      // Draw a triangle for the direction (last node)
      if (layer->is_baselayer && i==layer->size()-1)
        {
          auto poly = (*layer)[i].item_poly;

          // TBD - Create a triangle pointing in the direction
          auto points = Goocanvas::Points(3);

          // Angle of line
          // TBD - Do this in canvas space!
          double dx,dy,th0;
          profile_delta_to_canvas_delta((*layer)[i].dxym.x,
                                        (*layer)[i].dxym.y,
                                        // output
                                        dx,dy);
          th0 = MY_TWO_PI/6-atan2(dy,dx);

          double radius = 20;
          for (int j=0; j<3; j++)
            {
              double th = th0 + j * MY_TWO_PI/3;
              double st = sin(th);
              double ct = cos(th);
              points.set_coordinate(j, cx+radius * ct, cy-radius * st);
            }
          
          poly->property_points() = points;
          poly->property_fill_color() = color;

        }
      else
        {
          auto circle = (*layer)[i].item_circle;
          circle->property_center_x() = cx;
          circle->property_center_y() = cy;
          circle->property_fill_color() = color;
        }

      // Update the control points. 
      vector<Vec2> coords;
      coords.clear();
      if (i>0)
          {
            double dx,dy;
            profile_delta_to_canvas_delta(n.dxym.x,n.dxym.y,
                                          // output
                                          dx,dy);
            double vx=cx+dx;
            double vy=cy+dy;
            n.item_control_minus->property_center_x() = vx;
            n.item_control_minus->property_center_y() = vy;
            coords.push_back({vx,vy});
          }
      coords.push_back({cx,cy});
      if (i < layer->size()-1)
          {
            double dx,dy;
            profile_delta_to_canvas_delta(n.dxyp.x,n.dxyp.y,
                                          // output
                                          dx,dy);
            double vx=cx+dx;
            double vy=cy+dy;
            (*layer)[i].item_control_plus->property_center_x() = vx;
            (*layer)[i].item_control_plus->property_center_y() = vy;
            coords.push_back({vx,vy});
          }
      auto points = Goocanvas::Points(coords.size());
      for (size_t i=0; i<coords.size(); i++)
        points.set_coordinate(i, coords[i][0], coords[i][1]);
      (*layer)[i].item_control_line->property_points() = points;
      (*layer)[i].auto_show_control();
    }
      
    layer->item_path->property_data() = path_string;
  }
}

void ProfileEditor::canvas_delta_to_profile_delta(double dcx, double dcy,
                                                  // output
                                                  double &dpx,double &dpy)
{
  double sx = (m_canvas_width - 2*m_xmargin - m_graph_xmargin)/m_profile_maxx;
  double sy = -(m_canvas_height - 2*m_ymargin - m_graph_ymargin)/m_profile_maxy;
  dpx = dcx/sx;
  dpy = dcy/sy;
}

void ProfileEditor::profile_delta_to_canvas_delta(double dpx, double dpy,
                                                  // output
                                                  double &dcx,double &dcy)
{
  double sx = (m_canvas_width - 2*m_xmargin - m_graph_xmargin)/m_profile_maxx;
  double sy = -(m_canvas_height - 2*m_ymargin - m_graph_ymargin)/m_profile_maxy;
  dcx = dpx*sx;
  dcy = dpy*sy;
}

void ProfileEditor::profile_coord_to_canvas_coord(double px, double py,
                                                  // output
                                                  double &cx,double &cy)
{
  double sx = (m_canvas_width - 2*m_xmargin - m_graph_xmargin)/m_profile_maxx;
  double sy = -(m_canvas_height - 2*m_ymargin - m_graph_ymargin)/m_profile_maxy;
  cx = m_xmargin + sx * px;
  cy = m_canvas_height - m_ymargin + sy * py;
}

void ProfileEditor::on_add_node_clicked()
{
  for (auto& layer : m_layers)
    layer->insert_node();
}

void ProfileEditor::on_remove_node_clicked()
{
  for (auto& layer : m_layers)
    layer->remove_selected_nodes();
}

void ProfileEditor::on_corner_node_clicked()
{
  for (auto& layer : m_layers)
    layer->corner_selected_nodes();
}

void ProfileEditor::on_round_node_clicked()
{
  for (auto& layer : m_layers)
    layer->round_selected_nodes();
}

void ProfileEditor::on_round_symmetric_node_clicked()
{
  for (auto& layer : m_layers)
    layer->round_selected_symmetric_nodes();
}

void ProfileEditor::on_add_layer_clicked()
{
  // just a dummy layer
  m_layers.push_back(make_shared<Layer>());
  size_t num_layers = m_layers.size();
  Layer *layer = m_layers[num_layers-1].get();
  layer->is_baselayer = false;
  layer->pe = this;
  layer->push_back(Node(this,layer,NODE_CORNER, 0,0.1*num_layers*m_profile_maxy));
  layer->push_back(Node(this,layer,NODE_CORNER, m_profile_maxx,m_profile_maxy));

  // Set control points for the first and the last points
  (*layer)[0].dxyp = {m_profile_maxx/10,m_profile_maxy/10};
  (*layer)[1].dxym = {-m_profile_maxx/10,-m_profile_maxy/10};

  populate_canvas_items();
  draw_layers();
}


void ProfileEditor::on_remove_layer_clicked()
{
}

void ProfileEditor::clear_all_selected(Layer *except_layer)
{
  for (auto layer : m_layers) {
    if (layer.get() == except_layer)
      {
        print("Skipping zeroing selected\n");
        continue;
      }
    for (auto& node : *layer)
      {
        node.selected = false;
        node.set_show_control(false);
      }
  }
}

bool Node::on_button_press(const Glib::RefPtr<Goocanvas::Item>& item,
                           GdkEventButton* event)
{
  // Set the except layer to the current layer if we are to
  // retain the selection for it
  Layer *except_layer = nullptr;
  if (event->state & GDK_SHIFT_MASK)
    except_layer = this->layer;
  
  if (!this->selected)
    pe->clear_all_selected(except_layer);

  this->selected = true;
  this->dragging = true;
  pe->draw_layers();

  drag_xy = { event->x, event->y };

  return true;
}
  
bool Node::on_button_release(const Glib::RefPtr<Goocanvas::Item>& item,
                             GdkEventButton* event)
{
  this->dragging = false;
  return true;
}


// constructor
Layer::Layer()
{
  // Dummy items
  this->item_group = Goocanvas::Group::create();
  this->item_path = Goocanvas::Path::create("");
}

Layer::~Layer()
{
  
}

void Layer::move_selected(double dpx, double dpy)
{
  // TBD: after each dpx,dpy make sure that the resulting path
  // is still simple.
  for (size_t i=0; i<this->size(); i++)
    {
      if ((*this)[i].selected)
        {
          // The first and last nodes for the base layer are fixed in x!
          if (this->is_baselayer && (i==0 || i==size()-1))
            (*this)[i].translate(0,dpy);
          else
            (*this)[i].translate(dpx,dpy);
        }
    }
}

void Node::move_control_point(int control_point, double dpx, double dpy)
{
  if (control_point==0)
    {
      dxym.x += dpx;
      dxym.y += dpy;

      if (type == NODE_CURVE_SYMMETRIC
          || type == NODE_CURVE)
        {
          double old_length = sqrt(dxyp.x*dxyp.x+dxyp.y*dxyp.y);
          double sym_length = sqrt(dxym.x*dxym.x+dxym.y*dxym.y);
          double s=1.0;
          if (type==NODE_CURVE)
            s = old_length/sym_length;

          dxyp.x = -dxym.x*s;
          dxyp.y = -dxym.y*s;
        }
    }
  else
    {
      dxyp.x += dpx;
      dxyp.y += dpy;

      if (type == NODE_CURVE_SYMMETRIC
          || type == NODE_CURVE)
        {
          double old_length = sqrt(dxym.x*dxym.x+dxym.y*dxym.y);
          double sym_length = sqrt(dxyp.x*dxyp.x+dxyp.y*dxyp.y);
          double s=1.0;
          if (type==NODE_CURVE)
            s = old_length/sym_length;

          dxym.x = -dxyp.x*s;
          dxym.y = -dxyp.y*s;
        }

    }
}

// Insert a new node between the first pair of selected nodes
void Layer::insert_node()
{
  // TBD - add multiple selected nodes
  for (size_t i=0; i<this->size()-1; i++)
    {
      if ((*this)[i].selected && (*this)[i+1].selected) {
        // Create an average of the two nodes and insert it.
        // TBD: update it for bezier curves
        double new_x = 0.5*((*this)[i].xy.x+(*this)[i+1].xy.x);
        double new_y = 0.5*((*this)[i].xy.y+(*this)[i+1].xy.y);
        Node new_node(this->pe, this, NODE_CURVE, new_x, new_y);

        // Calculcate control points!
        Vec2 v = (*this)[i+1].xy - (*this)[i].xy;
        double s = 0.1; // arbitrary strength of new node
        new_node.dxym = -v*s;
        new_node.dxyp = v*s;

        this->insert(this->begin()+i+1, new_node);
        break;
      }
    }
  pe->populate_canvas_items();
  pe->draw_layers();
}

// Remove selected nodes except the first and the last that
// may not be removed.
void Layer::remove_selected_nodes()
{
  for (size_t i=size()-2; i>0; i--)
    {
      if ((*this)[i].selected)
        this->erase(this->begin()+i,this->begin()+i+1);
    }
  pe->populate_canvas_items();
  pe->draw_layers();
}

// Make selected nodes into corner nodes
void Layer::corner_selected_nodes()
{
  for (size_t i=0; i<size(); i++)
    {
      if ((*this)[i].selected)
        {
          (*this)[i].type = NODE_CORNER;
          print("Making node {} into a corner\n", i);
        }
    }
}

// Make selected nodes into round
void Layer::round_selected_nodes()
{
  for (size_t i=0; i<size(); i++)
    {
      if ((*this)[i].selected)
        {
          (*this)[i].type = NODE_CURVE;
          (*this)[i].set_show_control(true);
          (*this)[i].move_control_point(0,0,0);
        }
    }
  pe->draw_layers();
}

// Make selected nodes into round
void Layer::round_selected_symmetric_nodes()
{
  for (size_t i=0; i<size(); i++)
    if ((*this)[i].selected)
      {
        (*this)[i].type = NODE_CURVE_SYMMETRIC;
        (*this)[i].set_show_control(true);
        (*this)[i].move_control_point(0,0,0);
      }
  pe->draw_layers();
}

bool Node::on_motion_notify(const Glib::RefPtr<Goocanvas::Item>& item,
                            GdkEventMotion* event)
{
  if (this->dragging)
    {
      auto new_x = event->x;
      auto new_y = event->y;

      // Calculate and update the translation of the node
      double dpx,dpy;
      pe->canvas_delta_to_profile_delta(new_x-drag_xy.x, new_y-drag_xy.y,
                                        // output
                                        dpx,dpy);

      // Check whether we are moving the point or the control points
      if (item == this->item)
        this->layer->move_selected(dpx,dpy);
      else
        {
          if (item == this->item_control_minus)
            this->move_control_point(0,dpx,dpy);
          else
            this->move_control_point(1,dpx,dpy);
        }

      drag_xy = {new_x, new_y};

      pe->draw_layers();
    }
  return true;
}

void Node::translate(double dpx, double dpy)
{
  this->xy = this->xy + Vec2(dpx, dpy);
}

// show the nodes if selected (and possibly other properties layer on)
void Node::auto_show_control()
{
  set_show_control(this->selected);
}

void Node::set_show_control(bool show_control)
{
  // ask parent to display or hide the control nodes
  if (show_control)
    {
      item_control_group->set_simple_transform(0,0,1.0,0);
      item_control_group->raise();
    }
  else
    // "Hide it" by moving it far far away
    item_control_group->set_simple_transform(0,9999,1.0,0);
}

// Get the current edited profile as an exported profile data
ProfileData ProfileEditor::get_profile()
{
  ProfileData prof;

  for (const auto& layer : m_layers)
  {
    LayerData layer_data;

    for (auto& n : *layer)
    {
      NodeData node_data;
      node_data.node_type = int(n.type); // tbd
      node_data.xy = n.xy;
      node_data.control_before_xy = n.xy+n.dxym;
      node_data.control_after_xy = n.xy+n.dxyp;
      layer_data.push_back(node_data);
    }

    prof.push_back(layer_data);
  }

  return prof;
}

// Set the profile from the ProfileData
void ProfileEditor::set_profile(const ProfileData& prof)
{
  m_layers.clear();
  print("prof_size = {}\n", prof.size());
  for (size_t i=0; i<prof.size(); i++)
  {
    m_layers.push_back(make_shared<Layer>());
    auto & layer_data = prof[i];

    // Initialize the new layer
    m_layers[i]->pe = this;
    
    m_layers[i]->clear();
    for (auto& node_data : layer_data)
    {
      Node n(this,
             m_layers[i].get(),
             NodeType(node_data.node_type));
      n.xy = node_data.xy;
      n.dxym = node_data.control_before_xy - node_data.xy;
      n.dxyp = node_data.control_after_xy - node_data.xy;
      m_layers[i]->push_back(n);
    }
  }
  populate_canvas_items();
  draw_layers();
}
