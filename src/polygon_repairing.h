#ifndef POLYGON_REPAIRING_H
#define POLYGON_REPAIRING_H

#include <utility>

#include <CGAL/basic.h>
#include <CGAL/Arrangement_with_history_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include "Winding_number.h"


template <typename Traits, typename Input_iterator, typename Container>
void polygon_repairing(Input_iterator segments_begin,
                       Input_iterator segments_end,
                       Container& res,
                       const Traits& traits)
{
  // Each face is extended with a pair of a Boolean flag and an integral
  // field: The former indicates whether the face has been discovered
  // already during the traversal. The latter stores the winding number.
  typedef std::pair<bool, int>                              Data;
  typedef CGAL::Arr_face_extended_dcel<Traits, Data>        Dcel;
  typedef CGAL::Arrangement_with_history_2<Traits, Dcel>    Arrangement;

  Arrangement arr(&traits);

  CGAL::insert(arr, segments_begin, segments_end);

  Winding_number<Arrangement> winding_number(arr);

  for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
    // Here is where we do our winding number test

    
    // The original code contained an odd/even test
    //    if ((fi->data().second % 2) == 0) continue;

    // This test looks for positive polygons. 
    if (fit->data().second != 1
        ) continue;

    CGAL_assertion(!fit->is_unbounded());
    typename Container::value_type polygon;
    typename Arrangement::Ccb_halfedge_circulator cco = fit->outer_ccb();
    do
      polygon.push_back({
          CGAL::to_double(cco->target()->point().x()),
          CGAL::to_double(cco->target()->point().y())});
    while (++cco != fit->outer_ccb());

    res.push_back(polygon);
  }
}

#endif
