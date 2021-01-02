// A basic viewer that shows a fixed triangular mesh.

#include "mesh-viewer.h"
#include <iostream>
#include <limits>
#include <fmt/core.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include "trackball.h"

using namespace std;
using namespace fmt;
using namespace glm;

#define VIEW_INIT_AXIS_X 1.0
#define VIEW_INIT_AXIS_Y 0.0
#define VIEW_INIT_AXIS_Z 0.0
#define VIEW_INIT_ANGLE  20.0
#define VIEW_SCALE_MAX 100.0
#define VIEW_SCALE_MIN 0.001

#define DIG_2_RAD (G_PI / 180.0)
#define RAD_2_DIG (180.0 / G_PI)

// Constructor
MeshViewer::MeshViewer()
{
  this->add_events (
    Gdk::EventMask(GDK_POINTER_MOTION_MASK    |
                   GDK_BUTTON_PRESS_MASK      |
                   GDK_SCROLL_MASK      |
                   GDK_VISIBILITY_NOTIFY_MASK 
                   ));


  // Setup the project matrix. Currently static
  double thetaInDeg=8.0;
  double near=1;
  double far=20;
  double width = get_allocation().get_width();
  double height = get_allocation().get_height();
  double aspect = 1.0*width / height; // aspect ratio
  static double PI { 3.141592653589793 };
  float theta = thetaInDeg / 180.0 * PI;
  float range = far - near;
  float invtan = 1./tan(theta/2.);

#if 0
  m_proj_matrix = glm::perspective(thetaInDeg,aspect,near,far);
  m_proj_matrix = glm::lookAt(glm::vec3(0,0,-10),
                              glm::vec3(0,0,0),
                              glm::vec3(0,1,0));

#endif
  m_proj_matrix = glm::mat4(1.0);
  m_proj_matrix[0][0] = invtan / aspect;
  m_proj_matrix[1][1] = invtan;
  m_proj_matrix[2][2] = -(near + far) / range;
  m_proj_matrix[3][2] = -1;
  m_proj_matrix[2][3] = -2 * near * far / range;
  m_proj_matrix[3][3] = 1;
  //  m_proj_matrix = glm::transpose(m_proj_matrix);


  // Some sample static data
  static const struct MeshViewer::VertexInfo vertex_data[] = {
    { {  0.0f,  0.500f, 0.0f }, {1,0,0}, {0,0,1}, {1,0,0} },
    { {  0.5f, -0.366f, 0.0f }, {0,1,0}, {0,0,1}, {0,1,0} },
    { { -0.5f, -0.366f, 0.0f }, {0,0,1}, {0,0,1}, {0,0,1} },
  };

  m_hw_mesh.vertices.clear();

  for (auto v : vertex_data)
    m_hw_mesh.vertices.push_back(v);

}

// Describe geometry. This is a bit wasteful, but I don't expect
// to show huge meshes in Pomelo in any case

// Returns the shader
static guint
create_shader (int shader_type,
               const Glib::RefPtr<const Glib::Bytes>& source)
{
  guint shader = glCreateShader (shader_type);
  
  // Get pointer and size
  gsize len=0;
  const char* src = (const char*)source->get_data(len);

  // Turn into GL structure
  int lengths[] = { (int)len };
  glShaderSource (shader, 1, &src, lengths);
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

      throw runtime_error(
        format(
          "Compilation failure in {} shader: {}",
          shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
          error_string));
    }

  return shader;
}

void MeshViewer::init_shaders ()
{
  // TBD - init the shaders
  /* load the vertex shader */
  Glib::RefPtr<const Glib::Bytes> vertex_source = Gio::Resource::lookup_data_global("/shaders/vertex.glsl");
  Glib::RefPtr<const Glib::Bytes> fragment_source = Gio::Resource::lookup_data_global("/shaders/fragment.glsl");
  
  guint vertex = create_shader(GL_VERTEX_SHADER, vertex_source);
  guint fragment = create_shader(GL_FRAGMENT_SHADER, fragment_source);
  m_program = glCreateProgram ();
  glAttachShader (m_program, vertex);
  glAttachShader (m_program, fragment);
  glLinkProgram (m_program);

  m_position_index = 0;
  m_color_index = 1;
  m_normal_index = 2;
  m_bary_index = 3;

  glBindAttribLocation(m_program, m_position_index, "position");
  glBindAttribLocation(m_program, m_color_index, "color");
  glBindAttribLocation(m_program, m_normal_index, "normal");
  glBindAttribLocation(m_program, m_bary_index, "bary");

  int status = 0;
  glGetProgramiv (m_program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len = 0;
      glGetProgramiv (m_program, GL_INFO_LOG_LENGTH, &log_len);

      string error_string;
      error_string.resize(log_len+1);
      glGetProgramInfoLog (m_program, log_len, NULL, &error_string[0]);

      glDeleteProgram (m_program);
      throw runtime_error(
        format(
          "Linking failure in program: {}",
          error_string));
    }

  print("pos={} color={} normal={} bary={}\n",
        m_position_index, m_color_index, m_normal_index, m_bary_index);

  /* the individual shaders can be detached and destroyed */
  glDetachShader (m_program, vertex);
  glDetachShader (m_program, fragment);
  glDeleteShader (vertex);
  glDeleteShader (fragment);
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
  print("glBufferData()\n");
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
  print("glBufferData()\n");
  glBufferData (GL_ARRAY_BUFFER,
                m_hw_mesh.vertices.size() * sizeof(MeshViewer::VertexInfo),
                &m_hw_mesh.vertices[0].position[0], GL_DYNAMIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void MeshViewer::on_realize()
{
  m_realized = true;
  printf("On realize...\n");
  Gtk::GLArea::on_realize();

  this->make_current();
  this->set_has_depth_buffer(true);

  init_shaders();

  // Init the buffer
  init_buffers (&m_vao);

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
  try
  {
    throw_if_error();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor (0.4, 0.4, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    draw_mesh();

    glFlush();

  }
  catch(const Gdk::GLError& gle)
  {
    cerr << "An error occurred in the render callback of the GLArea" << endl;
    cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << endl;
    return false;
  }
  

  return true;
  
}

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

void MeshViewer::build_projection_matrix()
{
  double thetaInDeg=8.0;
  double near=1;
  double far=20;
  double width = get_allocation().get_width();
  double height = get_allocation().get_height();
  double aspect = 1.0*width / height; // aspect ratio
  static double PI { 3.141592653589793 };
  float theta = thetaInDeg / 180.0 * PI;
  float range = far - near;
  float invtan = 1./tan(theta/2.);

#if 0
  m_proj_matrix = glm::perspective(thetaInDeg,aspect,near,far);
  m_proj_matrix = glm::lookAt(glm::vec3(0,0,-10),
                              glm::vec3(0,0,0),
                              glm::vec3(0,1,0));

#endif
  m_proj_matrix = glm::mat4(1.0);
  m_proj_matrix[0][0] = invtan / aspect;
  m_proj_matrix[1][1] = invtan;
  m_proj_matrix[2][2] = -(near + far) / range;
  m_proj_matrix[3][2] = -1;
  m_proj_matrix[2][3] = -2 * near * far / range;
  m_proj_matrix[3][3] = 1;
  //  m_proj_matrix = glm::transpose(m_proj_matrix);
}

// Draw the mesh (or meshes)
void MeshViewer::draw_mesh()
{
  setup_world(m_size_scale*m_view_scale, 
              m_view_quat, m_pivot);
  
  glUseProgram(m_program);

  // get the uniform locations
  m_proj_loc = glGetUniformLocation (m_program, "projMatrix");
  m_mv_loc = glGetUniformLocation (m_program, "mvMatrix");
  m_normal_matrix_loc = glGetUniformLocation (m_program, "normalMatrix");
  guint shininess_loc=glGetUniformLocation (m_program, "shininess");
  guint specular_loc=glGetUniformLocation (m_program, "specular");
  guint diffuse_loc=glGetUniformLocation (m_program, "diffuse");
  guint ambient_loc=glGetUniformLocation (m_program, "ambient");

  // Fixed light properties - Should be configurable
  glUniform1f(shininess_loc, 10.0);
  glUniform1f(specular_loc, 0.1);
  glUniform1f(diffuse_loc, 0.8);
  glUniform1f(ambient_loc, 0.2);

  //  m_world[2][1] = 0;
  //  m_world[3] = glm::vec4 {0,0,0,1};
  auto model_view_matrix = glm::translate(glm::mat4(1.0), m_camera) * m_world;

  // Build the projection matrix. Is there a better place?
  build_projection_matrix();

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

    view_port_to_world(glm::vec3(src_x,src_y,0), // z=0.5*(near+far)
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

void MeshViewer::set_mesh(shared_ptr<Mesh> mesh)
{
  vector<vec3>& vertices = mesh->vertices; // shortcut
  auto& bbox = m_hw_mesh.bbox; // shortcut
  m_hw_mesh.vertices.clear();
  if (vertices.size() % 3 != 0)
    throw runtime_error("Expected number of vertices to a multiple of 3!");

  for (int k=0; k<3; k++) {
    bbox[k] = numeric_limits<float>::infinity();
    bbox[k+3] = -1 * numeric_limits<float>::infinity();
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
              vec3(0.8,0.8,0.8),
              normal,
              vec3(j==0,j==1,j==2) // bary
            });
        }
  }

  print("Loaded {} triangles\n", m_hw_mesh.vertices.size()/3);

  // setup the scaling
  double dim_max = 0;
  for (int i=0; i<3; i++){
    double dim = bbox[i+3]-bbox[i];
    if (dim >dim_max)
        dim_max = dim;
  }
  m_size_scale = 1.0/dim_max/1.44 * 2; // Not sure why I need the two here...
  
  // Pivot point for rotating around the center of the object.
  m_pivot = vec3 {
    0.5*(bbox[0]+bbox[3]),
    0.5*(bbox[1]+bbox[4]),
    0.5*(bbox[2]+bbox[5])};

  print("bbox = {} {} {} {} {} {}\n",
        bbox[0],
        bbox[1],
        bbox[2],
        bbox[3],
        bbox[4],
        bbox[5]);

  //  redraw();
  if (m_vao)
    update_geometry();
}

void MeshViewer::set_mesh_file(const std::string& mesh_filename)
{
  shared_ptr<Mesh> mesh = read_stl(mesh_filename);
  set_mesh(mesh);
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
