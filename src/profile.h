//======================================================================
//  profile.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Jul 13 22:09:01 2021
//----------------------------------------------------------------------
#ifndef PROFILE_H
#define PROFILE_H

#include <array>
#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>


using Vec2 = glm::dvec2;

// use for import and output vec2 as json
nlohmann::json as_json(const Vec2& v);
Vec2 from_json(const nlohmann::json& j);

class NodeData {
  public:
  int node_type;
  Vec2 xy;
  Vec2 control_before_xy;
  Vec2 control_after_xy;

  nlohmann::json as_json(){
    nlohmann::json j;
    j["node_type"] = node_type;
    j["xy"] = ::as_json(xy);
    j["control_before_xy"] = ::as_json(control_before_xy);
    j["control_after_xy"] = ::as_json(control_after_xy);
    return j;
  }

  void from_json(const nlohmann::json& j) {
    node_type = j["node_type"];
    xy = ::from_json(j["xy"]);
    control_before_xy = ::from_json(j["control_before_xy"]);
    control_after_xy = ::from_json(j["control_after_xy"]);
  }
};

class LayerData : public std::vector<NodeData> {
  public:

  // Set the linear limit and calculate cache
  void set_linear_limit(double linear_limit=0.1);

  // Get a copy of the flat list. 
  std::vector<Vec2> get_flat_list();

  // Get the direction beyond the flattened version
  Vec2 get_end_dir() { return Vec2(0,0); } // TBD

  // Export to json
  nlohmann::json as_json() {
    nlohmann::json j;
    for (auto& v : *this)
        j.push_back(v.as_json());
    return j;
  }

  void from_json(const nlohmann::json& j) {
    this->resize(j.size());
    for (size_t i=0; i<j.size(); i++)
      (*this)[i].from_json(j[i]);
  }

  private:
  std::vector<Vec2> flat_list;
};

class ProfileData : public std::vector<LayerData> {
  public:
  void load_from_file(const std::string& filename);
  void save_to_file(const std::string& filename);
  std::string export_string();
  void load_from_string(const std::string& profile_string);

  // for debugging
  void save_flat_to_giv(const std::string& filename);
  
};

#endif /* PROFILE */
