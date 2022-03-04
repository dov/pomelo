// Data structures and flattener for the profile

#include <fstream>
#include <streambuf>
#include "profile.h"
#include <nlohmann/json.hpp>
#include <cairo.h>
#include <cairomm/cairomm.h>
#include <fmt/core.h>

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

static void string_to_file(const string& text,
                           const string& filename)
{
  ofstream out(filename);
  out << text;
  out.close();
}

static string load_string_from_file(const string& filename)
{
  ifstream inf(filename);
  return string(istreambuf_iterator<char>(inf),
                istreambuf_iterator<char>());
}

void ProfileData::load_from_file(const string& filename)
{
  json j;

  string js = load_string_from_file(filename);

  j = json::parse(js);

  this->resize(j["layers"].size());

  for (size_t i = 0; i<j["layers"].size(); i++)
  {
    auto jl = j["layers"][i];
    
    (*this)[i].from_json(jl);
  }
}

void ProfileData::save_to_file(const std::string& filename)
{
  json root;
  vector<json> layers;

  for (auto& v : *this)
      layers.push_back(v.as_json());
  root["layers"] = layers;

  string js = root.dump(2); 

  string_to_file(js, filename);
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

vector<Vec2> LayerData::get_flat_list()
{
  return this->flat_list;
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

