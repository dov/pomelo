# Technical description

(To be continued)

Here is a rought description of how pomelo works:

1. The user chooses a font and a text, or an svg. 
2. The input is rendered into a bitmap which is then traced to create a set of polygons with holes. This step may seen redundant, but the reason this is done is to solve the problem of self intersecting contours. Fonts are notorious to contain tiny self intersections, and these prevent the straight skeleton algorithm from working. The tracing of the bitmap is guaranteed to always produce a non-selfintersecting contour.
3. If the sharp acute angle option is on, then all sharp inner corners of the contour are replaced with an arc of a fixed number of points, currently 16.
4. The resulting polygon with holes collection is passed as input to the straight skeleton algorithm. (The skeleton may be viewed by View/Skeleton). The skeleton creates a set of polygonal "regions" that fill the area between the edge of the body, up to the straight skeleton. These regions all have one edge that sit on the original polyline edges.
5. The further processing is done for each region.
6. To turn the geometry into a mesh, sample the profile equidistantly. Take a pair of subsequent points of the profile between d[n] and d[n+1] (from the edge). Cut the regions in step 4 between the distance d[n] and d[n+1] from the polyline edge. Create a quad with the x,y points from the region, and the height according to the profile map. Create a flat quad at z=-zdepth. Add these two quads to the mesh.

