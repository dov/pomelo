//======================================================================
//  mesh-viewer.h - A viewer for a mesh
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Dec 16 23:14:11 2020
//----------------------------------------------------------------------
#ifndef MESH_VIEWER_H
#define MESH_VIEWER_H

#include <gtkmm.h>
#include <glm/mat4x4.hpp>
#include <epoxy/gl.h>
#include "pomelo-settings.h"
#include "mesh.h"

class MeshViewer : public Gtk::GLArea
{
  public:
  struct VertexInfo {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec3 bary;
  };


  // Constructor
  MeshViewer(std::shared_ptr<PomeloSettings> pomelo_settings);

  // Create a mesh 
  void set_mesh(std::shared_ptr<Mesh> mesh);
  void set_mesh_file(const std::string& mesh_filename);
  void redraw();

  // Reset the view to the original view
  void reset_view();

  // Whether to use the edge shader or the non-edge shader
  void set_show_edge(bool show_edge);
  void set_show_matcap(bool show_matcap);
  void refresh_from_settings();

  private:

  // signals
  void on_realize() override;
  void on_unrealize() override;
  bool on_render (const Glib::RefPtr< Gdk::GLContext >& context) override;
  bool on_button_press_event (GdkEventButton* button_event) override;
  bool on_key_press_event (GdkEventKey* key_event) override;
  bool on_motion_notify_event (GdkEventMotion* motion_event) override;
  bool on_scroll_event (GdkEventScroll* scroll_event) override;
  bool on_enter_notify_event (GdkEventCrossing *event) override;

  // Used during init
  void init_shaders();
  void init_buffers(guint *vao_out);
  void update_geometry();
  void update_matcap(); // TBD add parameter

  // Request a redraw
  void draw_mesh();

  // Setup the world view
  void setup_world(double scale,
                   float *view_quat,
                   const glm::vec3& pivot);

  // The default quaternion
  void setup_quat();

  // Build the projection matrix. This uses the current window size.
  void build_projection_matrix();

  void view_port_to_world(glm::vec3 view_port_coord,
                          // output
                          glm::vec3& world_coordinate);

  // Refresh from settings

  bool m_realized = false;
  
  enum {
    DEFAULT_PROGRAM=0,
    DEFAULT_PROGRAM_EDGE,
    MATCAP_PROGRAM,
    MATCAP_PROGRAM_EDGE,
    NUM_PROGRAMS
  };

  // OpenGl structures
  guint m_vao {0};
  guint m_buffer {0};
  std::vector<guint> m_programs {0};

  // Describe the vertex attribute layout
  guint m_position_index {0};
  guint m_color_index {1};
  guint m_normal_index {2};
  guint m_bary_index {3};

  // Uniform locations in the shaders
  guint m_proj_loc {0};
  guint m_mv_loc {0};
  guint m_normal_matrix_loc {0};
  guint m_buffer_id;

  // Material definition
  guint m_shininess;
  guint m_specular;
  guint m_diffuse;
  guint m_ambient;

  // The texture for the matcap
  guint m_matcap_texture {0};

  Glib::RefPtr< Gdk::Pixbuf > m_img;

  // which shader to use
  bool m_show_edge = false;
  bool m_show_matcap = false;

  // world description
  glm::mat4 m_proj_matrix;
  glm::mat4 m_world;
  glm::vec3 m_pivot {0,0,0};
  glm::vec3 m_camera {0,0,-10};

  // Viewing direction quaternion used via the trackball class
  float m_view_quat[4] = { 0.0, 0.0, 0.0, 1.0 };

  // User scale
  double m_view_scale = 1.0;
  double m_size_scale = 1.0;
  
  // Background color and mesh colors
  glm::vec3 m_background = {0.4,0.4,0.5};
  glm::vec3 m_mesh_color = {0.8,0.8,0.8};

  // Whether there is a valid matcap
  bool m_has_matcap = false;

  // State for the trackball
  int m_begin_x;
  int m_begin_y;

  // TBD - change this to vertices and indices.
  struct HWMesh {
    std::array<float,6> bbox;
    std::vector<VertexInfo> vertices;
  };

  HWMesh m_hw_mesh;
  std::shared_ptr<PomeloSettings> m_pomelo_settings;
  std::shared_ptr<Mesh> m_mesh;
};

#endif /* MESH-VIEWER */
