# Intro

Pomelo is a graphic interactive program for creating 3D meshes of text. The resulting mesh may be exported as a STL file, e.g. for importing into Blender for further processing.

![Pomelo Screenshot](pomelo-screenshot-color.png)

See also a short intro at: https://www.youtube.com/watch?v=mCoGFMXl4X8

# Used technologies

- pango
- cairo
- gtkmm
- CGAL

# Usage

1. Run Pomelo
2. Enter desired text
3. Choose font
4. Press build button to build the "skeloton"
5. Choose profile, either Round or Curve
6. Choose profile parametes for round or edit the curve 
7. Choose Z-depth
8. Press Build
9. Export to STL or GLTF

![Pomelo Rounded Profile](pomelo-screenshot-simple.png)
![Pomelo Curve Profile](pomelo-screenshot-curve.png)

# Overview 

Pomelo works by a two phase algorithm:

1. Creation of a [Straight Skeleton](https://en.wikipedia.org/wiki/Straight_skeleton)
2. Creation of the mesh based on the straight skeleton, and the choosen profile.

# Profiles

Pomelo supports two types of profiles:

1. The round profile, creates a round edge, smoothing the transition vertical to horizontal with a round radius.
2. Through a Bezier based profile editor, that allow creating multiple profiles that are inserted on top of one another.

![Profile editor](profile-editor-example.png)

# Smoothing

The straight skeleton's sometimes create sharp and ugly geometry. This may be partly mitigated 
by turning on Settings/Smoothing option, which inserts additional nodes in the skeleton. However, the smoothing may itself create other artifacts. 

![Non smoothed skeleton](pomelo-non-smooth-geometry.png)
![Smoothed skeleton](pomelo-smooth-geometry.png)

# Technical description

See: [Technical description](TechnicalDescription.md)

# License

This program is released under the GPLv3 license. See COPYING for licensing details.

# Author

Dov Grobgeld <dov.grobgeld@gmail.com>

# Gallery

![Pomelo Screenshot 2](pomelo-screenshot2.png)
![Pomelo Screenshot 3](pomelo-screenshot3.png)
![Pomelo Screenshot 4](pomelo-screenshot4.png)
