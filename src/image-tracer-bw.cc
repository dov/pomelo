// ImageTracer reduced for BW (1-bit images)

/*
  ImageTracerBW.cpp
  (Desktop version in C++. See ImageTracerAndroid.java for the Android version.)
  Simple raster image tracer and vectorizer written in C++.
  This is a port of https://github.com/jankovicsandras/imagetracerjava

  by Dov Grobgeld <dov.grobgeld@gmail.com>
 */

/*

The Unlicense / PUBLIC DOMAIN

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to http://unlicense.org/

 */

#include <vector>
#include <string>
#include <array>
#include <map>
#include <fstream>
#include <sstream>
#include <fmt/core.h>
#include <cmath>
#include <stdarg.h>
#include <cairo/cairo.h>
#include <chrono>
#include <spdlog/spdlog.h>

using namespace std;

string input_filename;

// Returns a timestamp as milliseconds since the epoch. Note this time may jump
// around subject to adjustments by the system, to measure elapsed time use
// Timer instead.
int64_t GetTimeInMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now() -
             std::chrono::system_clock::from_time_t(0))
      .count();
}

using namespace std;

const string versionnumber = "1.1.2";

#undef TRACE_QUADRATIC 

// Forward declarations
class ImageData;
class IndexedImage;
static map<string,double> checkoptions (map<string,double> options);
static string imagedataToSVG (const ImageData& imgd,
                              map<string,double> options);
static IndexedImage imagedataToTracedata (const ImageData& imgd,
                                          const map<string,double>& options);
static string getsvgstring (const IndexedImage& ii, map<string,double> options);
// This just turn an image into the "IndexedImage" (change of format)
static IndexedImage imagedataToIndexedImage (const ImageData& imgd);
static vector<vector<vector<int>>> layering (const IndexedImage& ii);
static vector<vector<vector<int>>> pathscan (vector<vector<int>>& arr,
                                             int pathomit);
static vector<vector<vector<vector<int>>>> batchpathscan(
  const vector<vector<vector<int>>>& layers, int pathomit);
static vector<vector<vector<double>>> internodes (vector<vector<vector<int>>> paths);
static vector<vector<vector<vector<double>>>> batchinternodes (vector<vector<vector<vector<int>>>> bpaths);
static vector<vector<double>> tracepath (vector<vector<double>> path,
                                         double ltreshold,
                                         double qtreshold);
static vector<vector<double>> fitseq (vector<vector<double>> path,
                                      double ltreshold,
                                      double qtreshold,
                                      int seqstart,
                                      int seqend);
static vector<vector<vector<double>>> batchtracepaths (vector<vector<vector<double>>> internodepaths, double ltres,double qtres);
static vector<vector<vector<vector<double>>>> batchtracelayers (vector<vector<vector<vector<double>>>> binternodes, double ltres, double qtres);
static string tosvgcolorstr (const array<uint8_t,3>& c);
static string saveLayersToGiv(
  const string& image_filename,
  const vector<vector<vector<vector<double>>>>& layers);
static void layersToCairo(cairo_t *cr,
                          const vector<vector<vector<vector<double>>>>& layers,
                          double resolution);
static void savestring (const string& filename, const string& str);

// Container for the color-indexed image before and tracedata after vectorizing
class IndexedImage
{
  public:
  int width, height;
  vector<vector<int>> aray; // aray[x][y] of palette colors
  vector<vector<vector<vector<double>>>> layers; // tracedata
  

  public:
  // Constructor
  IndexedImage(const vector<vector<int>>& marray)
  {
    aray = marray;
    width = marray[0].size()-2;
    height = marray.size()-2;
  }
};

class ImageData
{
  public:
  int width, height;
  uint8_t *data; // Externally owned raw byte data: R R R R
                        
  public:
  ImageData(int mwidth, int mheight, uint8_t *mdata)
  {
    width = mwidth; height = mheight; data = mdata;
  }
};

// External entry point. Receive's an image and traces it to to cr.
void trace_image(int width,
                 int height,
                 int stride,
                 uint8_t *data,
                 double resolution,
                 // output
                 cairo_t *cr
                 )
{
  spdlog::info("trace_image() width={} height={} resolution={:.3f}",
               width, height, resolution);
  fmt::print("Tracing image of size {}x{} pixels\n", width, height);

  // Assume 8-bit image
  ImageData imgd(stride,height,data);
  map<string,double> options;

  options = checkoptions(options);
  options["ltres"]=2.5; // Do 200 for testing
  options["qtres"]=-1; // Turn off quadratic tracing for pomelo
  // TBD - Do qthresh + linearization to see if it gives a smoother result.

  IndexedImage ii = imagedataToTracedata(imgd,options);

#if 0
  savestring("/tmp/trace.giv",
             saveLayersToGiv("/tmp/flat-by-image.png",
                             ii.layers));
#endif
  return layersToCairo(cr, ii.layers, resolution);
}

////////////////////////////////////////////////////////////
//
//  User friendly functions
//
////////////////////////////////////////////////////////////

// Record the layers into a cairo context
static void layersToCairo(cairo_t *cr,
                          const vector<vector<vector<vector<double>>>>& layers,
                          double resolution)
{
  double s = 1.0/resolution;

  for (int layer_idx=1; layer_idx >=0; layer_idx--)
  {
    const auto& layer = layers[layer_idx];

    for (int path_idx = 0; path_idx < (int)layer.size(); path_idx++)
    {
      // Skip path=0 for layer=0
      if (layer_idx==0 && path_idx == 0)
        continue;
      const auto& path = layer[path_idx];

      // TBD - How do I guarantee that the paths for layer 0
      // are in the opposite direction from layer 1.
      for (int coord_idx=0; coord_idx<(int)path.size(); coord_idx++)
      {
        vector<double> coord;
        // Revert the winding number of layer 0
        if (layer_idx == 0)
          coord = path[path.size()-1-coord_idx];
        else
          coord = path[coord_idx];

        if (coord_idx == 0)
          cairo_move_to(cr, s*coord[1], s*coord[2]);
        else // a line
          cairo_line_to(cr, s*coord[1], s*coord[2]);
      }
      cairo_close_path(cr);
    }
  }
}

// Tracing ImageData, then returning the SVG string
static string imagedataToSVG (const ImageData& imgd,
                              map<string,double> options)
{
  options = checkoptions(options);
  IndexedImage ii = imagedataToTracedata(imgd,options);
  return getsvgstring(ii, options);
}// End of imagedataToSVG()


// Loading an image from a file, tracing when loaded, then returning IndexedImage with tracedata in layers
vector<string> layer_colors = { "red", "green" };
static void savePathsToGiv(const string& filename,
                           const vector<vector<vector<vector<int>>>>& bps)
{
  ofstream fh(filename);

  int layer_idx=0;
  for (const auto& layer : bps)
  {
    int path_idx=0;
    for (const auto& path :layer)
    {
      fh << fmt::format("$path layer {}/path {}\n"
                        "$color {}\n",
                        layer_idx, path_idx,
                        layer_colors[layer_idx]);
      for (const auto& coord : path)
      {
        fh << fmt::format("{} {}\n", coord[0], coord[1]);
      }
      fh << fmt::format("z\n\n", path_idx);
      path_idx++;
    }
    layer_idx++;
  }
}

// overloaded for double for the result of the "interpolation"
static void savePathsToGiv(const string& filename,
                           const vector<vector<vector<vector<double>>>>& bps)
{
  ofstream fh(filename);

  int layer_idx=0;
  for (const auto& layer : bps)
  {
    int path_idx=0;
    for (const auto& path :layer)
    {
      fh << fmt::format("$path layer {}/path {}\n"
                   "$color {}\n",
                   layer_idx, path_idx,
                   layer_colors[layer_idx]);
      for (const auto& coord : path)
      {
        fh << fmt::format("{} {}\n", coord[0], coord[1]);
      }
      fh << fmt::format("z\n\n", path_idx);
      path_idx++;
    }
    layer_idx++;
  }
}

// overloaded for double for the result of the "interpolation"
static string saveLayersToGiv(const string& image_filename,
                              const vector<vector<vector<vector<double>>>>& layers)
{
  stringstream strm;

  if (image_filename.size())
    strm << fmt::format("$image {}\n\n", image_filename);

  int layer_idx=0;
  for (const auto& layer : layers)
  {
    int path_idx=0;
    for (const auto& path :layer)
    {
      strm << fmt::format("$path layer {}/path {}\n"
                     "$color {}\n"
                     "$lw 3\n"
                     ,
                   layer_idx, path_idx,
                   layer_colors[layer_idx]
                   );
      int coord_idx = 0;
      double old_x = -9e9, old_y = -9e9;
      for (const auto& coord : path)
      {
        if (coord_idx == 0
            || (coord[1]!= old_x || coord[2] != old_y))
          strm << fmt::format("{} {}\n", coord[1], coord[2]);

        if (coord[0] == 2.0) // quadratic
        {
          strm << fmt::format("R {} {} {} {}\n",
                         coord[3], coord[4], coord[5], coord[6]);
          old_x = coord[5];
          old_y = coord[6];
        }
        else // a line
        {
          strm << fmt::format("{} {}\n", coord[3], coord[4]);
          old_x = coord[1];
          old_y = coord[2];
        }

        coord_idx++;
      }
      strm << fmt::format("z\n\n", path_idx);
      path_idx++;
    }
    layer_idx++;
  }
  return strm.str();
}



// Tracing ImageData, then returning IndexedImage with tracedata in layers
static IndexedImage imagedataToTracedata (const ImageData& imgd,
                                          const map<string,double>& options)
{
  // 1. Color quantization
  int64_t time0 = GetTimeInMillis();
  IndexedImage ii = imagedataToIndexedImage(imgd);
  fmt::print("imagedataToIndexedImage: {} ms\n", GetTimeInMillis()-time0);
  time0 = GetTimeInMillis();
  // 2. Layer separation and edge detection
  vector<vector<vector<int>>> rawlayers = layering(ii);   // Layer,y,x (rtl meaning of indices)
  fmt::print("layering: {} ms\n", GetTimeInMillis()-time0);
  time0 = GetTimeInMillis();
  // 3. Batch pathscan
  vector<vector<vector<vector<int>>>> bps = batchpathscan(
    rawlayers,
    (int)(floor(options.at("pathomit"))));
  fmt::print("batchpathscan: {} ms\n", GetTimeInMillis()-time0);
  time0 = GetTimeInMillis();

#if 0
  savePathsToGiv("image_trace.giv", bps);
#endif

  // 4. Batch interpollation
  vector<vector<vector<vector<double>>>> bis = batchinternodes(bps);
  fmt::print("batchinterpolation: {}ms\n", GetTimeInMillis()-time0);
  time0 = GetTimeInMillis();

#if 0
  savePathsToGiv("image_trace_bi.giv", bis);
#endif
  
  // 5. Batch tracing
  ii.layers = batchtracelayers(bis,options.at("ltres"),options.at("qtres"));
  fmt::print("batchtracelayers: {}ms\n", GetTimeInMillis()-time0);
  time0 = GetTimeInMillis();

#if 0
  savestring("image_layers.giv",
             saveLayersToGiv(input_filename, ii.layers));
#endif

  return ii;
}// End of imagedataToTracedata()


// creating options object, setting defaults for missing values
static map<string,double> checkoptions (map<string,double> options)
{
  // Tracing
  if(!options.count("ltres")){ options["ltres"]=3; }

  // A negative qtres will omit cubic tracing
  if(!options.count("qtres")){ options["qtres"]=1; }
  if(!options.count("pathomit")){ options["pathomit"]=8; }
  // Color quantization
  if(!options.count("colorsampling")){ options["colorsampling"]=1; }
  if(!options.count("numberofcolors")){ options["numberofcolors"]=16; }
  if(!options.count("mincolorratio")){ options["mincolorratio"]=2; }
  if(!options.count("colorquantcycles")){ options["colorquantcycles"]=3; }
  // SVG rendering
  if(!options.count("scale")){ options["scale"]=1; }
  if(!options.count("simplifytolerance")){ options["simplifytolerance"]=0; }
  if(!options.count("roundcoords")){ options["roundcoords"]=1; }
  if(!options.count("lcpr")){ options["lcpr"]=0; }
  if(!options.count("qcpr")){ options["qcpr"]=0; }
  if(!options.count("desc")){ options["desc"]=1; }
  if(!options.count("viewbox")){ options["viewbox"]=0; }

  return options;
}// End of checkoptions()

////////////////////////////////////////////////////////////
//
//  Vectorizing functions
//
////////////////////////////////////////////////////////////

// 1. Color quantization repeated "cycles" times, based on K-means clustering
// https://en.wikipedia.org/wiki/Color_quantization    https://en.wikipedia.org/wiki/K-means_clustering
static IndexedImage imagedataToIndexedImage (const ImageData& imgd)
{
  // Creating indexed color array arr which has a boundary filled with -1 in every direction
  vector<vector<int>> arr(imgd.height+2,
                          vector<int>(imgd.width+2));

  for(int j=0; j<(imgd.height+2); j++)
  {
    arr[j][0] = -1;
    arr[j][imgd.width+1 ] = -1;
  }

  for(int i=0; i<(imgd.width+2) ; i++)
  {
    arr[0][i] = -1;
    arr[imgd.height+1][i] = -1;
  }

  for (int j=0; j< imgd.height; j++)
      for (int i=0; i<imgd.width; i++)
          arr[j+1][i+1] = imgd.data[j*imgd.width + i];

  return IndexedImage(arr);
}// End of imagedataToIndexedImage

// 2. Layer separation and edge detection
// Edge node types ( ▓:light or 1; ░:dark or 0 )
// 12  ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓
// 48  ░░  ░░  ░░  ░░  ░▓  ░▓  ░▓  ░▓  ▓░  ▓░  ▓░  ▓░  ▓▓  ▓▓  ▓▓  ▓▓
//     0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
//
// Index meaning from inner to outer:
//
//   - 0 : x (size width+2)
//   - 1 : y (size height+2)
//   - layer_idx
// 
static vector<vector<vector<int>>> layering (const IndexedImage& ii)
{
  // Creating layers for each indexed color in arr
  int aw = ii.aray[0].size(), ah = ii.aray.size();
  
  vector<vector<vector<int>>> layers(
    2, // Two fixed colors (or is it 1?)
    vector<vector<int>>(ah, vector<int>(aw,0)));

  // Looping through all pixels and calculating edge node type
  for(int j=1; j<(ah-1); j++)
  {
    for(int i=1; i<(aw-1); i++)
    {
      // This pixel's indexed color
      int val = ii.aray[j][i];
      int layer_idx = (int)(val>0);

      // Are neighbor pixel colors the same?
      int n1 = ii.aray[j-1][i-1]==val ? 1 : 0;
      int n2 = ii.aray[j-1][i  ]==val ? 1 : 0;
      int n3 = ii.aray[j-1][i+1]==val ? 1 : 0;
      int n4 = ii.aray[j  ][i-1]==val ? 1 : 0;
      int n5 = ii.aray[j  ][i+1]==val ? 1 : 0;
      int n6 = ii.aray[j+1][i-1]==val ? 1 : 0;
      int n7 = ii.aray[j+1][i  ]==val ? 1 : 0;
      int n8 = ii.aray[j+1][i+1]==val ? 1 : 0;

      // this pixel"s type and looking back on previous pixels
      {
        layers[layer_idx][j+1][i+1] = 1 + (n5 * 2) + (n8 * 4) + (n7 * 8) ;
        if(n4==0)
          layers[layer_idx][j+1][i  ] = 0 + 2 + (n7 * 4) + (n6 * 8) ; 
        if(n2==0)
          layers[layer_idx][j  ][i+1] = 0 + (n3*2) + (n5 * 4) + 8 ; 
        if(n1==0)
          layers[layer_idx][j  ][i  ] = 0 + (n2*2) + 4 + (n4 * 8) ;
      }

    }// End of i loop
  }// End of j loop

  return layers;
}// End of layering()


// Lookup tables for pathscan
static constexpr uint8_t pathscan_dir_lookup[] = {0,0,3,0, 1,0,3,0, 0,3,3,1, 0,3,0,0};
static constexpr bool pathscan_holepath_lookup[] = {false,false,false,false, false,false,false,true, false,false,false,true, false,true,true,false };
// pathscan_combined_lookup[ arr[py][px] ][ dir ] = [nextarrpypx, nextdir, deltapx, deltapy];
static constexpr int8_t pathscan_combined_lookup[16][4][4] = {
    {{-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}},// arr[py][px]==0 is invalid
    {{ 0, 1, 0,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 2,-1, 0}},
    {{-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 1, 0,-1}, { 0, 0, 1, 0}},
    {{ 0, 0, 1, 0}, {-1,-1,-1,-1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}},

    {{-1,-1,-1,-1}, { 0, 0, 1, 0}, { 0, 3, 0, 1}, {-1,-1,-1,-1}},
    {{13, 3, 0, 1}, {13, 2,-1, 0}, { 7, 1, 0,-1}, { 7, 0, 1, 0}},
    {{-1,-1,-1,-1}, { 0, 1, 0,-1}, {-1,-1,-1,-1}, { 0, 3, 0, 1}},
    {{ 0, 3, 0, 1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}, {-1,-1,-1,-1}},

    {{ 0, 3, 0, 1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}, {-1,-1,-1,-1}},
    {{-1,-1,-1,-1}, { 0, 1, 0,-1}, {-1,-1,-1,-1}, { 0, 3, 0, 1}},
    {{11, 1, 0,-1}, {14, 0, 1, 0}, {14, 3, 0, 1}, {11, 2,-1, 0}},
    {{-1,-1,-1,-1}, { 0, 0, 1, 0}, { 0, 3, 0, 1}, {-1,-1,-1,-1}},

    {{ 0, 0, 1, 0}, {-1,-1,-1,-1}, { 0, 2,-1, 0}, {-1,-1,-1,-1}},
    {{-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 1, 0,-1}, { 0, 0, 1, 0}},
    {{ 0, 1, 0,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, { 0, 2,-1, 0}},
    {{-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {-1,-1,-1,-1}}// arr[py][px]==15 is invalid
};


// 3. Walking through an edge node array, discarding edge node types 0 and 15 and creating paths from the rest.
// Walk directions (dir): 0 > ; 1 ^ ; 2 < ; 3 v
// Edge node types ( ▓:light or 1; ░:dark or 0 )
// ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓  ░░  ▓░  ░▓  ▓▓
// ░░  ░░  ░░  ░░  ░▓  ░▓  ░▓  ░▓  ▓░  ▓░  ▓░  ▓░  ▓▓  ▓▓  ▓▓  ▓▓
// 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
//
// Index meaning
// 
//   0 - 0=x-coord, 1=y coord, 2=edge type
//   1 - coord in  path
//   2 - path index
static vector<vector<vector<int>>> pathscan (vector<vector<int>>& arr,
                                             int pathomit)
{
  vector<vector<vector<int>>> paths;
  int px=0,py=0,w=arr[0].size(),h=arr.size(),dir=0;
  bool pathfinished=true, holepath = false;

#if 0
  print("pathomit={}\n",pathomit);
  print("arr=\n  ");
  for (int j=0; j<h; j++)
  {
    for(int i=0;i<w;i++)
      print("{:2d} ", arr[j][i]);
    print("\n  ");
  }
  print("\n");
#endif

  
  for(int j=0;j<h;j++)
  {
    for(int i=0;i<w;i++)
    {
      if((arr[j][i]!=0)&&(arr[j][i]!=15))
      {
        // Init
        px = i; py = j;
        paths.push_back(vector<vector<int>>());
        auto& thispath = paths.back();
        pathfinished = false;

        // fill paths will be drawn, but hole paths are also required to remove unnecessary edge nodes
        dir = pathscan_dir_lookup[ arr[py][px] ];
        holepath = pathscan_holepath_lookup[ arr[py][px] ];

        // Path points loop
        while(!pathfinished)
        {
          // New path point
          thispath.push_back(vector<int>(3,0));
          thispath.back()[0] = px-1;
          thispath.back()[1] = py-1;
          thispath.back()[2] = arr[py][px];

          // Next: look up the replacement, direction and coordinate changes = clear this cell, turn if required, walk forward
          const int8_t* lookuprow = pathscan_combined_lookup[ arr[py][px] ][ dir ];
          arr[py][px] = lookuprow[0];
          dir = lookuprow[1];
          px += lookuprow[2];
          py += lookuprow[3];

          // Close path
          if(((px-1)==thispath[0][0])&&((py-1)==thispath[0][1]))
          {
            pathfinished = true;
            // Discarding 'hole' type paths and paths shorter than pathomit
            if( (holepath) || ((int)thispath.size()<pathomit) )
            {
              paths.pop_back();
            }
          }

        }// End of Path points loop

      }// End of Follow path

    }// End of i loop
  }// End of j loop

  return paths;
}// End of pathscan()


// 3. Batch pathscan
//
// Index meaning
//
//   0 - 0=x-coord, 1=y coord, 2=edge type
//   1 - coord in  path
//   2 - path index
//   3 - layer
static vector<vector<vector<vector<int>>>> batchpathscan (
  const vector<vector<vector<int>>>& layers, int pathomit)
{
  vector<vector<vector<vector<int>>>> bpaths;

  for (auto layer : layers) {
    bpaths.push_back(pathscan(layer,pathomit));
  }
  return bpaths;
}


// 4. interpolating between path points for nodes with 8 directions ( East, SouthEast, S, SW, W, NW, N, NE )
//
// Index meaning:
//
//   0 - 0:x, 1:y, 2:dir East=0,...,NE=7 (see above0
//   1 - coord idx
//   2 - path idx
//
static vector<vector<vector<double>>> internodes (vector<vector<vector<int>>> paths)
{
  vector<vector<vector<double>>> ins;
  vector<double> nextpoint(2,0.0);
  vector<int> pp1, pp2, pp3;
  int palen=0,nextidx=0,nextidx2=0;

  // paths loop
  for(size_t pacnt=0; pacnt<paths.size(); pacnt++)
  {
    ins.push_back(vector<vector<double>>());
    auto& thisinp = ins.back();
    palen = paths[pacnt].size();

    // pathpoints loop
    for(int pcnt=0;pcnt<palen;pcnt++)
    {
      // interpolate between two path points
      nextidx = (pcnt+1)%palen; nextidx2 = (pcnt+2)%palen;
      thisinp.push_back(vector<double>(3,0));
      auto& thispoint = thisinp.back();
      pp1 = paths[pacnt][pcnt];
      pp2 = paths[pacnt][nextidx];
      pp3 = paths[pacnt][nextidx2];
      thispoint[0] = (pp1[0]+pp2[0]) / 2.0;
      thispoint[1] = (pp1[1]+pp2[1]) / 2.0;
      nextpoint[0] = (pp2[0]+pp3[0]) / 2.0;
      nextpoint[1] = (pp2[1]+pp3[1]) / 2.0;

      // line segment direction to the next point
      if(thispoint[0] < nextpoint[0])
      {
        if     (thispoint[1] < nextpoint[1])
          thispoint[2] = 1.0; // SouthEast
        else if(thispoint[1] > nextpoint[1])
          thispoint[2] = 7.0; // NE
        else
          thispoint[2] = 0.0; // E
      }
      else if(thispoint[0] > nextpoint[0])
      {
        if     (thispoint[1] < nextpoint[1])
          thispoint[2] = 3.0; // SW
        else if(thispoint[1] > nextpoint[1])
          thispoint[2] = 5.0; // NW
        else
          thispoint[2] = 4.0; // W
      }else{
        if     (thispoint[1] < nextpoint[1])
          thispoint[2] = 2.0; // S
        else if(thispoint[1] > nextpoint[1])
          thispoint[2] = 6.0; // N
        else
          thispoint[2] = 8.0; // center, this should not happen
      }

    }// End of pathpoints loop
  }// End of paths loop
  return ins;
}// End of internodes()


// 4. Batch interpolation
//
// Index meaning:
//
//   0 
//   1
//   2
//   3 - layer
static vector<vector<vector<vector<double>>>> batchinternodes (
  vector<vector<vector<vector<int>>>> bpaths)
{
  vector<vector<vector<vector<double>>>> binternodes;
  for(size_t k=0; k<bpaths.size(); k++) 
    binternodes.push_back(internodes(bpaths[k]));

  return binternodes;
}


// 5. tracepath() : recursively trying to fit straight and quadratic spline segments on the 8 direction internode path

// 5.1. Find sequences of points with only 2 segment types
// 5.2. Fit a straight line on the sequence
// 5.3. If the straight line fails (an error>ltreshold), find the point with the biggest error
// 5.4. Fit a quadratic spline through errorpoint (project this to get controlpoint), then measure errors on every point in the sequence
// 5.5. If the spline fails (an error>qtreshold), find the point with the biggest error, set splitpoint = (fitting point + errorpoint)/2
// 5.6. Split sequence and recursively apply 5.2. - 5.7. to startpoint-splitpoint and splitpoint-endpoint sequences
// 5.7. TODO? If splitpoint-endpoint is a spline, try to add new points from the next sequence

// This returns an SVG Path segment as a double[7] where
// segment[0] ==1.0 linear  ==2.0 quadratic interpolation
// segment[1] , segment[2] : x1 , y1
// segment[3] , segment[4] : x2 , y2 ; middle point of Q curve, endpoint of L line
// segment[5] , segment[6] : x3 , y3 for Q curve, should be 0.0 , 0.0 for L line
//
// path type is discarded, no check for path.size < 3 , which should not happen
//
// Return indices:
//
//  0 - 
//  1 - 
static vector<vector<double>> tracepath (vector<vector<double>> path,
                                         double ltreshold,
                                         double qtreshold)
{
  int pcnt=0, seqend=0; double segtype1, segtype2;
  vector<vector<double>> smp;
  //Double [] thissegment;
  int pathlength = path.size();

  while(pcnt<pathlength)
  {
    // 5.1. Find sequences of points with only 2 segment types
    segtype1 = path[pcnt][2]; segtype2 = -1; seqend=pcnt+1;
    while(
        ((path[seqend][2]==segtype1) || (path[seqend][2]==segtype2) || (segtype2==-1))
        && (seqend<(pathlength-1)))
    {
      if((path[seqend][2]!=segtype1) && (segtype2==-1))
        segtype2 = path[seqend][2];
      seqend++;
    }
    if(seqend==(pathlength-1))
      seqend = 0; 

    // 5.2. - 5.6. Split sequence and recursively apply 5.2. - 5.6. to startpoint-splitpoint and splitpoint-endpoint sequences
    for (auto& v : fitseq(path,ltreshold,qtreshold,pcnt,seqend))
      smp.push_back(v);
    // 5.7. TODO? If splitpoint-endpoint is a spline, try to add new points from the next sequence

    // forward pcnt;
    if(seqend>0)
      pcnt = seqend;
    else
      pcnt = pathlength; 
  }// End of pcnt loop

  return smp;

}// End of tracepath()


// 5.2. - 5.6. recursively fitting a straight or quadratic line segment on this sequence of path nodes,
// called from tracepath()
//
//  Return indices:
//
//    - 0: Fixed size 7: 0: 1.0 for line, 2.0 for bezier
//                       1,2: x,y
//                       3,4,5,6 : bezier coordinates
//    - 1: Segment
static vector<vector<double>> fitseq (vector<vector<double>> path,
                                      double ltreshold,
                                      double qtreshold,
                                      int seqstart,
                                      int seqend)
{
  vector<vector<double>> segment;
  int pathlength = path.size();

  // return if invalid seqend
  if((seqend>pathlength)||(seqend<0))
    return segment;

  int errorpoint=seqstart;
  bool curvepass=true;
  double px, py, dist2, errorval=0;
  double tl = (seqend-seqstart); if(tl<0){ tl += pathlength; }
  double vx = (path[seqend][0]-path[seqstart][0]) / tl,
      vy = (path[seqend][1]-path[seqstart][1]) / tl;

  // 5.2. Fit a straight line on the sequence
  int pcnt = (seqstart+1)%pathlength;
  double pl;
  while(pcnt != seqend)
  {
    pl = pcnt-seqstart;
    if(pl<0)
      pl += pathlength; 
    px = path[seqstart][0] + (vx * pl);
    py = path[seqstart][1] + (vy * pl);
    dist2 = ((path[pcnt][0]-px)*(path[pcnt][0]-px)) + ((path[pcnt][1]-py)*(path[pcnt][1]-py));
    if(dist2>ltreshold)
      curvepass=false;
    if(dist2>errorval)
    {
      errorpoint=pcnt;
      errorval=dist2;
    }
    pcnt = (pcnt+1)%pathlength;
  }

  // return straight line if fits
  if(curvepass)
  {
    segment.push_back(vector<double>(7,0));
    auto& thissegment = segment.back();
    thissegment[0] = 1.0;
    thissegment[1] = path[seqstart][0];
    thissegment[2] = path[seqstart][1];
    thissegment[3] = path[seqend][0];
    thissegment[4] = path[seqend][1];
    thissegment[5] = 0.0;
    thissegment[6] = 0.0;
    return segment;
  }

  // 5.3. If the straight line fails (an error>ltreshold), find the point with the biggest error
  int fitpoint = errorpoint; 

  if (qtreshold > 0)
  {
    curvepass = true; errorval = 0;
  
    // 5.4. Fit a quadratic spline through this point, measure errors on every point in the sequence
    // helpers and projecting to get control point
    double t=(fitpoint-seqstart)/tl, t1=(1.0-t)*(1.0-t), t2=2.0*(1.0-t)*t, t3=t*t;
    double cpx = (((t1*path[seqstart][0]) + (t3*path[seqend][0])) - path[fitpoint][0])/-t2 ,
        cpy = (((t1*path[seqstart][1]) + (t3*path[seqend][1])) - path[fitpoint][1])/-t2 ;
  
    // Check every point
    pcnt = seqstart+1;
    while(pcnt != seqend)
    {
      t=(pcnt-seqstart)/tl; t1=(1.0-t)*(1.0-t); t2=2.0*(1.0-t)*t; t3=t*t;
      px = (t1 * path[seqstart][0]) + (t2 * cpx) + (t3 * path[seqend][0]);
      py = (t1 * path[seqstart][1]) + (t2 * cpy) + (t3 * path[seqend][1]);
  
      dist2 = ((path[pcnt][0]-px)*(path[pcnt][0]-px)) + ((path[pcnt][1]-py)*(path[pcnt][1]-py));
  
      if(dist2>qtreshold)
        curvepass=false;
      if(dist2>errorval)
      {
        errorpoint=pcnt; errorval=dist2;
      }
      pcnt = (pcnt+1)%pathlength;
    }
  
    // return spline if fits
    if(curvepass)
    {
      fmt::print("Got a spline!\n");
      segment.push_back(vector<double>(7,0));
      auto& thissegment = segment.back();
      thissegment[0] = 2.0;
      thissegment[1] = path[seqstart][0];
      thissegment[2] = path[seqstart][1];
      thissegment[3] = cpx;
      thissegment[4] = cpy;
      thissegment[5] = path[seqend][0];
      thissegment[6] = path[seqend][1];
      return segment;
    }
  }

  // 5.5. If the spline fails (an error>qtreshold), find the point with the biggest error,
  // set splitpoint = (fitting point + errorpoint)/2
  int splitpoint = (fitpoint + errorpoint)/2;

  // 5.6. Split sequence and recursively apply 5.2. - 5.6. to startpoint-splitpoint and splitpoint-endpoint sequences
  segment = fitseq(path,ltreshold,qtreshold,seqstart,splitpoint);
  for (const auto& v : fitseq(path,ltreshold,qtreshold,splitpoint,seqend))
    segment.push_back(v);
  return segment;

}// End of fitseq()


// 5. Batch tracing paths
//
// Return indices
//
//   0 - 
//   1 - 
//   2 - path
//
static vector<vector<vector<double>>> batchtracepaths (
  vector<vector<vector<double>>> internodepaths,
  double ltres,
  double qtres)
{
  vector<vector<vector<double>>> btracedpaths;
  for(size_t k=0; k<internodepaths.size(); k++)
    btracedpaths.push_back(tracepath(internodepaths[k],ltres,qtres) );
  return btracedpaths;
}


// 5. Batch tracing layers
//
// Return indices
//
//  0 - 
//  1 - 
//  2 - path
//  3 - layer
// 
static vector<vector<vector<vector<double>>>> batchtracelayers (
  vector<vector<vector<vector<double>>>> binternodes,
  double ltres,
  double qtres)
{
  vector<vector<vector<vector<double>>>> btbis;
  for(size_t k=0; k<binternodes.size(); k++)
    btbis.push_back( batchtracepaths( binternodes[k],ltres,qtres) );
  return btbis;
}


////////////////////////////////////////////////////////////
//
//  SVG Drawing functions
//
////////////////////////////////////////////////////////////

static double roundtodec (double val, double places)
{
  return round(val*pow(10,places))/pow(10,places);
}

// Getting SVG path element string from a traced path
static void svgpathstring (stringstream& sb,
                           const string& desc,
                           const vector<vector<double>>& segments,
                           const string& colorstr,
                           const map<string,double>& options)
{
  double scale = options.at("scale"), lcpr = options.at("lcpr"), qcpr = options.at("qcpr"), roundcoords = (double) floor(options.at("roundcoords"));

  // Path
  sb << "<path "
     << desc
     << colorstr
     << " d=\""
     << "M "
     << segments[0][1]*scale
     << " "
     << segments[0][2]*scale
     << " ";

  if( roundcoords == -1 )
  {
    for(size_t pcnt=0;pcnt<segments.size();pcnt++)
    {
      if(segments[pcnt][0]==1.0)
        sb << "L "
           << segments[pcnt][3]*scale
           << " "
           << segments[pcnt][4]*scale
           << " ";
      else
        sb << "Q "
           << segments[pcnt][3]*scale
           << " "
           << segments[pcnt][4]*scale
           << " "
           << segments[pcnt][5]*scale
           << " "
           << segments[pcnt][6]*scale
           << " ";
    }
  }
  else
  {
    for(size_t pcnt=0;pcnt<segments.size();pcnt++)
    {
      if(segments[pcnt][0]==1.0)
      {
        sb << "L "
           << roundtodec(double(segments[pcnt][3]*scale),roundcoords) << " "
           << roundtodec(double(segments[pcnt][4]*scale),roundcoords) << " ";
      }
      else
      {
        sb << "Q "
           << roundtodec(double(segments[pcnt][3]*scale),roundcoords) << " "
           << roundtodec(double(segments[pcnt][4]*scale),roundcoords) << " "
           << roundtodec(double(segments[pcnt][5]*scale),roundcoords) << " "
           << roundtodec(double(segments[pcnt][6]*scale),roundcoords) << " ";
      }
    }
  }// End of roundcoords check

  sb << "Z\" />";

  // Rendering control points
  for(size_t pcnt=0;pcnt<segments.size();pcnt++)
  {
    if((lcpr>0)&&(segments[pcnt][0]==1.0))
    {
      sb <<  "<circle cx=\""
         << segments[pcnt][3]*scale
         << "\" cy=\""
         << segments[pcnt][4]*scale
         << "\" r=\""
         << lcpr
         << "\" fill=\"white\" stroke-width=\""
         << lcpr*0.2
         << "\" stroke=\"black\" />";
    }
    if((qcpr>0)&&(segments[pcnt][0]==2.0))
    {
      sb <<  "<circle cx=\""
         << segments[pcnt][3]*scale
         << "\" cy=\""
         << segments[pcnt][4]*scale
         << "\" r=\""
         << qcpr
         << "\" fill=\"cyan\" stroke-width=\""
         << qcpr*0.2
         << "\" stroke=\"black\" />";
      sb <<  "<circle cx=\""
         << segments[pcnt][5]*scale
         << "\" cy=\""
         << segments[pcnt][6]*scale
         << "\" r=\""
         << qcpr
         << "\" fill=\"white\" stroke-width=\""
         << qcpr*0.2
         << "\" stroke=\"black\" />";
      sb <<  "<line x1=\""
         << segments[pcnt][1]*scale
         << "\" y1=\""
         << segments[pcnt][2]*scale
         << "\" x2=\""
         << segments[pcnt][3]*scale
         << "\" y2=\""
         << segments[pcnt][4]*scale
         << "\" stroke-width=\""
         << qcpr*0.2
         << "\" stroke=\"cyan\" />";
      sb <<  "<line x1=\""
         << segments[pcnt][3]*scale
         << "\" y1=\""
         << segments[pcnt][4]*scale
         << "\" x2=\""
         << segments[pcnt][5]*scale
         << "\" y2=\""
         << segments[pcnt][6]*scale
         << "\" stroke-width=\""
         << qcpr*0.2
         << "\" stroke=\"cyan\" />";
    }// End of quadratic control points
  }

}// End of svgpathstring()


// Converting tracedata to an SVG string, paths are drawn according to a Z-index
// the optional lcpr and qcpr are linear and quadratic control point radiuses
static string getsvgstring (const IndexedImage& ii, map<string,double> options)
{
  stringstream svgstr;

  options = checkoptions(options);
  // SVG start
  int w = (int) (ii.width * options.at("scale")), h = (int) (ii.height * options.at("scale"));
  string viewboxorviewport;
  if (options.at("viewbox")!=0)
    viewboxorviewport = fmt::format("viewBox=\"0 0 {} {}",w,h);
  else
    viewboxorviewport = fmt::format("width=\"{}\" height=\"{}\" ",w,h);

  svgstr << "<svg " << viewboxorviewport << "version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" ";

  if(options.at("desc")!=0)
  {
    svgstr << "desc=\"Created with ImageTracerC++ version "
           << versionnumber+"\" ";
  }
  svgstr << ">";

  // creating Z-index
  map <double, vector<int>> zindex;
  double label;

  // Layer loop
  for(size_t k=0; k<ii.layers.size(); k++)
  {
    // Path loop
    for(size_t pcnt=0; pcnt<ii.layers[k].size(); pcnt++)
    {
      // Label (Z-index key) is the startpoint of the path, linearized
      label = (ii.layers[k][pcnt][0][2] * w) + ii.layers[k][pcnt][0][1];
      // Creating new list if required
      if(!zindex.count(label))
        zindex[label] = vector<int>(2,0);

      // Adding layer and path number to list
      zindex[label][0] = k;
      zindex[label][1] = pcnt;
    }// End of path loop

  }// End of layer loop

  // Sorting Z-index is not required, TreeMap is sorted automatically

  // Drawing
  // Z-index loop
  string thisdesc = "";

  // An arbitrary color table for two bit images
  vector<array<uint8_t,3>> colors = {
    {32,32,32},
    {255,0,0}};

  for(auto& [key, value] : zindex)
  {
    if(options.at("desc")!=0)
      thisdesc = fmt::format("desc=\"l {} p {}\" ", value[0], value[1]);
    else
      thisdesc = ""; 

    svgpathstring(svgstr,
                  thisdesc,
                  ii.layers[value[0]][value[1]],
                  tosvgcolorstr(colors[value[0]]), 
                  options);
  }

  // SVG End
  svgstr << "</svg>";

  return svgstr.str();

}// End of getsvgstring()

static string tosvgcolorstr (const array<uint8_t,3>& c)
{
  return fmt::format("fill=\"rgb({},{},{})\" stroke=\"rgb({},{},{})\" stroke-width=\"1\"",
                c[0],c[1],c[2],
                c[0],c[1],c[2]);
}


// Saving a string as a file
static void savestring (const string& filename, const string& str)
{
  ofstream file(filename);

  if (file.good())
    file << str;

  file.close();
}

#if 0
static void die(const char *format, ...)
{
    va_list ap;
    va_start(ap,format); 
    
    vfprintf(stderr, format, ap);
    exit(-1);
}

#define CASE(s) if (S_ == s)
#define CASE2(s1,s2) if (S_ == s1 || S_==s2)

int main (int argc, char *argv[])
{
  vector<string> args;
  for (int i=0; i<argc; i++)
    args.push_back(argv[i]);

  // Defaults
  string output_file;
  map<string,double> options;

  size_t argp = 1;
  while(argp < args.size() && args[argp][0]=='-')
  {
    string S_ = args[argp++];
    CASE("--help")
    {
      print("ImageTracerBW - Trace a bw image\n"
            "\n"
            "Syntax:\n"
            "   ImageTracerBW [options] filename\n"
            "\n"
            "Options:\n"
            "  -o fn   Output filename\n"
            "  --ltres  Linear fitting tolerance\n"
            "  --qtres  Quadratic fitting tolarence. Set to a negative number to turn off\n"
            "  --pathomit  ...\n"
            "  --scale  ...\n"
            "  --simplifytolerance  ...\n"
            "  --roundcoords  ...\n"
            "  --lcpr  ...\n"
            "  --qcpr  ...\n"
            "  --desc  ...\n"
            "  --viewbox  ...\n"
            );
      exit(0);
    }
    CASE2("-o","--output")
    {
      output_file = args[argp++];
      continue;
    }
    CASE("--ltres")
    {
      options["ltres"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--qtres")
    {
      options["qtres"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--pathomit")
    {
      options["pathomit"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--scale")
    {
      options["scale"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--simplifytolerance")
    {
      options["simplifytolerance"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--roundcoords")
    {
      options["roundcoords"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--lcpr")
    {
      options["lcpr"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--qcpr")
    {
      options["qcpr"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--desc")
    {
      options["desc"] = atof(args[argp++].c_str());
      continue;
    }
    CASE("--viewbox")
    {
      options["viewbox"] = atof(args[argp++].c_str());
      continue;
    }
    die("Unknown parameter %s!\n",S_.c_str());
  }

  if (argp >= args.size())
    die("ERROR: there's no input filename. Basic usage: \n\n"
          "ImageTracer <filename>\n\n"
          "or\n\n"
          "ImageTracer --help\n");
  
  // Loading image, tracing, rendering SVG, saving SVG file
  input_filename = args[argp];

  string output_string;
  if (output_file.find(".giv") != string::npos)
    output_string = imageToGiv(input_filename, options);
  else
    output_string = imageToSVG(input_filename,options);
  savestring(output_file, output_string);

}// End of main()

#endif  
