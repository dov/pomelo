#ifndef WINDING_NUMBER_H
#define WINDING_NUMBER_H

#include <utility>

#include <CGAL/basic.h>
#include <CGAL/enum.h>

template <typename Arrangement> class Winding_number {
private:
  Arrangement& _arr;
  typename Arrangement::Traits_2::Compare_endpoints_xy_2 _cmp_endpoints;

  // The Boolean flag indicates whether the face has been discovered already
  // during the traversal. The integral field stores the winding number.
  typedef std::pair<bool, int>                              Data;
  
public:
  Winding_number(Arrangement& arr) : _arr(arr)
  {
    // Initialize the winding numbers of all faces.
    typename Arrangement::Face_iterator fi;
    for (fi = _arr.faces_begin(); fi != _arr.faces_end(); ++fi)
      fi->set_data(Data(false, 0));
    _cmp_endpoints = _arr.traits()->compare_endpoints_xy_2_object(); /* \label{lst:winding_number:a} */
    propagate_face(_arr.unbounded_face(), 0);   // compute the winding numbers
  }

private:
  // Count the net change to the winding number when crossing a halfedge.
  int count(typename Arrangement::Halfedge_handle he)
  {
    bool l2r = he->direction() == CGAL::ARR_LEFT_TO_RIGHT;
    typename Arrangement::Originating_curve_iterator ocit;
    int num = 0;
    for (ocit = _arr.originating_curves_begin(he);
         ocit != _arr.originating_curves_end(he); ++ocit)
      (l2r == (_cmp_endpoints(*ocit) == CGAL::SMALLER)) ? ++num : --num; /* \label{lst:winding_number:b} */
    return num;
  }

  // Traverse all faces neighboring the given face and compute the
  // winding numbers of all faces while traversing the arrangement.
  void propagate_face(typename Arrangement::Face_handle fh, int num)
  {
    if (fh->data().first) return;
    fh->set_data(Data(true, num));

    // Traverse the inner boundary (holes).
    typename Arrangement::Hole_iterator hit;
    for (hit = fh->holes_begin(); hit != fh->holes_end(); ++hit) {
      typename Arrangement::Ccb_halfedge_circulator cch = *hit;
      do {
        typename Arrangement::Face_handle inner_face = cch->twin()->face();
        if (inner_face == cch->face()) continue;        // discard antenas
        propagate_face(inner_face, num + count(cch->twin()));
      } while (++cch != *hit);
    }

    // Traverse the outer boundary.
    if (fh->is_unbounded()) return;
    typename Arrangement::Ccb_halfedge_circulator cco = fh->outer_ccb();
    do {
      typename Arrangement::Face_handle outer_face = cco->twin()->face();
      propagate_face(outer_face, num + count(cco->twin()));
    } while (++cco != fh->outer_ccb());
  }
};

#endif
