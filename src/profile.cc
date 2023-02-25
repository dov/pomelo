// Data structures and flattener for the profile

#include <fstream>
#include <streambuf>
#include "profile.h"
#include <nlohmann/json.hpp>
#include <cairo.h>
#include <cairomm/cairomm.h>
#include <fmt/core.h>
#include "bezier-intersect.h"
#include "utils.h"

using json = nlohmann::json;

using namespace std;
using namespace fmt;

nlohmann::json as_json(const Vec2& v)
{
    nlohmann::json j;
    j["x"] = v.x;
    j["y"] = v.y;
    return j;
}

Vec2 from_json(const nlohmann::json& j)
{
  return Vec2(j.value("x", 0.0),
              j.value("y", 0.0));
}

void ProfileData::load_from_string(const string& profile_string)
{
  json j = json::parse(profile_string);

  this->resize(j["layers"].size());

  for (size_t i = 0; i<j["layers"].size(); i++)
  {
    auto jl = j["layers"][i];
    
    (*this)[i].from_json(jl);
  }
}

void ProfileData::load_from_file(const string& filename)
{
  string js = load_string_from_file(filename);
  load_from_string(js);
}

string ProfileData::export_string()
{
  json root;
  vector<json> layers;

  for (auto& v : *this)
      layers.push_back(v.as_json());
  root["layers"] = layers;

  return root.dump(2); 
}

void ProfileData::save_to_file(const std::string& filename)
{
  json root;
  vector<json> layers;

  for (auto& v : *this)
      layers.push_back(v.as_json());
  root["layers"] = layers;

  string js = root.dump(2); 

  string_to_file(export_string(), filename);
}

// Get a flattened version of the curve
void LayerData::set_linear_limit(double linear_limit)
{
  auto surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 100, 100);
  auto cr = Cairo::Context::create(surface);

  // build the bezier
  for (size_t i=0; i<this->size(); i++)
  {
    Vec2& xy = (*this)[i].xy;
    if (i==0)
      cr->move_to(xy.x,xy.y);
    else
    {
      Vec2& cp1 = (*this)[i-1].control_after_xy;
      Vec2& cp2 = (*this)[i].control_before_xy;
      Vec2& xy = (*this)[i].xy;
      cr->curve_to(cp1.x,cp1.y,
                   cp2.x,cp2.y,
                   xy.x,xy.y);
    }
  }

  cr->set_tolerance(linear_limit); 
  Cairo::Path *path = cr->copy_path_flat();

  cairo_path_t *cpath = path->cobj();
  this->flat_list.clear();
  for (int i=0; i < cpath->num_data; i += cpath->data[i].header.length) {
      auto data = &cpath->data[i];
      switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO:
      case CAIRO_PATH_LINE_TO:
        this->flat_list.push_back(Vec2(data[1].point.x,data[1].point.y));
        break;
      default:
        print("Oops...\n");
      }
  }
}

vector<Vec2> LayerData::get_flat_list(double x_start, double x_end)
{
  if (x_start < this->flat_list[0].x
      && x_end >  this->flat_list.back().x)
    return this->flat_list;

  vector<Vec2> flattened;
  bool inside=false; // flip flop for when we are in the interval
  for (size_t i=0; i<this->flat_list.size(); i++)
  {
    const Vec2& this_v = this->flat_list[i];
    if (!inside && this_v.x > x_start)
    {
      // interpolate between the previous pos
      if (i>0)
      {
        const Vec2& prev_v = this->flat_list[i-1];
        double xi = (x_start - prev_v.x)/(this_v.x-prev_v.x);
        flattened.push_back(
          {x_start, prev_v.y + xi * (this->flat_list[i].y-prev_v.y)});
      }
      inside=true;
    }
    if (inside && this->flat_list[i].x > x_end)
    {
      if (i>0)
      {
        const Vec2& prev_v = this->flat_list[i-1];
        double xi = (x_end - prev_v.x)/(this_v.x-prev_v.x);
        flattened.push_back(
          {x_end, prev_v.y + xi * (this->flat_list[i].y-prev_v.y)});
      }
      inside=false;
      break;
    }
    if (inside)
      flattened.push_back(this_v);
  }

  return flattened;
}

// for debugging
void ProfileData::save_flat_to_giv(const std::string& filename)
{
  ofstream fh(filename);

  const vector<string> colors = {"red","green","blue"};
  int layer_idx=0;
  for (auto& layer : *this)
  {
    layer.set_linear_limit();
    auto path = layer.get_flat_list();

    fh << format("$path layer {}\n"
                 "$color {}\n"
                 ,
                 layer_idx,
                 colors[layer_idx%colors.size()]);
    for (auto& p : path)
      fh << format("{} {}\n", p.x,p.y);
    fh << "\n";
    layer_idx++;
  }
}

// Get the intersection
bool LayerData::get_intersect_coord(double x,
                                    // output
                                    double& y)
{
  int ret = false;

  bezier_intersect::Line ln {{x,-1},{x,1}};

  for (int i=0; i<(int)this->size()-1; i++)
  {
    bezier_intersect::Bezier bz {
      { (*this)[i].xy.x,(*this)[i].xy.y},
      { (*this)[i].control_after_xy.x,(*this)[i].control_after_xy.y},
      { (*this)[i+1].control_before_xy.x,(*this)[1+i].control_before_xy.y},
      { (*this)[i+1].xy.x,(*this)[i+1].xy.y}};

    auto intersections = bezier_intersect::find_bezier_line_intersection(bz, ln);
  }

  return ret;
}
