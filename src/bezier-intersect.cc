//======================================================================
//  bezier-intersect.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jun 24 10:51:42 2022
//----------------------------------------------------------------------

#include "bezier-intersect.h"
#include <math.h>

using namespace std;

namespace bezier_intersect {

// Code from http://math.ivanovo.ac.ru/dalgebra/Khashin/poly/index.html

#define	TwoPi  6.28318530717958648
const double eps=1e-14;

//=============================================================================
// _root3, root3 from http://prografix.narod.ru
//=============================================================================
static double _root3 ( double x )
{
  double s = 1.;
  while ( x < 1. )
  {
    x *= 8.;
    s *= 0.5;
  }
  while ( x > 8. )
  {
    x *= 0.125;
    s *= 2.;
  }
  double r = 1.5;
  r -= 1./3. * ( r - x / ( r * r ) );
  r -= 1./3. * ( r - x / ( r * r ) );
  r -= 1./3. * ( r - x / ( r * r ) );
  r -= 1./3. * ( r - x / ( r * r ) );
  r -= 1./3. * ( r - x / ( r * r ) );
  r -= 1./3. * ( r - x / ( r * r ) );
  return r * s;
}

double root3 ( double x )
{
  if ( x > 0 ) return _root3 ( x ); else
    if ( x < 0 ) return-_root3 (-x ); else
      return 0.;
}


//---------------------------------------------------------------------------
// x - array of size 3
// In case 3 real roots: => x[0], x[1], x[2], return 3
//         2 real roots: x[0], x[1],          return 2
//         1 real root : x[0], x[1] ± i*x[2], return 1
int SolveP3(double *x,double a,double b,double c) {	// solve cubic equation x^3 + a*x^2 + b*x + c = 0
  double a2 = a*a;
  double q  = (a2 - 3*b)/9; 
  double r  = (a*(2*a2-9*b) + 27*c)/54;
  // equation y^3 - 3q*y + r/2 = 0 where x = y-a/3
  if (fabs(q) < eps)
  {		// y^3 =-r/2	!!! Thanks to John Fairman <jfairman1066@gmail.com>
    if (fabs(r) < eps) {	// three identical roots
      x[0] = x[1] = x[2] = -a/3;
      return(3);
    }
    // y^3 =-r/2
    x[0] = root3(-r/2);
    x[1] = x[0] * 0.5;
    x[2] = x[0] * sqrt(3.) / 2;
    return(1);
  }
  // now favs(q)>eps
  double r2 = r*r;
  double q3 = q*q*q;
  double A,B;
  if (r2 <= (q3 + eps)) {//<<-- FIXED!
    double t=r/sqrt(q3);
    if( t<-1) t=-1;
    if( t> 1) t= 1;
    t=acos(t);
    a/=3; q=-2*sqrt(q);
    x[0]=q*cos(t/3)-a;
    x[1]=q*cos((t+TwoPi)/3)-a;
    x[2]=q*cos((t-TwoPi)/3)-a;
    return(3);
  }
  else
  {
    //A =-pow(fabs(r)+sqrt(r2-q3),1./3); 
    A =-root3(fabs(r)+sqrt(r2-q3)); 
    if( r<0 )
      A=-A;
    B = (A==0? 0 : q/A);

    a/=3;
    x[0] =(A+B)-a;
    x[1] =-0.5*(A+B)-a;
    x[2] = 0.5*sqrt(3.)*(A-B);
    if(fabs(x[2])<eps) { x[2]=x[1]; return(2); }
    return(1);
  }
}// SolveP3(double *x,double a,double b,double c) {	

vector<double> cubicRoots(double a, double b, double c, double d)
{
  double A=b/a;
  double B=c/a;
  double C=d/a;

  double X[3];
  int num = SolveP3(X, A,B,C);

  vector<double> Res;
  for (int i=0; i<num; i++)
    Res.push_back(X[i]);

  return Res;
}

// Find all intersections between a bezier and a line
vector<Vec2> find_bezier_line_intersection(const Bezier& bezier,
                                           const Line& line)
{
  vector<Vec2> res;

  double Ax = 3 * (bezier.cp0.x - bezier.cp1.x) + bezier.xy1.x - bezier.xy0.x;
  double Ay = 3 * (bezier.cp0.y - bezier.cp1.y) + bezier.xy1.y - bezier.xy0.y;
  double Bx = 3 * (bezier.xy0.x - 2 * bezier.cp0.x + bezier.cp1.x);
  double By = 3 * (bezier.xy0.y - 2 * bezier.cp0.y + bezier.cp1.y);
  double Cx = 3 * (bezier.cp0.x - bezier.xy0.x);
  double Cy = 3 * (bezier.cp0.y - bezier.xy0.y);
  double Dx = bezier.xy0.x;
  double Dy = bezier.xy0.y;
  double vy = line.xy0.y - line.xy1.y;
  double vx = line.xy1.x - line.xy0.x;

  double d = line.xy0.x * vy + line.xy0.y * vx;

  vector<double> roots = cubicRoots(
    vy * Ax + vx * Ay,
    vy * Bx + vx * By,
    vy * Cx + vx * Cy,
    vy * Dx + vx * Dy - d);

  for (const auto& t : roots)
  {
    if (t < 0 || t > 1)
      continue;
    res.push_back(
      { ((Ax * t + Bx) * t + Cx) * t + Dx,
        ((Ay * t + By) * t + Cy) * t + Dy });
  }
  return res;

}
  
  
}
