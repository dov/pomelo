// A basic OpenGL viewer that shows the resulting mesh

#include <spdlog/spdlog.h>
#include <epoxy/gl.h>
#include "mesh-viewer.h"
#include <iostream>
#include <limits>
#include <fmt/core.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gdkmm/pixbuf.h>
#include "trackball.h"
#include <spdlog/spdlog.h>

using namespace std;
using namespace fmt;
using namespace glm;

#define VIEW_INIT_AXIS_X 1.0
#define VIEW_INIT_AXIS_Y 0.0
#define VIEW_INIT_AXIS_Z 0.0
#define VIEW_INIT_ANGLE  20.0
#define VIEW_SCALE_MAX 10000.0
#define VIEW_SCALE_MIN 0.001

#define DIG_2_RAD (G_PI / 180.0)
#define RAD_2_DIG (180.0 / G_PI)

// Print a floating matrix from a pointer
static void print_mat(float *m)
{
  for (int i=0; i<16; i++)
    {
      print("{:.2} ", m[i]);
      if ((i+1)%4==0)
        print("\n");
    }
}

// Constructor
MeshViewer::MeshViewer(std::shared_ptr<PomeloSettings> pomelo_settings)
  :  m_pomelo_settings(pomelo_settings)
{
  spdlog::info("Creating MeshViewer");
  spdlog::info("gl_version = {}", epoxy_gl_version());
  this->add_events (
    Gdk::EventMask(GDK_POINTER_MOTION_MASK    |
                   GDK_BUTTON_PRESS_MASK      |
                   GDK_KEY_PRESS_MASK      |
                   GDK_FOCUS_CHANGE_MASK |
                   GDK_LEAVE_NOTIFY_MASK |
                   GDK_ENTER_NOTIFY_MASK |
                   GDK_SCROLL_MASK      |
                   GDK_VISIBILITY_NOTIFY_MASK 
                   ));


  set_can_focus(true);
  setup_projection_matrix();

  // Some sample static data
  static const struct MeshViewer::VertexInfo vertex_data[] = {
    { {  0.0f,  0.500f, 0.0f }, {1,0,0}, {0,0,1}, {1,0,0} },
    { {  0.5f, -0.366f, 0.0f }, {0,1,0}, {0,0,1}, {0,1,0} },
    { { -0.5f, -0.366f, 0.0f }, {0,0,1}, {0,0,1}, {0,0,1} },
  };

  m_hw_mesh.vertices.clear();

  for (auto v : vertex_data)
    m_hw_mesh.vertices.push_back(v);

  spdlog::info("Done creating meshviewer");
}

// setup the projection matrix based on the current setting
void MeshViewer::setup_projection_matrix()
{
  // Setup the project matrix. Currently static
  double thetaInDeg=8.0;
  double znear=1;
  double zfar=20;
  double width = get_allocation().get_width();
  double height = get_allocation().get_height();
  double aspect = 1.0*width / height; // aspect ratio
  static double PI { 3.141592653589793 };
  float theta = thetaInDeg / 180.0 * PI;
  float range = zfar - znear;
  float invtan = 1./tan(theta/2.);

  m_proj_matrix = glm::mat4(1.0);

  if (m_orthonormal)
    {
      double l=-2.4,r=2.4;
      double dx = l-r;
      double dy = -dx/aspect;
      double dz = 1000; // Not sure about this!

      m_proj_matrix[0][0] = -2.0 / dx;
      m_proj_matrix[1][1] = 2.0 / dy;
      m_proj_matrix[2][2] = -2.0/ dz;
      m_proj_matrix[3][3] = 1;

#if 0
      double rx = 0;
      double ry = 0;
      double rz = 0; // -(zfar+znear)/(zfar-znear);

      m_proj_matrix[3][0] = rx;
      m_proj_matrix[3][1] = ry;
      m_proj_matrix[3][2] = rz*0.01;
#endif

    }
  else
    {
    #if 0
      m_proj_matrix = glm::perspective(thetaInDeg,aspect,znear,zfar);
      m_proj_matrix = glm::lookAt(glm::vec3(0,0,-10),
                                  glm::vec3(0,0,0),
                                  glm::vec3(0,1,0));
    
    #endif
      m_proj_matrix[0][0] = invtan / aspect;
      m_proj_matrix[1][1] = invtan;
      m_proj_matrix[2][2] = -(znear + zfar) / range;
      m_proj_matrix[3][2] = -1;
      m_proj_matrix[2][3] = -2 * znear * zfar / range;
      m_proj_matrix[3][3] = 1;
      //  m_proj_matrix = glm::transpose(m_proj_matrix);
    }
#if 0
  print("proj matrix\n");
  print_mat(&m_proj_matrix[0][0]);
#endif
}

// Describe geometry. This is a bit wasteful, but I don't expect
// to show huge meshes in Pomelo in any case

// Returns the shader
static guint
create_shader (int shader_type,
               const Glib::RefPtr<const Glib::Bytes>& source)
{
  spdlog::info("create_shader(). type={}",
               shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment");
  guint shader = glCreateShader (shader_type);
  spdlog::info("Ok. creating the shader");
  
  // Get pointer and size
  gsize len=0;
  const char* src = (const char*)source->get_data(len);

  // Turn into GL structure
  int lengths[] = { (int)len };
  glShaderSource (shader, 1, &src, lengths);
  spdlog::info("Ok shader src");
  glCompileShader (shader);

  int status;
  glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len;
      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      string error_string;
      error_string.resize(log_len+1);
      glGetShaderInfoLog (shader, log_len, NULL, &error_string[0]);

      glDeleteShader (shader);
      shader = 0;

      auto error_str = format(
          "Compilation failure in {} shader: {}",
          shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
          error_string);
      spdlog::error("{}", error_str);
      throw runtime_error(error_str);
    }

  return shader;
}

void MeshViewer::init_shaders ()
{
  spdlog::info("init_shaders()");
  // TBD - init the shaders
  /* load the vertex shader */
  Glib::RefPtr<const Glib::Bytes> vertex_source = Gio::Resource::lookup_data_global("/shaders/vertex.glsl");
  Glib::RefPtr<const Glib::Bytes> fragment_source = Gio::Resource::lookup_data_global("/shaders/fragment.glsl");
  Glib::RefPtr<const Glib::Bytes> fragment_edge_source = Gio::Resource::lookup_data_global("/shaders/fragment-edge.glsl");
  Glib::RefPtr<const Glib::Bytes> vertex_matcap_source = Gio::Resource::lookup_data_global("/shaders/vertex-matcap.glsl");
  Glib::RefPtr<const Glib::Bytes> fragment_matcap_source = Gio::Resource::lookup_data_global("/shaders/fragment-matcap.glsl");
  Glib::RefPtr<const Glib::Bytes> fragment_matcap_edge_source = Gio::Resource::lookup_data_global("/shaders/fragment-edge-matcap.glsl");
  
  m_position_index = 0;
  m_color_index = 1;
  m_normal_index = 2;
  m_bary_index = 3;

  guint shaders[] = {
    create_shader(GL_VERTEX_SHADER, vertex_source),
    create_shader(GL_FRAGMENT_SHADER, fragment_source),
    create_shader(GL_VERTEX_SHADER, vertex_source),
    create_shader(GL_FRAGMENT_SHADER, fragment_edge_source),
    create_shader(GL_VERTEX_SHADER, vertex_matcap_source),
    create_shader(GL_FRAGMENT_SHADER, fragment_matcap_source),
    create_shader(GL_VERTEX_SHADER, vertex_matcap_source),
    create_shader(GL_FRAGMENT_SHADER, fragment_matcap_edge_source)
  };
  m_programs.clear();
  spdlog::info("make_current");
  this->make_current();
  for (int prog_idx=0; prog_idx<NUM_PROGRAMS; prog_idx++)
    {
      guint program = glCreateProgram();
      glAttachShader(program, shaders[prog_idx*2]);
      glAttachShader(program, shaders[prog_idx*2+1]);
      glLinkProgram (program);

      int status = 0;
      glGetProgramiv (program, GL_LINK_STATUS, &status);
      if (status == GL_FALSE)
        {
          int log_len = 0;
          glGetProgramiv (program, GL_INFO_LOG_LENGTH, &log_len);
    
          string error_string;
          error_string.resize(log_len+1);
          glGetProgramInfoLog (program, log_len, NULL, &error_string[0]);
    
          glDeleteProgram (program);
          throw runtime_error(
            format(
              "Linking failure in program {}: {}",
              program, error_string));
        }

      glBindAttribLocation(program, m_position_index, "position");
      glBindAttribLocation(program, m_color_index, "color");
      glBindAttribLocation(program, m_normal_index, "normal");
      glBindAttribLocation(program, m_bary_index, "bary");

      m_programs.push_back(program);
    }

  /* the individual shaders can be detached and destroyed */
  for (int prog_idx=0; prog_idx<(int)m_programs.size(); prog_idx++)
    {
      glDetachShader (m_programs[prog_idx], shaders[2*prog_idx]);
      glDetachShader (m_programs[prog_idx], shaders[2*prog_idx+1]);
    }

  for (auto sh : shaders)
    glDeleteShader (sh);

  spdlog::info("Done init_shaders()");
}

void MeshViewer::init_buffers (guint *vao_out)
{
  guint vao;

  /* we need to create a VAO to store the other buffers */
  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);

  /* this is the VBO that holds the vertex data */
  glGenBuffers (1, &m_buffer_id);
  glBindBuffer (GL_ARRAY_BUFFER, m_buffer_id);
  spdlog::info("glBufferData()");
  glBufferData (GL_ARRAY_BUFFER,
                m_hw_mesh.vertices.size() * sizeof(MeshViewer::VertexInfo),
                &m_hw_mesh.vertices[0].position[0], GL_DYNAMIC_DRAW);

  // enable and set the attributes
  glEnableVertexAttribArray (m_position_index);
  glVertexAttribPointer (m_position_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct VertexInfo),
                         (GLvoid *) (G_STRUCT_OFFSET (struct VertexInfo,
                                                      position)));
  glEnableVertexAttribArray (m_color_index);
  glVertexAttribPointer (m_color_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct VertexInfo),
                         (GLvoid *) (G_STRUCT_OFFSET (struct VertexInfo,
                                                      color)));
  glEnableVertexAttribArray (m_normal_index);
  glVertexAttribPointer (m_normal_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct VertexInfo),
                         (GLvoid *) (G_STRUCT_OFFSET (struct VertexInfo,
                                                      normal)));
  glEnableVertexAttribArray (m_bary_index);
  glVertexAttribPointer (m_bary_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct VertexInfo),
                         (GLvoid *) (G_STRUCT_OFFSET (struct VertexInfo,
                                                      bary)));

  /* reset the state; we will re-enable the VAO when needed */
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindVertexArray (0);

  /* the VBO is referenced by the VAO */
  //  glDeleteBuffers (1, &m_buffer_id); /* Do I need this?

  if (vao_out != NULL)
    *vao_out = vao;

}

void MeshViewer::update_geometry()
{
  glBindBuffer (GL_ARRAY_BUFFER, m_buffer_id);
  glBufferData (GL_ARRAY_BUFFER,
                m_hw_mesh.vertices.size() * sizeof(MeshViewer::VertexInfo),
                &m_hw_mesh.vertices[0].position[0], GL_DYNAMIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void MeshViewer::update_matcap()
{
  spdlog::info("update_matcap");

  // The texture for the matcap
  spdlog::info("generating texture");
  glGenTextures(1, &m_matcap_texture);

  spdlog::info("getting texture from settings");
  string filename = m_pomelo_settings->get_string_default("matcap_filename");

  try {
    spdlog::info("creating pixbuf from {}", filename);
    m_img = Gdk::Pixbuf::create_from_file(filename);
    int width = m_img->get_width();
    int height = m_img->get_height();
    int n_channels = m_img->get_n_channels();
    spdlog::info("n_channels = {}\n", n_channels);
    uint8_t *data = m_img->get_pixels();
    glBindTexture(GL_TEXTURE_2D, m_matcap_texture);
    GLenum input_format =  n_channels == 4 ? GL_RGBA : GL_RGB;

    // No support for row stride in glTexImage2D
  
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Match gdkpixbuf
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
      GL_CLAMP_TO_EDGE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, input_format, GL_UNSIGNED_BYTE, data);
    m_has_matcap = true;
  }
  catch(...) {
    spdlog::error("Got unknown!");
    m_has_matcap= false;
  }
}

void MeshViewer::on_realize()
{
  m_realized = true;
  spdlog::info("On realize...");
  Gtk::GLArea::on_realize();

  refresh_from_settings();

  spdlog::info("make_current");
  this->make_current();
  spdlog::info("set_has_depth_buffer");
  this->set_has_depth_buffer(true);
  spdlog::info("done set_has_depth_buffer");

  init_shaders();

  // Init the buffer
  init_buffers (&m_vao);

  //  update_matcap();

  setup_quat();
}

// The default quaternion
void MeshViewer::setup_quat()
{
  // Set up the initial view transformation
  float sine = sin (0.5 * VIEW_INIT_ANGLE * DIG_2_RAD);
  m_view_quat[0] = VIEW_INIT_AXIS_X * sine;
  m_view_quat[1] = VIEW_INIT_AXIS_Y * sine;
  m_view_quat[2] = VIEW_INIT_AXIS_Z * sine;
  m_view_quat[3] = cos (0.5 * VIEW_INIT_ANGLE * DIG_2_RAD);
}

void MeshViewer::on_unrealize()
{
  Gtk::GLArea::on_unrealize();
}

bool MeshViewer::on_render (const Glib::RefPtr< Gdk::GLContext >& context)
{
  spdlog::info("on_render()");
  try
  {
    throw_if_error(); // grom gle area
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor (m_background[0],
                  m_background[1],
                  m_background[2], 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    draw_mesh();

    glFlush();

  }
  catch(const Gdk::GLError& gle)
  {
    spdlog::error("An error occurred in the render callback of the GLArea: {} - {} - {}",
                  gle.domain(), gle.code(), gle.what().c_str());
    return false;
  }
  

  return true;
  
}

// Draw the mesh (or meshes)
void MeshViewer::draw_mesh()
{
  guint program;
  if (m_has_matcap && m_show_matcap)
    {
      if (m_show_edge)
        program = m_programs[MATCAP_PROGRAM_EDGE];
      else
        program = m_programs[MATCAP_PROGRAM];
    }
  else
    {
      if (m_show_edge)
        program = m_programs[DEFAULT_PROGRAM_EDGE];
      else
        program = m_programs[DEFAULT_PROGRAM];
    }
  
  glUseProgram(program);

  setup_world(m_size_scale*m_view_scale, 
              m_view_quat, m_pivot);
  
  // get the uniform locations
  m_proj_loc = glGetUniformLocation (program, "projMatrix");
  m_mv_loc = glGetUniformLocation (program, "mvMatrix");
  m_normal_matrix_loc = glGetUniformLocation (program, "normalMatrix");

  if (m_has_matcap && m_show_matcap)
    {
      guint texture_loc = glGetUniformLocation (program, "texMatcap");
      glUniform1i(texture_loc, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_matcap_texture);
    }
  else
    {
      guint shininess_loc=glGetUniformLocation (program, "shininess");
      guint specular_loc=glGetUniformLocation (program, "specular");
      guint diffuse_loc=glGetUniformLocation (program, "diffuse");
      guint ambient_loc=glGetUniformLocation (program, "ambient");
    
      // Fixed light properties - Should be configurable
      glUniform1f(shininess_loc, 10.0);
      glUniform1f(specular_loc, 0.1);
      glUniform1f(diffuse_loc, 0.8);
      glUniform1f(ambient_loc, 0.2);
    }

  //  m_world[2][1] = 0;
  //  m_world[3] = glm::vec4 {0,0,0,1};
  auto model_view_matrix = glm::translate(glm::mat4(1.0), m_camera) * m_world;

  // Build the projection matrix. Is there a better place?
  setup_projection_matrix();

  glUniformMatrix4fv (m_proj_loc, 1, GL_FALSE, &m_proj_matrix[0][0]);

  // update the "mvp" matrix we use in the shader 
  glUniformMatrix4fv (m_mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);

  glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_view_matrix)));
  glUniformMatrix3fv (m_normal_matrix_loc, 1, GL_FALSE, &normal_matrix[0][0]);

  glBindVertexArray (m_vao);

#if 0
  cout << "Draw mesh:\n"
       << "proj:\n"
       << glm::to_string(m_proj_matrix[0]) << "\n"
       << glm::to_string(m_proj_matrix[1]) << "\n"
       << glm::to_string(m_proj_matrix[2]) << "\n"
       << glm::to_string(m_proj_matrix[3]) << "\n"
       << "modelview:\n"
       << glm::to_string(model_view_matrix[0]) << "\n"
       << glm::to_string(model_view_matrix[1]) << "\n"
       << glm::to_string(model_view_matrix[2]) << "\n"
       << glm::to_string(model_view_matrix[3]) << "\n"
       << "pivot:\n"
       << glm::to_string(m_pivot) << "\n"
    ;
#endif
  
  /* draw the three vertices as a triangle */
  glDrawArrays (GL_TRIANGLES, 0, m_hw_mesh.vertices.size());

  glBindVertexArray (0);
  glUseProgram(0);
}

bool MeshViewer::on_button_press_event (GdkEventButton* button_event)
{
  m_begin_x = button_event->x;
  m_begin_y = button_event->y;
    
  return true;
}

bool MeshViewer::on_key_press_event (GdkEventKey* key_event)
{
  spdlog::info("Got a key press = {}", key_event->string);
  string key_string(key_event->string);
  if (key_string == "1"
      || key_string == "f")
    reset_view();

  return true;
}

void MeshViewer::reset_view()
{
  auto bbox = m_hw_mesh.bbox; // shortcut
  
  m_view_scale = 1.0;
  // Pivot point for rotating around the center of the object.
  m_pivot = vec3 {
    0.5*(bbox[0]+bbox[3]),
    0.5*(bbox[1]+bbox[4]),
    0.5*(bbox[2]+bbox[5])};
  
  setup_quat();

  redraw();
}

bool MeshViewer::on_motion_notify_event (GdkEventMotion* motion_event)
{
  double w = get_allocation().get_width();
  double h = get_allocation().get_height();
  double x = motion_event->x;
  double y = motion_event->y;
  // print("w h x y = {} {} {} {}\n", w,h,x,y);
  float d_quat[4];

  if (motion_event->state & GDK_BUTTON1_MASK)
  {
    trackball (d_quat,
               (2.0 * m_begin_x - w) / w,
               (h - 2.0 * m_begin_y) / h,
               (2.0 * x - w) / w,
               (h - 2.0 * y) / h
               );
    add_quats (d_quat, m_view_quat, m_view_quat);
  }
  else if (motion_event->state & GDK_BUTTON2_MASK)
  {
    // Panning
    // Calculate the world distance for a window line from
    // (beginX,beginY,Z) to (x,y,Z) where Z is the z-pos
    // of the current pivot.

    double src_x = 2.0*m_begin_x/w-1.0;
    double src_y = 1.0-2.0*m_begin_y/h;
    double dst_x = 2.0*x/w-1.0;
    double dst_y = 1.0-2.0*y/h;
    glm::vec3 worldSrc, worldDst;

    view_port_to_world(glm::vec3(src_x,src_y,0), // z=0.5*(znear+zfar)
                       // output
                       worldSrc);
    view_port_to_world(glm::vec3(dst_x,dst_y,0),
                       // output
                       worldDst);

    glm::vec3 dxyz = worldDst-worldSrc;
    dxyz.x*=10;
    dxyz.y*=10;
    dxyz.z*=10;

    m_pivot -= dxyz;
  }

  m_begin_x = x;
  m_begin_y = y;

  redraw();
  
  return true;
}

bool MeshViewer::on_scroll_event(GdkEventScroll *scroll_event)
{
  if (scroll_event->direction)
    m_view_scale /= 1.2;
  else
    m_view_scale *= 1.2;

  if (m_view_scale > VIEW_SCALE_MAX)
    m_view_scale = VIEW_SCALE_MAX;
  else if (m_view_scale < VIEW_SCALE_MIN)
    m_view_scale = VIEW_SCALE_MIN;

  redraw();

  return true;
}

bool MeshViewer::on_enter_notify_event (GdkEventCrossing *event)
{
  if (!this->has_focus())
    this->grab_focus();
  return true;
}

void MeshViewer::redraw()
{
  double w = get_window()->get_width();
  double h = get_window()->get_height();

  Gdk::Rectangle rect;
  rect.set_x(0);
  rect.set_y(0);
  rect.set_width(w);
  rect.set_height(h);

  get_window()->invalidate_rect(rect, true);
}

void MeshViewer::set_meshes(vector<shared_ptr<Mesh>> meshes,
                            bool update_view)
{
  spdlog::info("set_meshes update_view={}. mesh_size={}",
               update_view, meshes.size());
  m_meshes = meshes;
  if (!m_meshes.size())
  {
    spdlog::info("No mesh to use!");
    return;
  }
  auto& bbox = m_hw_mesh.bbox; // shortcut
  m_hw_mesh.vertices.clear();
  for (int k=0; k<3; k++)
    {
      bbox[k] = numeric_limits<float>::infinity();
      bbox[k+3] = -1 * numeric_limits<float>::infinity();
    }
    
  for (size_t mesh_idx=0; mesh_idx<meshes.size(); mesh_idx++)
    {
      if (mesh_idx < m_show_layer.size() && !m_show_layer[mesh_idx])
        continue;

      vector<dvec3>& vertices = meshes[mesh_idx]->vertices; // shortcut
      if (vertices.size() % 3 != 0)
      {
        spdlog::error("Expected number of vertices to a multiple of 3!");
        throw runtime_error("Expected number of vertices to a multiple of 3!");
      }
    
      for (size_t i=0; i<vertices.size(); i+= 3)
        {
          const vec3& v1 {vertices[i]};
          const vec3& v2 {vertices[i+1]};
          const vec3& v3 {vertices[i+2]};
    
          vec3 normal = glm::normalize(cross(v2-v1,v3-v1));
          for (int j=0; j<3; j++)
            {
              // Calculate the bounding box
              for (int k=0; k<3; k++) {
                float vv = vertices[i+j][k];
                if (vv<bbox[k])
                  bbox[k] =vv;
                if (vv>bbox[k+3])
                  bbox[k+3] = vv;
              }
              m_hw_mesh.vertices.push_back({
                  vertices[i+j],
                  //vec3(1.0,fmod(1.0*i/(20*3),1.0),fmod(1.0*i/10,1.0)), // Color red
                  m_mesh_color[mesh_idx],
                  normal,
                  vec3(j==0,j==1,j==2) // bary
                });
            }
      }
    }
    
  spdlog::info("Loaded {} triangles", m_hw_mesh.vertices.size()/3);

  if (update_view)
    {
      // setup the scaling
      double dim_max = 0;
      for (int i=0; i<3; i++){
        double dim = bbox[i+3]-bbox[i];
        if (dim >dim_max)
          dim_max = dim;
      }
      m_size_scale = 2.0/dim_max; // Not sure why I need the two here...
    
      // Pivot point for rotating around the center of the object.
      m_pivot = vec3 {
        0.5*(bbox[0]+bbox[3]),
        0.5*(bbox[1]+bbox[4]),
        0.5*(bbox[2]+bbox[5])};
  
      spdlog::info("bbox = {} {} {} {} {} {}\n",
                   bbox[0],
                   bbox[1],
                   bbox[2],
                   bbox[3],
                   bbox[4],
                   bbox[5]);
    }

  //  redraw();
  if (m_vao)
    update_geometry();
  spdlog::info("Done set_meshes()!");
}

void MeshViewer::set_mesh_file(const std::string& mesh_filename)
{
  shared_ptr<Mesh> mesh = read_stl(mesh_filename);
  vector<shared_ptr<Mesh>> meshes = {mesh};
  set_meshes(meshes);
}

// Setup the world view
void MeshViewer::setup_world(double scale,
                             float *view_quat,
                             const glm::vec3& pivot)
{
  m_world = glm::scale(glm::mat4(1.0), vec3(scale,scale,scale));
  m_pivot = pivot;

  glm::mat4 m_view(1.0);
  build_rotmatrix_glm (m_view, view_quat);
#if 0
  // SGI and glm have different order!
  m_view = glm::toMat4(glm::quat(view_quat[3],
                                 -view_quat[0],
                                 -view_quat[1],
                                 -view_quat[2]));
#endif
  m_world = glm::translate(m_world*m_view, -pivot);
}

void MeshViewer::view_port_to_world(glm::vec3 view_port_coord,
                                    // output
                                    glm::vec3& world_coord)
{
  auto t_camera = glm::translate(glm::mat4(1.0), m_camera);

  auto Mmv = t_camera * m_world;
  auto Mpmv = m_proj_matrix * Mmv;
  auto M = glm::inverse(Mpmv);
  glm::vec4 v = M * glm::vec4(view_port_coord.x,
                              view_port_coord.y,
                              view_port_coord.z,
                              1.0);
                  
  world_coord = glm::vec3(v.x/v.w,v.y/v.w,v.z/v.w);
}

void MeshViewer::set_show_edge(bool show_edge)
{
  m_show_edge = show_edge;
  queue_render();
}

void MeshViewer::set_show_layer(int layer_id, bool show_layer)
{
  if (layer_id < (int)m_show_layer.size())
    {
      m_show_layer[layer_id] = show_layer;
      refresh_from_settings(false);
    }
}

void MeshViewer::set_orthonormal(bool orthonormal)
{
  m_orthonormal = orthonormal;
  setup_projection_matrix();
  queue_render();
}

void MeshViewer::set_show_matcap(bool show_matcap)
{
  m_show_matcap = show_matcap;
  queue_render();
}

void MeshViewer::refresh_from_settings(bool update_view)
{
  spdlog::info("refresh_from_settings");

  Gdk::RGBA background_color(
    m_pomelo_settings->get_string_default("background_color",
                                          "#606080"));
  m_background = {background_color.get_red(),
    background_color.get_green(),
    background_color.get_blue()};

  Gdk::RGBA mesh_color(
    m_pomelo_settings->get_string_default("mesh_color",
                                          "#c0c0c0"));
  // TBD - a list of colors for each mesh
  m_mesh_color[0] = {
    mesh_color.get_red(),
    mesh_color.get_green(),
    mesh_color.get_blue()};

  //spdlog::info("updating matcap");
  //update_matcap();
  spdlog::info("setting meshes");
  set_meshes(m_meshes, update_view);
  spdlog::info("queue render");
  queue_render();

  spdlog::info("done refresh_from_settings");
}
