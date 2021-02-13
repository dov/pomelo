// A utility for replacing acute angles with radius of points
//
// For prototype of the needed math see:
//
//  file:../smooth-corner.py

#include "textrusion.h"
#include <math.h>
#include <fmt/core.h>

using namespace std;
using namespace fmt;

static const double MY_PI=3.141592653589793;

double signed_angle(const Vector_2& v, const Vector_2& w)
{
  // Return the signed angle between two 2D vectors
  auto v_norm = v/CGAL::sqrt(v.squared_length());
  auto w_norm = w/CGAL::sqrt(w.squared_length());

  auto angle = acos(CGAL::to_double(v_norm * w_norm));

  // Calculate perpendicular vector from first principle
  Vector_2 v_norm_perp(-v_norm[1], v_norm[0]);

  // The following dot product tests which side we are on
  if (v_norm_perp * w < 0)
    angle = -angle + MY_PI*2;

 return angle;
}

// Given three points find the center of rotation and a radius
// find the center of rotation, the start, and the end angle,
// as well as an indication on whether it is an inner
// angle.
//
// output:
//    c - center of rotation
//    theta_s - start angle
//    theta_e - end angle
//    bool - Whether the angle is an inner or outer angle
void find_center_of_rotation(const Point_2& p,
                             const Point_2& q,
                             const Point_2& r,
                             double radius,
                             // output
                             Point_2& c,
                             double& theta_s,
                             double& theta_e,
                             bool& inner_angle
                             )
{
  Vector_2 v(q,p);
  Vector_2 v_norm = v/CGAL::sqrt(v.squared_length());
  Vector_2 w(q,r);
  Vector_2 w_norm = w/CGAL::sqrt(w.squared_length());
  double theta = signed_angle(v,w);
  double x = radius/tan(theta/2);
  Vector_2 v_norm_perp (-v_norm[1],v_norm[0]);
  Vector_2 d_perp(radius * v_norm_perp);

  double alpha = -atan2(w_norm[1],w_norm[0]);
  theta_e = MY_PI/2-alpha;
  theta_s = theta_e + 2 * (MY_PI/2-atan2(radius, x));

  inner_angle = theta > MY_PI;
  if (inner_angle)
    {
      c = q - x * v_norm - d_perp;
      theta_s += MY_PI;
      theta_e += MY_PI;
    }
  else 
    c = q + x * v_norm + d_perp;
}

static double calc_angle(const Point_2& p,
                         const Point_2& q,
                         const Point_2& r)
{
  Vector_2 v(q,p);
  Vector_2 w(q,r);
  double angle = acos(v*w/(CGAL::sqrt(v.squared_length())*CGAL::sqrt(w.squared_length())));
  //  print("angle = {} deg\n", angle*180/MY_PI);
  return angle;
}

// This routine will replace all acute angles with an arc
// where n indicates the number of points if the arc was a full
// circle and radius is the radius.
static Polygon_2 
calc_smooth_polygon(const Polygon_2& poly,
                    double radius,
                    double max_angle_to_smooth,
                    int num_smooth_points,
                    bool smooth_inner_angles,
                    bool smooth_outer_angles)
{
  Polygon_2 ret;
  size_t n = poly.size();

  for (size_t i=0; i<poly.size(); i++)
    {
      const auto& p = poly[(i-1+n)%n];
      const auto& q = poly[i];
      const auto& r = poly[(i+1)%n];

      if (calc_angle(p,q,r) > max_angle_to_smooth)
        ret.push_back(q);
      else
        {
        Point_2 c;
        double th_s, th_e;
        bool inner_angle;
        find_center_of_rotation(p,q,r,radius,
                                // output
                                c, th_s, th_e, inner_angle);
        if ((inner_angle && !smooth_inner_angles)
            || (!inner_angle && !smooth_outer_angles))
          ret.push_back(q);
        else
          {
            double dth = (th_e-th_s)/num_smooth_points;
            for (int i=0; i<num_smooth_points+1; i++)
              {
                double th = th_s + dth * i;
                ret.push_back(
                  {c[0] + radius * cos(th),
                   c[1] + radius * sin(th)});
              }
          }
      }
    }

  return ret;
}

vector<Polygon_with_holes>
smooth_acute_angles(double radius,
                    double max_angle_to_smooth,
                    const vector<Polygon_with_holes>& hpolys,
                    int num_smooth_points)
{
  vector<Polygon_with_holes> ret;
  for (auto &ph : hpolys)
    {
      Polygon_with_holes out_ph(
        calc_smooth_polygon(ph.outer_boundary(),
                            radius,
                            max_angle_to_smooth,
                            num_smooth_points,
                            false,
                            true));

      for (const auto& h : ph.holes())
        out_ph.add_hole(
          calc_smooth_polygon(h,
                              radius,
                              max_angle_to_smooth,
                              num_smooth_points,
                              true,
                              false));
      ret.push_back(out_ph);
    }
  return ret;
}
