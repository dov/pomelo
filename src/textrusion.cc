// A prototype for turning a pango markup string into contours.
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>
#include "textrusion.h"
#include "svgpath-to-cairo.h"
#include "pango-to-cairo.h"

using namespace std;
using namespace Glib;
using namespace fmt;

// Create a pango context from a svg filename
Cairo::RefPtr<Cairo::Context> TeXtrusion::svg_filename_to_context(const string& filename)
{
  auto surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 5000, 5000);
  auto cr = Cairo::Context::create(surface);

  svgpaths_to_cairo(cr->cobj(), filename.c_str(), true);

  return cr;
}

// Take a pango markup and turn it into a cairo context that is returned
Cairo::RefPtr<Cairo::Context> TeXtrusion::markup_to_context(const string& markup)
{
  auto surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 5000, 500);
  auto cr = Cairo::Context::create(surface);

  pangomarkup_to_cairo(cr->cobj(),       
                       markup.c_str(),
                       font_description.gobj());
  return cr;
#if 0
  PangoFontMap *fm;
  fm = pango_ft2_font_map_new();
  RefPtr<Pango::FontMap> fontmap = Glib::wrap(fm);
  RefPtr<Pango::Context> context = fontmap->create_context();

  // TBD: Remove this to see if works on windows
  //  font_description = Pango::FontDescription("DejaVu Serif Bold 48");

  context->set_font_description(font_description);

  auto surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 5000, 500);
  auto cr = Cairo::Context::create(surface);
  RefPtr<Pango::Layout> layout = Pango::Layout::create(cr);
  layout->set_wrap(Pango::WRAP_WORD);
  layout->set_width(10000*PANGO_SCALE);
  layout->set_font_description(font_description);
  layout->get_context()->set_language(Pango::Language("C"));
  layout->get_context()->set_base_dir (do_rtl ? Pango::Direction::DIRECTION_RTL : Pango::Direction::DIRECTION_LTR);
  layout->set_alignment(Pango::ALIGN_CENTER); // Make configurable
  layout->set_markup(markup);

  cr->translate(0,0);
  //  cr->scale(0.1,0.1); // This scaling is done to prevent clipping in the
                      // cairo layout. The linear limit is changed in
                      // in correspondance.
  cr->move_to(0,0);
  cr->set_source_rgb(0,0,0);

  // TBD - Use pango and the more low level function
  // pango-cairo_glyph_string() path to retain the information of the
  // glyphs ligatures etc. This may be used for caching the skeletons
  // per cluster. It will also allow fix the currently wasteful O(n**2)
  // searching of which holes belong to which outer skeleton!
  //
  // To do this properly we need to merge this routine with the
  // TeXtrusion::cairo_path_to_polygons() and
  // TeXtrusion::polys_to_polys_with_holes().
  pango_cairo_update_layout(cr->cobj(), layout->gobj());
  PangoLayoutIter *layout_iter = pango_layout_get_iter(layout->gobj());
  float line_spacing = pango_layout_get_line_spacing(layout->gobj());
  do {
    do {
      print("showing a glyph item\n");
      PangoGlyphItem *glyph_item = pango_layout_iter_get_run(layout_iter);
      
      // TBD - add one glyph at a time
      print("TBD: Layout {} glyphs\n", glyph_item->glyphs->num_glyphs);
  
      for (int i=0; i<glyph_item->glyphs->num_glyphs; i++)
        {
          PangoGlyphInfo *glyph_info = glyph_item->glyphs->glyphs+i;
          PangoGlyphString gs = {1,
            glyph_info,
            glyph_item->glyphs->log_clusters + i};
          
          // Move to the position of the glyph
          cr->rel_move_to(glyph_info->geometry.x_offset/PANGO_SCALE,
                          glyph_info->geometry.y_offset/PANGO_SCALE);
          pango_cairo_glyph_string_path(cr->cobj(),
                                        glyph_item->item->analysis.font,
                                        &gs);
          cr->rel_move_to(-glyph_info->geometry.x_offset/PANGO_SCALE,
                          -glyph_info->geometry.y_offset/PANGO_SCALE);
          cr->rel_move_to(glyph_info->geometry.width/PANGO_SCALE,0);
        }
    } while(!pango_layout_iter_next_run(layout_iter));
    cr->rel_move_to(0, line_spacing/PANGO_SCALE);
  } while(!pango_layout_iter_next_line(layout_iter));

  pango_layout_iter_free(layout_iter);

#if 0
  // The following is equivalent
  pango_cairo_layout_path(cr->cobj(), layout->gobj());
#endif

  return cr;
#endif
}

// Convert a cairo path to a vector of CGAL polygons.
//
//   TBD:
//     - For each vertex retain its normal from the bezier curve
// 
vector<Polygon_2>
TeXtrusion::cairo_path_to_polygons(Cairo::RefPtr<Cairo::Context>& cr)
{
  vector<Polygon_2> polys;
  Polygon_2 poly;
  
  cr->set_tolerance(linear_limit*1e-2); 
  Cairo::Path *path = cr->copy_path_flat();
  cairo_path_t *cpath = path->cobj();
  double eps = 1e-5;
  double eps2 = eps*eps;
   
  for (int i=0; i < cpath->num_data; i += cpath->data[i].header.length) {
      if (updater && i%100)
          if (updater->info("cairo path to polygons", 1.0*i/cpath->num_data))
              throw EAborted("Aborted!");
      auto data = &cpath->data[i];
      switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO:
      case CAIRO_PATH_LINE_TO:
          poly.push_back(Point_2(data[1].point.x,data[1].point.y));
          break;
      case CAIRO_PATH_CURVE_TO:
          // No curve support. Just draw a line to last point
          printf("Warning! We shouldn't be here! We should be flattened!!\n");
          break;
      case CAIRO_PATH_CLOSE_PATH:
          // Optionally pop of a point to ensure the polygon is simple.
          size_t n = poly.size();
          if (CGAL::squared_distance(poly[0], poly[n-1]) < eps2)
              poly.erase(poly.vertices_end()-1);

          if (!poly.is_simple()) {
              printf("Oops! The resulting polygon isn't simple!!\n");
#if 0
              ofstream of("not_simple.giv");
              of << format("$marks fcircle\n");
              for (auto it = poly.vertices_begin(); it != poly.vertices_end(); ++it)
              {
                auto& p = *it;
                //              for (const auto &p : poly)
                  of << format("{:f} {:f}\n",
                               p.x(), p.y());
              }
              of << "z\n";
#endif
          }
                   
          polys.push_back(poly);
          poly.clear(); 
          break;
      }
  }
  print("cpath->num_data={} poly_size={}\n", cpath->num_data, polys.size());
  if (updater)
      if (updater->info("cairo path to polygons", 1.0))
          throw EAborted("Aborted!");

  // Get rid of collinear points. TBD move into a routine
  for (auto& poly : polys) {
      // search for a degenerate sliver angle and remove the vertex
      size_t n = poly.size();
      for (size_t i=0; i<n; i++) {
          const auto& pp = poly[(i-1+n)%n];
          const auto& pn = poly[(i+1)%n];

          // My non exact collinearity test
          if (CGAL::squared_distance(poly[i],Line_2(pp,pn)) < eps2) {
              poly.erase(poly.vertices_begin()+i);
              i-=1;
              n-=1;
          }
      }
  }

  return polys;
}

// The routine polys_to_polys_with_holes() is called after
// cairo_path_to_polygon(). It uses geometric searching and 
// inclusion testing to separate the incoming polygons to
// a list of polygons with holes. It also fixes the polygon
// directions so that outer polygons are counter clock wise 
// and holes are clock wise.
//
// This algorithm is currently a very unefficient O(n**2)
// algorithm. This routine should be joined with the previous
// two routines by using lower level cairo and pango routine
// that retain the correspondance between the outer polygons
// and their holes!
vector<Polygon_with_holes>
TeXtrusion::polys_to_polys_with_holes(vector<Polygon_2> polys)
{
    vector<Polygon_with_holes> polys_with_holes;
    vector<Polygon_2> polys_outer, polys_holes;

    // Brute force check polygon to polygon inclusion in O(n**2).
    // This can be speeded up by spatial datastructures!
    set<int> hole_indices;
    for (size_t i=0; i<polys.size(); i++) {
        const auto& poly = polys[i];
        for (size_t j=0; j<polys.size(); j++) {
            if (i==j || hole_indices.count(j))
                continue;
            if (poly.bounded_side(polys[j][0]) == CGAL::ON_BOUNDED_SIDE)
                hole_indices.insert(j);
        }
    }

    // Partition by what was found in the previous step
    for (size_t i=0; i<polys.size(); i++) {
        auto& poly = polys[i];
        if (hole_indices.count(i)) {
            if (poly.orientation()>0)
                poly.reverse_orientation();
            polys_holes.push_back(poly);
        }
        else {
            if (poly.orientation()<0)
                poly.reverse_orientation();
            polys_outer.push_back(poly);
        }
    }
            

    // Optionally save the cairo paths colored by the direction.
    if (do_save_cairo_paths) {
        ofstream fh("cairo_paths.giv");
        array<string,2> color = {"red","green"};
  
        int poly_idx=0;
        for (auto &poly : polys) {
            fh << format("$arrow end\n"
                         "$marks fcircle\n"
                         "$line\n"
                         "$color {}\n"
                         "$lw 2\n"
                         "$balloon orientation {}\n"
                         "$path poly/{}\n",
                         color[poly.orientation()<0],
                         poly.orientation(),
                         poly_idx++);

            for (auto it=poly.vertices_begin(); it!= poly.vertices_end(); ++it) {
                auto&p = *it;
            //            for (auto&p : poly) 
                fh << format("{:.7f} {:.7f}\n", p.x(), p.y());
            }
            fh << "z\n\n";
        }
    }


    // Build the polygons with holes. Must do it with exact arithmetics
    // This again checks inclusion from the previous steps. This should
    // be cached so that we don't need to redo it.
    int hole_idx=0; // This is increasing
    for (const Polygon_2& poly : polys_outer) {
        Polygon_with_holes polygon_with_holes(poly);

        // This assumes ordering. Not sure this always holds.
        // A worst case scenario is testing for inclusion of
        // all paths.
        while(hole_idx < (int)polys_holes.size()
              && poly.bounded_side(polys_holes[hole_idx][0]) == CGAL::ON_BOUNDED_SIDE) {
            polygon_with_holes.add_hole(polys_holes[hole_idx]);
            hole_idx++;
        }

        // Check if there is an overlap with the previous glyph (a ligature)
        if (polys_with_holes.size()) {
            PolygonE_with_holes unionER;
            Polygon_with_holes unionR;

            // TBD - check bounding boxes...

            PolygonE pA, pB;
            auto& outer_boundary = polys_with_holes[polys_with_holes.size()-1].outer_boundary();
            for (auto it = outer_boundary.vertices_begin(); it!=outer_boundary.vertices_end(); ++it) {
                const auto& p = *it;
              //            for (const auto& p : outer_boundary)
                pA.push_back(PointE2(p.x(), p.y()));
            }
            for (auto it = poly.vertices_begin(); it!= poly.vertices_end(); ++it) {
                const auto& p = *it;
            //            for (const auto& p : poly)
                pB.push_back(PointE2(p.x(), p.y()));
            }
            if (CGAL::join (pA,pB,
                            // output
                            unionER)) {
                Polygon_with_holes prev = polys_with_holes.back();
                polys_with_holes.pop_back();
                Polygon_with_holes next = polygon_with_holes;

                // Convert the result back to inexact...
                Polygon_2 pp, hh;

                auto& outer_boundary = unionER.outer_boundary();
                for (auto it = outer_boundary.vertices_begin(); it!=outer_boundary.vertices_end(); ++it) {
                    auto &p = *it;

                    //                for (const auto &p : outer_boundary)
                    pp.push_back(Point_2(CGAL::to_double(p.x()),CGAL::to_double(p.y())));
                }
                polygon_with_holes = Polygon_with_holes(pp);
                
                for (auto it=unionER.holes_begin(); it!= unionER.holes_end(); ++it) {
                    auto& h = *it;
                //                for (auto &h : unionER.holes()) {
                    hh.clear();
                    for (auto it = h.vertices_begin(); it!= h.vertices_end(); ++it) {
                        auto& p = *it;
                    //                    for (const auto&p : h) {
                        hh.push_back(Point_2(CGAL::to_double(p.x()),CGAL::to_double(p.y())));
                    }
                    polygon_with_holes.add_hole(hh);
                }
                
                for (auto it = prev.holes_begin(); it!= prev.holes_end(); ++it) {
                    auto& h = *it;
                  //                for (auto &h : prev.holes())
                    polygon_with_holes.add_hole(h);
                }
                for (auto it = next.holes_begin(); it!= next.holes_end(); ++it) {
                    auto& h = *it;
                    //                for (auto &h : next.holes())
                    polygon_with_holes.add_hole(h);
                }

            }
        }
            
        polys_with_holes.push_back(polygon_with_holes);
    }

    return polys_with_holes;
}

// Skeletonize the pholes one by one.
vector<PHoleInfo> TeXtrusion::skeletonize(const std::vector<Polygon_with_holes>& polys_with_holes,
                                          // output
                                          std::string& giv_string
                                          )
{
    stringstream ss;

    vector<PHoleInfo> phole_infos;

    for (int ph_idx=0; ph_idx < (int)polys_with_holes.size(); ph_idx++) {
        if (updater &&
            updater->info("skeletonize", 1.0*ph_idx/polys_with_holes.size()))
            throw EAborted("Aborted!");

        const auto& ph = polys_with_holes[ph_idx];

        PHoleInfo phi(ph);
        phi.skeletonize();
        phi.divide_into_regions();
        phole_infos.push_back(phi);
    }
    if (updater && updater->info("skeletonize", 1.0))
        throw EAborted("Aborted!");

    // Create a skeleton giv path. This is a replication of the code
    // below and should be merged
    for (int ph_idx=0; ph_idx < (int)phole_infos.size(); ph_idx++) {
        auto& phi = phole_infos[ph_idx];
        for (auto &r : phi.regions) {
            string path = "skeleton";
            string color = "blue";

            if (!r.polygon.is_simple()) {
                path += "/not_simple";
                color = "orange";
            }


            ss << format("$color {}\n"
                         "$line\n"
                         "$marks fcircle\n"
                         "$path boundary\n"
                         "{} {}\n"
                         "{} {}\n\n"
                         "$color red\n"
                         "$line\n"
                         "$marks fcircle\n"
                         "$path {}\n"
                         ,
                         color,
                         r.polygon[0].x(), r.polygon[0].y(),
                         r.polygon[1].x(), r.polygon[1].y(),
                         path);
            // Inner skeleton lines
            int n = r.polygon.size();
            for (size_t i=1; i<r.polygon.size()+1; i++) 
                ss << format("{} {}\n", r.polygon[i%n].x(), r.polygon[i%n].y());
            ss << ("\n");
        }

    }
    giv_string = ss.str();

    return phole_infos;
}

// TBD restorethis!

// Turn the skeleton into a 3D mesh
Mesh TeXtrusion::skeleton_to_mesh(const vector<PHoleInfo>& phole_infos,
                                  // output
                                  string& giv_string
                                  )
{
    // Always save the string stream and provide it to the skeleton
    // viewer!
    stringstream ss;

    Mesh mesh; // A container for the 3D mesh

    // The upper surface and lower surfaces. The upper surface
    // is bumped, and the lower is flat, but we are using the same
    // triangulation for both.
    double offset_thickness = 0.5;
    for (int ph_idx=0; ph_idx < (int)phole_infos.size(); ph_idx++) {
        if (updater && updater->info("profile", 1.0*ph_idx/phole_infos.size()))
            throw EAborted("Aborted!");

        auto& phi = phole_infos[ph_idx];
        int r_idx = -1;
        for (auto &r : phi.regions) {
            r_idx++;
            double depth = r.get_depth();
            string color = "blue";
            string path_modifier;

            if (!r.polygon.is_simple()) {
                path_modifier += " (not_simple)";
                color = "orange";
            }


            ss << format("$color {}\n"
                         "$line\n"
                         "$marks fcircle\n"
                         "$path boundary{}/ph {}/region {}\n"
                         "{} {}\n"
                         "{} {}\n"
                         "\n"
                         "$color red\n"
                         "$line\n"
                         "$marks fcircle\n"
                         "$path skeleton{}/ph {}/region {}\n\n"
                         ,
                         color,
                         path_modifier,
                         ph_idx+1,
                         r_idx+1,
                         r.polygon[0].x(), r.polygon[0].y(),
                         r.polygon[1].x(), r.polygon[1].y(),
                         path_modifier,
                         ph_idx+1,
                         r_idx+1);
            // Inner skeleton lines
            int n = r.polygon.size();
            for (size_t i=1; i<r.polygon.size()+1; i++) 
                ss << format("{} {}\n", r.polygon[i%n].x(), r.polygon[i%n].y());
            ss << ("\n");

            if (!r.polygon.is_simple()) {
                cerr << format("Oops! The polygon isn't simple. Skipping!\n");
                continue;
            }

            // The upper surface Loop over the offsets
            double epsilon = 1e-5;
            double angle_span = profile_round_max_angle;
            for (int d_idx=0; d_idx<this->profile_num_radius_steps+1; d_idx++) {
                double angle_start = angle_span * d_idx/this->profile_num_radius_steps;
                double angle_end = angle_span * (d_idx+1)/this->profile_num_radius_steps;
                double offs_start = profile_radius*(1-cos(angle_start));
                double offs_end = profile_radius*(1-cos(angle_end));
                if (d_idx == this->profile_num_radius_steps)
                  offs_end = depth+epsilon;

                auto pp = r.get_offset_curve_and_triangulate(offs_start,offs_end);
                int poly_idx = 0;
                for (const auto &poly : pp) {
                    ss << format("$color green\n"
                                 "$line\n"
                                 "$marks fcircle\n"
                                 "$path offset curves/ph {}/region {}/{}/{}\n"
                                 ,
                                 ph_idx+1,
                                 r_idx+1,
                                 d_idx+1,
                                 poly_idx+1
                                 );
                    poly_idx++;
                    if (poly.size()!=3)
                        throw std::runtime_error("Expected 3 vertices!");
                    glm::vec3 tri[3], tri_back[3];
                    for (int i=0; i<3; i++) {
                        const auto &p = poly[i];

                        double z = p.z();

                        // Transform z
                        double r = profile_radius;
                        if (z > r - r * cos(angle_span))
                            z = r * sin(angle_span);
                        else 
                            z = sqrt(r*r-(z-r)*(z-r));
                        
                        // Do the 2-i to get the correct direction
                        // for light from the upper side
                        tri[2-i] = glm::vec3 {
                            float(p.x()),
                            -float(p.y()), 
                            float(z) };
                        ss << format("{} {}\n", p.x(), p.y());

                        // Back side
                        tri_back[i] = glm::vec3 {
                            float(p.x()),
                            -float(p.y()), 
                            float(-zdepth) };
                    }
                    ss << "z\n";
                    ss << "\n";

                    // Add triangles to mesh
                    for (int i=0; i<3; i++)
                      mesh.vertices.push_back(tri[i]);
                    for (int i=0; i<3; i++)
                      mesh.vertices.push_back(tri_back[i]);
                }
            }

            // Add a tube between upper and lower segment (quad)
            {
                auto& p = r.polygon;

                mesh.vertices.push_back( { float(p[0].x()),float(-p[0].y()),0.0f });
                mesh.vertices.push_back( { float(p[1].x()),float(-p[1].y()),0.0f });
                mesh.vertices.push_back( { float(p[0].x()),float(-p[0].y()),float(-zdepth)});
                mesh.vertices.push_back( { float(p[1].x()),float(-p[1].y()),0.0f});
                mesh.vertices.push_back( { float(p[1].x()),float(-p[1].y()),float(-zdepth)});
                mesh.vertices.push_back( { float(p[0].x()),float(-p[0].y()),float(-zdepth)});
            }
        }
    }
    giv_string = ss.str();

#if 0
    ofstream of("path.giv");
    of << giv_string;
    of.close();
#endif
    if (updater && updater->info("profile", 1.0))
        throw EAborted("Aborted!");

    return mesh;
}

// The following two functions are from:
//   https://doc.cgal.org/latest/Triangulation_2/Triangulation_2_2polygon_triangulation_8cpp-example.html
static void
mark_domains(CDT& ct,
             Face_handle start,
             int index,
             std::list<CDT::Edge>& border )
{
  if(start->info().nesting_level != -1){
    return;
  }
  std::list<Face_handle> queue;
  queue.push_back(start);
  while(! queue.empty()){
    Face_handle fh = queue.front();
    queue.pop_front();
    if(fh->info().nesting_level == -1){
      fh->info().nesting_level = index;
      for(int i = 0; i < 3; i++){
        CDT::Edge e(fh,i);
        Face_handle n = fh->neighbor(i);
        if(n->info().nesting_level == -1){
          if(ct.is_constrained(e)) border.push_back(e);
          else queue.push_back(n);
        }
      }
    }
  }
}

//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void
mark_domains(CDT& cdt)
{
  for (auto it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it) {
    //  for(CDT::Face_handle f : cdt.all_face_handles()){
    it->info().nesting_level = -1;
  }
  std::list<CDT::Edge> border;
  mark_domains(cdt, cdt.infinite_face(), 0, border);
  while(! border.empty()){
    CDT::Edge e = border.front();
    border.pop_front();
    Face_handle n = e.first->neighbor(e.second);
    if(n->info().nesting_level == -1){
      mark_domains(cdt, n, e.first->info().nesting_level+1, border);
    }
  }
}

// Convert the member polygon_with_holes into a skeleton
void PHoleInfo::skeletonize()
{
    skeleton = CGAL::create_interior_straight_skeleton_2(polygon_with_holes);
}

// Divide the member skeleton into SkeletonPolygonRegions
void PHoleInfo::divide_into_regions()
{
    for (auto it = skeleton->halfedges_begin(); it != skeleton->halfedges_end(); ++it) {
        // Find a boundary edge
        if (it->is_bisector())
            continue;

        Polygon_2 poly;
        // The first points are on the boundary
        poly.push_back(it->opposite()->vertex()->point());
        poly.push_back(it->vertex()->point());

        // The rest of the skeleton lines
        for (auto it2 = it->next(); it2->is_bisector(); it2=it2->next()) {
            auto p= it2->vertex()->point();
            poly.push_back(p);
        }

        // Cleanup the polygons
        double eps2 = 1e-10;
        if (CGAL::squared_distance(poly[poly.size()-1], poly[0])<eps2)
            poly.erase(poly.vertices_begin()+poly.size()-1);

        // search for a degenerate sliver angle and remove the vertex
        size_t n = poly.size();
        for (size_t i=0; i<n; i++) {
            const auto& pp = poly[(i-1+n)%n];
            const auto& pn = poly[(i+1)%n];

            // My non exact collinearity test
            if (CGAL::squared_distance(poly[i],Line_2(pp,pn)) < eps2) {
                poly.erase(poly.vertices_begin()+i);
                i-=1;
                n-=1;
            }
        }
        
        // This should n't happen anymore
        if (!poly.is_simple()) {
            printf("Oops! Poly isn't simple. Saving to not-simple.giv\n");

#if 0
            ofstream of("not-simple.giv");
            of << format("$marks fcircle\n"
                         "$color red\n"
                         );
            for (auto it=poly.vertices_begin(); it!= poly.vertices_end(); ++it) {
                auto& p = *it;
            //            for (const auto &p : poly) {
                of << format("{:f} {:f}\n", p.x(), p.y());
            }
            of << "z\n";
            of.close();
#endif
        }

        if (poly.size()>2) { // Why do I get smaller lines??
            // Drop duplicate last point
            SkeletonPolygonRegion region(poly);
            regions.push_back(region);
        }
    }
}

double SkeletonPolygonRegion::get_depth() const
{
    Line_2 line(polygon[0],polygon[1]);
    double max_depth = 0;

    for (size_t i=2; i<polygon.size(); i++) {
        double dline = sqrt(CGAL::to_double(CGAL::squared_distance(line,polygon[i])));
        if (dline > max_depth)
            max_depth = dline;
    }

    return max_depth;
}

// Sutherland-Hodgman polygon clipping
vector<Polygon_2> cut_polygon_by_line(const Polygon_2& poly,
                                      const Line_2& line)
{
    vector<Polygon_2> cuts; // output polygons
    Polygon_2 cut; // The current cutting polygon

    size_t poly_size = poly.size();
    vector<bool> cut_on_boundary;
    for (size_t i=0; i<poly.size(); i++) {
        const Point_2& p = poly[i%poly_size];
        const Point_2& q = poly[(i+1)%poly_size];

        // case 1
        if (line.has_on_positive_side(p)
            && line.has_on_positive_side(q)) {
            cut.push_back(q);
            cut_on_boundary.push_back(false);
        }
        // case 2
        else if (!line.has_on_positive_side(p)
                 && line.has_on_positive_side(q)) {
            auto result = CGAL::intersection(Line_2(p,q), line);

            // starts a new polygon
            if (result) {
                if (const Point_2* pi = boost::get<Point_2 >(&*result)) {
                    cut.push_back(*pi);
                    cut_on_boundary.push_back(true);
                }
            }
            cut.push_back(q); 
            cut_on_boundary.push_back(false);
        }
        // case 3
        else if (line.has_on_positive_side(p)
                 && !line.has_on_positive_side(q)) {
            auto result = CGAL::intersection(Line_2(p,q), line);
            if (result) {
                if (const Point_2* pi = boost::get<Point_2 >(&*result)) {
                    cut.push_back(*pi);
                    cut_on_boundary.push_back(true); 
                }
            }
            else {
                cut.push_back(q); // collinear
                cut_on_boundary.push_back(true);
            }

        }
        else // case 4: both are outside igonre
            ;
    }

    // Break apart cut by removing the line segments of the cut line
    // that are outside the original polygon.

    // Find a point of cut that is on line and whose next point
    // is not on the line.

    size_t cut_size = cut.size();
    int start = 0;
    for (size_t i=0; i<cut.size(); i++) {
        int ie = (i+1)%cut_size;
        if (cut_on_boundary[i] && !cut_on_boundary[ie]) {
            start = i;
            break;
        }
    }

    // now cut apart cut into multiple cuts by checking what
    // points we inserted that were on the boundary through
    // cut_on_boundary.
    Polygon_2 cut2;

    auto poly_orientation = poly.orientation();
    for (size_t i=0; i<cut.size(); i++) {
        int is = (start+i)%cut_size;
        int ie = (start+i+1)%cut_size;
        const Point_2& p = cut[is];

        cut2.push_back(p);

        // Heuristics for cutting the cut polygon! Only prune 
        // polygons with the same orientation as the original
        // polygon.
        if (cut_on_boundary[is]
            && cut_on_boundary[ie]) {
            // We've got a candidate. Check if it is simple 
            if (cut2.is_simple()
                && cut2.orientation() == poly_orientation) {
                cuts.push_back(cut2);
                cut2.clear();
            }
        }
    }

    if (cut2.size())
        cuts.push_back(cut2);
        

    return cuts;
}

// Get a slice of the polygon between the line pairs line1 and
// line2. Line1 and line2 should be properly oriented!
vector<Polygon_2> cut_polygon_by_line_pair(const Polygon_2& poly,
                                           const Line_2& line1,
                                           const Line_2& line2)
{
    vector<Polygon_2> ret;
    vector<Polygon_2> cuts1 = cut_polygon_by_line(poly,line1); // output polygons

    for (const auto& p1 : cuts1) {
        vector<Polygon_2> cuts2 = cut_polygon_by_line(p1,line2);
        for (const auto& p2 : cuts2) 
            ret.push_back(p2);
    }
    return ret;
}

// TBD: This needs to be modified when adding support for ridge smoothing.
// It is not enough to cut the polygon by the polygon pairs. We need to
// cut it again for offset curves around all the non boundary polygon
// edges.
//
// Further the output can no longer be only z, but we also need the
// distance from the non-boundary edge.
//
// Once we have this info, we have enough to cut the edges.

vector<Polygon3D> SkeletonPolygonRegion::get_offset_curve(double d1, double d2) const
{
    Line_2 boundary_line(polygon[0],polygon[1]); // the boundary curve
    Vector_2 vec = boundary_line.perpendicular(polygon[0]).to_vector();
    vec = vec/CGAL::sqrt(vec.squared_length()); // normalize
    Line_2 line1(polygon[0] + vec*d1, polygon[1] + vec*d1);
    Line_2 line2 = Line_2(polygon[0] + vec*d2, polygon[1] + vec*d2).opposite();

    vector<Polygon3D> ret;
    auto polys = cut_polygon_by_line_pair(polygon, line1, line2);

    for (auto& poly : polys) {
        Polygon3D poly3;
        for (auto it=poly.vertices_begin(); it!= poly.vertices_end(); ++it) {
            auto& p = *it;
        //        for (auto& p : poly) {
            double z = sqrt(CGAL::squared_distance(p, boundary_line));
            poly3.emplace_back(p.x(), p.y(), z);
        }
        ret.push_back(poly3);
    }

    return ret;
}

vector<Polygon3D> SkeletonPolygonRegion::get_offset_curve_and_triangulate(double d1, double d2) const
{
    auto polys = get_offset_curve(d1,d2);
    Line_2 boundary_line(polygon[0],polygon[1]); // the boundary curve

    vector<Polygon3D> tris3;
    for (const auto& poly3 : polys) {
        // build a "normal" poly from the polys. We'll build the 3D coordinates
        // from the distance to the base line.
        Polygon_2 poly;
        for (const auto&p : poly3)
            poly.push_back(Point_2(p.x(),p.y()));
    
        CDT cdt;
        cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
        mark_domains(cdt);
        for (auto it = cdt.finite_faces_begin(); it != cdt.finite_faces_end(); ++it) {
            auto& f = *it;
            //        for (const Face_handle& f : cdt.finite_face_handles()) {
            if (!it->info().in_domain())
                continue;
            // For each vertex measure its distance to the boundary
            // line and insert it as z
            Polygon3D tri3;
            for (int i=0; i<3; i++) {
                const auto &p = f.vertex(i)->point();
                double z = sqrt(CGAL::squared_distance(
                                  p, boundary_line));
                tri3.emplace_back(p.x(),p.y(),z);
            }
            tris3.push_back(tri3);
        }
    }
  
    return tris3;
}
  
void poly_to_giv(ustring filename,
                  ustring header,
                  Polygon_2 poly,
                  bool append)
{
    auto flags = ofstream::out;
    if (append)
        flags |= ofstream::app;
    ofstream of(filename, flags);
    of << header;
    of << "m ";
    for (auto it = poly.vertices_begin(); it!= poly.vertices_end(); ++it) {
        auto& p = *it;
    //    for (const auto& p  : poly)
        of << p.x() << " " << p.y() << "\n";
    }
    of << "z\n\n";
    of.close();
}
                     
