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

#if 0
struct Point_with_index {
    Point_3 point;
    int index;

    Point_with_index(const Point_3& p, int i) : point(p), index(i) {}

    // Provide access to the point's coordinates for the kd-tree
    double operator[](std::size_t i) const {
        switch (i) {
            case 0: return point.x();
            case 1: return point.y();
            case 2: return point.z();
            default: throw std::out_of_range("Index out of range");
        }
    }
};
#endif
static Point_3 glm_to_cgal(const glm::dvec3& v) {
    return Point_3(v.x, v.y, v.z);
}

#if 0
static glm::dvec3 cgal_to_glm(const Point_3& p) {
    return glm::dvec3(p.x(), p.y(), p.z());
}
#endif


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

#if 0
// Traits class to use Point_with_index in the kd-tree
struct Custom_traits : public Traits {
    typedef Point_with_index Point_d;
};

typedef CGAL::Kd_tree<Custom_traits> CKd_tree;

int foo() {
    std::vector<Point_with_index> points;
    points.emplace_back(Point_3(1.0, 2.0, 3.0), 0);
    points.emplace_back(Point_3(4.0, 5.0, 6.0), 1);
    points.emplace_back(Point_3(7.0, 8.0, 9.0), 2);

    CKd_tree tree(points.begin(), points.end());

    // Example: Accessing the index of a point in the kd-tree
    for (const auto& p : points) {
        std::cout << "Point: (" << p.point.x() << ", " << p.point.y() << ", " << p.point.z() << "), Index: " << p.index << std::endl;
    }

    return 0;
}
#endif

#if 0
void merge_by_distance(Mesh& mesh, double distance_threshold) 
{
    std::vector<Point_with_index> points;
    for (int i=0; i< (int)mesh.vertices.size(); ++i) 
        points.emplace_back(glm_to_cgal(mesh.vertices[i]), i);
        
    CKd_tree tree(points.begin(), points.end());

    int num_verged_points = 0;

    // vertices map to themself by default
    vector<int> merge_map(mesh.vertices.size());
    for (size_t i = 0; i < merge_map.size(); ++i) {
        merge_map[i] = i;
    }

    auto new_vertices = mesh.vertices;
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] != (int)i) continue; // Skip already merged vertices

        Point_3 point = glm_to_cgal(mesh.vertices[i]);
        Fuzzy_sphere sphere(point, distance_threshold);
        vector<Point_with_index> neighbors;
        tree.search(back_inserter(neighbors), sphere);

        Point_3 merged_point = point;
        int count = 1;

        for (const auto& neighbor : neighbors) {
            int neighbor_index = neighbor.index;
            if (neighbor_index == (int)i // Skip myself
                || (neighbor.point == point)   // Skip equal points
                || merge_map[neighbor_index] != neighbor_index // skip unless if it already been mapped
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
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] != (int)i) {
            int m = merge_map[i];
            while (merge_map[m] != m)
                m = merge_map[m];
            new_vertices[i] = mesh.vertices[m];
        }
    }

    ofstream fh("/tmp/foo.csv");
    fh << "idx,org_x,org_y,org_z,map,new_x,new_y,new_z\n";
    for (int i=0; i<(int)new_vertices.size(); i++) {
      fh << format("{},{:.12f},{:.12f},{:.12f},{},{:.12f},{:.12f},{:.12f}\n",
                   i,
                   mesh.vertices[i].x,
                   mesh.vertices[i].y,
                   mesh.vertices[i].z,
                   merge_map[i],
                   new_vertices[i].x,
                   new_vertices[i].y,
                   new_vertices[i].z);
    }
    fh.close();

    // Update mesh.vertices with new_vertices
    mesh.vertices = new_vertices;

}
#endif

// This method seems to work, though I'm not really sure why. The idea
// is to create a kd tree of all the points and then merge nearby
// points. The problem is that in the kd tree below, only the points
// are attached, and not their indices.
//
// A better solution would be a to add the indices to the kdtree
// and create clusters of neighby points, and then map all the
// nearby points to their centroid. However, I never figured
// out how to create a kd tree with aux data in CGAL...
// But if this works, this is good enough. All I really want is
// to work around calculation inprecision.
void merge_by_distance(Mesh& mesh, double distance_threshold)
{
    Kd_tree tree;
    map<Point_3, int, point3_comparator> point_to_index;

    // Insert points into the Kd_tree and map them to their indices
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        Point_3 point = glm_to_cgal(mesh.vertices[i]);
        tree.insert(point);
        
        if (point_to_index.find(point) == point_to_index.end())
            point_to_index[point] = i;
    }

    int num_verged_points = 0;
    // vertices map to themself by default
    vector<int> merge_map(mesh.vertices.size());
    for (size_t i = 0; i < merge_map.size(); ++i) {
        merge_map[i] = i;
    }
    print("distance_threshold = {}\n", distance_threshold);

    auto new_vertices = mesh.vertices;
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] != (int)i) continue; // Skip already merged vertices

        Point_3 point = glm_to_cgal(mesh.vertices[i]);
        Fuzzy_sphere sphere(point, distance_threshold);
        vector<Point_3> neighbors;
        tree.search(back_inserter(neighbors), sphere);

        int count = 1;

        for (const auto& neighbor : neighbors) {
            int neighbor_index = point_to_index[neighbor];
            if (neighbor_index == (int)i // Skip myself
                || (neighbor == point)   // Skip equal points
                || merge_map[neighbor_index] != neighbor_index // skip unless if it already been mapped
                )
                continue;
                
            if (neighbor_index > (int)i)
                merge_map[neighbor_index] = i; // map the neighbor to this point
            else
                merge_map[i] = neighbor_index; // map this point to the lower indexed neighbor
            count++;
        }

        if (count > 1) {
//            mesh.vertices[i] = cgal_to_glm(merged_point); // Should be more precise to use average/centroid
            num_verged_points+= count-1;
        }
    }

    // Remove duplicate vertices
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (merge_map[i] != (int)i) {
            int m = merge_map[i];
            while (merge_map[m] != m)
                m = merge_map[m];
            new_vertices[i] = mesh.vertices[m];
        }
    }

#if 0
    // For debugging the algorithm
    ofstream fh("/tmp/foo.csv");
    fh << "idx,org_x,org_y,org_z,map,new_x,new_y,new_z\n";
    for (int i=0; i<(int)new_vertices.size(); i++) {
      fh << format("{},{:.12f},{:.12f},{:.12f},{},{:.12f},{:.12f},{:.12f}\n",
                   i,
                   mesh.vertices[i].x,
                   mesh.vertices[i].y,
                   mesh.vertices[i].z,
                   merge_map[i],
                   new_vertices[i].x,
                   new_vertices[i].y,
                   new_vertices[i].z);
    }
    fh.close();
#endif

    // Update mesh.vertices with new_vertices
    mesh.vertices = new_vertices;
}

// For debugging. Count the number of unique vertices
int count_unique_vertices(const Mesh& mesh)
{
    std::set<glm::dvec3, dvec3_comparator> unique_points(mesh.vertices.begin(), mesh.vertices.end());
    return unique_points.size();
}


