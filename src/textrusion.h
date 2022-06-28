//======================================================================
//  textrusion.h - Beveling of text (and svg)
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Dec 31 07:54:07 2020
//----------------------------------------------------------------------
#ifndef TEXTRUSION_H
#define TEXTRUSION_H

// pango and cairo dependencies
#include <glibmm.h>
#include <pangomm.h>
#include <cairo.h>
#include <cairomm/cairomm.h>
#include <pangomm/init.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <pango/pangocairo.h>

// Include CGAL for polygon processing
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Point_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/create_offset_polygons_2.h>
#include <CGAL/create_offset_polygons_from_polygon_with_holes_2.h>
#include <CGAL/squared_distance_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Aff_transformation_2.h>
#include <fmt/core.h>
#include "mesh.h"
#include "profile.h"

// An exception when being aborted
class EAborted : public std::runtime_error {
    public:
    EAborted(const std::string& msg)
        : std::runtime_error(msg) {}
};

// Used when triangulating for finding inside or outside triangles
struct FaceInfo2
{
  FaceInfo2(){}
  int nesting_level;
  bool in_domain(){
    return nesting_level%2 == 1;
  }
};

// Typedef s for my kernels, etc
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_exact_constructions_kernel KernelE;
typedef Kernel::Point_2                                   Point_2;
typedef Kernel::Point_3                                   Point3;
typedef KernelE::Point_2                                  PointE2;
typedef CGAL::Polygon_2<Kernel>                           Polygon_2;
typedef CGAL::Polygon_2<KernelE>                          PolygonE;
typedef std::vector<Point3>                               Polygon3D;
typedef CGAL::Line_2<Kernel>                              Line_2;
typedef CGAL::Vector_2<Kernel>                            Vector_2;
typedef CGAL::Segment_2<Kernel>                           Segment;
typedef CGAL::Polygon_with_holes_2<Kernel>                Polygon_with_holes;
typedef CGAL::Polygon_with_holes_2<KernelE>               PolygonE_with_holes;
typedef CGAL::Aff_transformation_2<Kernel>                Transformation;
typedef CGAL::Straight_skeleton_2<Kernel>                 StraightSkeleton ;
typedef CGAL::Triangulation_vertex_base_2<Kernel>                      Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2,Kernel>    Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Kernel,Fbb>        Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>               TDS;
typedef CGAL::Exact_predicates_tag                                Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel, TDS, Itag>  CDT;
typedef CDT::Face_handle                                          Face_handle;

// Used to update about the progress
class Updater {
    public:
    typedef enum {
        UPDATER_OK = 0,
        UPDATER_ABORT
    } ContinueStatus;
    Updater() {}
    virtual ~Updater() {}
    
    // update_progress returns the progress. If the function returns
    // UPDATER_ABORT then the underlying process should abort.
    virtual ContinueStatus info(const std::string& context, double progress) {
        std::cout << fmt::format("{} {:.0f}%\n", context, progress*100);
        return UPDATER_OK;
    }
};

// A SkeletonPolygonRegion is a polygon with the first line segment on
// the outer boundary of its corresponding PHoleInfo and the rest make
// up the skeleton lines.
//
// It can carry out the following operations:
//   - Triangulate itself (might be changed to quads in the future)
//   - Add itself to a roof mesh.
//   - offset slicing
class SkeletonPolygonRegion {
    public:
    SkeletonPolygonRegion(Polygon_2& polygon)
        : polygon(polygon) {}

    // Add itself to a roof mesh
    void add_to_roof_mesh(Mesh& mesh, double zscale=1.0) const;

    // How deep is the polygon (distance from the edge to the furthest
    // vertex perpendicular from the edge)
    double get_depth() const;

    // Get Offset slice returns a slice of the region polygon between
    // distances d1 and d2 from the boundary curve. The result is returned
    // as a vector of 3D polygons where the z-axis is the distance from
    // the outer boundary curve. (The first segment of the polygon)
    std::vector<Polygon3D> get_offset_curve(double d1, double d2) const;

    // Get offset curve and triangulate it. Each return Polygon3D
    // should contain exactly 3 polygons.
    std::vector<Polygon3D> get_offset_curve_and_triangulate(double d1, double d2) const;
    
    // The polygon region. The first edge lies on he associated PHoleInfo
    // boundary.
    Polygon_2 polygon;
};
    
// A PHoleInfo contain all the information belong to a polygon with holes
// (this sould be merged to GlyphInfo one day so that I can cache the
// information of glyphs!)
// 
//   - Polygon_2 with holes
//   - Skeleton
//   - Offset curves
//   - Triangulaton
class PHoleInfo {
    public:
    PHoleInfo(const Polygon_with_holes& polygon_with_holes)
        : polygon_with_holes(polygon_with_holes) {}

    // Run the skeletonize region on the polygon_with_holes to produce
    // skeleton member.
    void skeletonize();

    // Split the skeletons into SkeletonPolygonRegion:s
    void divide_into_regions();

    Polygon_with_holes polygon_with_holes;
    boost::shared_ptr<StraightSkeleton> skeleton;
    std::vector<SkeletonPolygonRegion> regions;
};


// The algorithmic class carrying out the roofyfication algorithm.
// The user needs to call the algorithms one by one in order to
// generate the final mesh.
class TeXtrusion {
  public:
  // constructor
    TeXtrusion(std::shared_ptr<Updater> updater = nullptr)
      : updater(updater)
    {}

    // Chain of transformations
    void set_updator(std::shared_ptr<Updater> updater) {
      this->updater = updater;
    }
  
    // Create a pango context from a svg filename
    Cairo::RefPtr<Cairo::Context> svg_filename_to_context(const std::string& filename);

    // Create a pango context for pango markup
    Cairo::RefPtr<Cairo::Context> markup_to_context(const std::string& markup);

    // Turn the pango markup into a vector of polygons
    std::vector<Polygon_2> cairo_path_to_polygons(Cairo::RefPtr<Cairo::Context>& cr);

    // Merge the vectors into "glyphs", i.e. polygons with holes. This
    // may revert the orientation of wrongly oriented polygons.
    std::vector<Polygon_with_holes> polys_to_polys_with_holes(std::vector<Polygon_2> polys);

    // Skeletonize the glyphs
    std::vector<PHoleInfo> skeletonize(const std::vector<Polygon_with_holes>& polys_with_holes,
                                       // output
                                       std::string& giv_string);

    // Turn the skeleton into by the layers
    std::vector<Mesh> skeleton_to_mesh(const std::vector<PHoleInfo>& phole_infos,
                                     // output
                                     std::string& giv_string);

    // configuration
    bool do_rtl = false;
    double linear_limit = 500;
    Pango::FontDescription font_description;
    bool use_profile_data = false;
    ProfileData profile_data;
    double profile_radius = 3.0;
    double profile_round_max_angle = M_PI/2;
    double profile_num_radius_steps = 10;
    std::string giv_filename;
    double zdepth = 2;
    bool do_save_cairo_paths = true; // used for debugging

    // Used to update and abort progress
    std::shared_ptr<Updater> updater;

  private:
    void add_region_contribution_to_mesh(
      Mesh& mesh,
      std::stringstream& ss,
      int layer_idx,
      int ph_idx,
      int r_idx,
      const std::vector<Vec2>& prev_flat_list,
      const std::vector<Vec2>& flat_list,
      const SkeletonPolygonRegion& region);
  
};

#endif /* TEXTRUSION */
