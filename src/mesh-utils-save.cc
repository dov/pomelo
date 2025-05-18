//======================================================================
//  mesh-utils.h - Utilities for manipulating meshes
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Jan 30 22:30:25 2025
//----------------------------------------------------------------------

#include "mesh-utils.h"
#include <fmt/core.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/hierarchy_simplify_point_set.h>
#include <vector>
#include <map>
#include <fmt/core.h>

using namespace std;
using fmt::print;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Search_traits_3<Kernel> Traits;
typedef CGAL::Kd_tree<Traits> Kd_tree;
typedef CGAL::Fuzzy_sphere<Traits> Fuzzy_sphere;
typedef CGAL::Point_set_3<Point_3> Point_set;

static Point_3 glm_to_cgal(const glm::dvec3& v) {
    return Point_3(v.x, v.y, v.z);
}

static glm::dvec3 cgal_to_glm(const Point_3& p) {
    return glm::dvec3(p.x(), p.y(), p.z());
}

// Custom comparator for glm::dvec3
struct dvec3_comparator {
    bool operator()(const glm::dvec3& lhs, const glm::dvec3& rhs) const {
        if (lhs.x != rhs.x) return lhs.x < rhs.x;
        if (lhs.y != rhs.y) return lhs.y < rhs.y;
        return lhs.z < rhs.z;
    }
};

struct point3_comparator {
    bool operator()(const Point_3& lhs, const Point_3& rhs) const {
        if (lhs.x() != rhs.x()) return lhs.x() < rhs.x();
        if (lhs.y() != rhs.y()) return lhs.y() < rhs.y();
        return lhs.z() < rhs.z();
    }
};

// This doesn't work!
void merge_by_distance(Mesh& mesh, double distance_threshold)
{
    Kd_tree tree;
    map<Point_3, int, point3_comparator> point_to_index;

    // Insert points into the Kd_tree and map them to their indices
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        Point_3 point = glm_to_cgal(mesh.vertices[i]);
        tree.insert(point);
        point_to_index[point] = i;
    }

    int num_verged_points = 0;
    // vertices map to themself by default
    vector<int> merge_map(mesh.vertices.size());
    for (size_t i = 0; i < merge_map.size(); ++i) {
        merge_map[i] = i;
    }
    print("distance_threshold = {}\n", distance_threshold);

    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] != (int)i) continue; // Skip already merged vertices

        Point_3 point = glm_to_cgal(mesh.vertices[i]);
        Fuzzy_sphere sphere(point, distance_threshold);
        vector<Point_3> neighbors;
        tree.search(back_inserter(neighbors), sphere);

        Point_3 merged_point = point;
        int count = 1;

        for (const auto& neighbor : neighbors) {
            int neighbor_index =
            if (neighbor_index == (int)i // Skip myself
                || (neighbor == point)   // Skip equal points
                || merge_map[neighbor_index] != neighbor_index // skip unless it has been mapped
                )
                continue;
                
            merge_map[neighbor_index] = i; // map the neighbor to this point
            count++;
        }

        if (count > 1) {
//            mesh.vertices[i] = cgal_to_glm(merged_point);
            num_verged_points+= count-1;
        }
    }
    print("num_merged_points = {}\n", num_verged_points);

    // Remove duplicate vertices
    std::vector<glm::dvec3> new_vertices;
    new_vertices.reserve(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] == (int)i) {
            new_vertices.push_back(mesh.vertices[i]);
        } else {
            int m = merge_map[i];
            while (merge_map[m] != m)
                m = merge_map[m];
            new_vertices.push_back(mesh.vertices[m]]);
        }
    }

    // Update mesh.vertices with new_vertices
    mesh.vertices = new_vertices;
}

// For debugging. Count the number of unique vertices
int count_unique_vertices(const Mesh& mesh)
{
    std::set<glm::dvec3, dvec3_comparator> unique_points(mesh.vertices.begin(), mesh.vertices.end());
    return unique_points.size();
}


