//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Gunnar Roth: add support for linear and radial gradients(support xlink attr),
// shape gradient opaqueness, rounded rects, circles,ellipses. support a command (arc)  in pathes. 
// set new origin correctly to last postion on z command in a path( was set to 0,0 before).
// enable parsing of colors written as rgb()
// some code was inspired by code from Haiku OS
/*
* Copyright 2006-2007, Haiku. All rights reserved.
* Distributed under the terms of the MIT License.
*
* Authors:
* Stephan Aßmus <superstippi@gmx.de>
*/
//

#include <stdio.h>
#include <stdlib.h>



#include "agg_svg_gradient.h"
#include <vector>
#include "agg_trans_affine.h"
#include "agg_gradient_lut.h"






namespace agg {
    namespace svg {



        // constructor
        gradient::gradient()
            : 
            m_type(GRADIENT_LINEAR),
            m_id(""),
            m_colcnt(0),
            m_opaque(1.0)
        {
        }

        // destructor
        gradient::~gradient()
        {
            
        }

        // SetID
        void
            gradient::set_id(const char* id)
        {
            m_id = id;
        }

        // ID
        const char*
            gradient::id() const
        {
            return m_id.c_str();
        }

        // AddStop
        void
            gradient::add_stop(double offset, rgba8 color)
        {
            m_colcnt++;
            m_gradient_lut.add_color(offset,color);
        }

        // SetTransformation
        void
            gradient::set_transformation(const trans_affine& transform)
        {
            m_transform.multiply(transform);
        }
        // constructor
        linear_gradient::linear_gradient()
            : gradient()
        {
        }

        // destructor
        linear_gradient::~linear_gradient()
        {
        }

        // MakeGradient
        
        void linear_gradient::realize() 
        {

            set_type(GRADIENT_LINEAR);
            // setup the gradient transform
            point_d start(-lut_range, -lut_range);
            point_d end(lut_range, -lut_range);
            std::string coordinate;
            if (find_string("x1", &coordinate) )
                start.x = atof(coordinate.c_str());
            if (find_string("y1", &coordinate) )
                start.y = atof(coordinate.c_str());
            if (find_string("x2", &coordinate) )
                end.x = atof(coordinate.c_str());
            if (find_string("y2", &coordinate) )
                end.y = atof(coordinate.c_str());


            // the transformed parallelogram
            double parl[6];
            parl[0] = start.x;
            parl[1] = start.y;
            parl[2] = end.x;
            parl[3] = end.y;
            parl[4] = end.x - (end.y - start.y);
            parl[5] = end.y + (end.x - start.x);
            trans_affine transform(-lut_range, -lut_range, lut_range, lut_range, parl);
            m_transform.premultiply(transform);
            m_gradient_lut.build_lut(1.0);      
        }

        // constructor
        radial_gradient::radial_gradient()
            : gradient()
        {
        }

        // destructor
        radial_gradient::~radial_gradient()
        {
        }

        // MakeGradient
        
        void    radial_gradient::realize()
        {
            //printf("SVGRadialGradient::MakeGradient()\n");
            // TODO: handle userSpaceOnUse/objectBoundingBox
            
            set_type(GRADIENT_CIRCULAR);

            double cx = 0.0;
            double cy = 0.0;
            double r = 100.0;

            std::string value;
            if (find_string("cx", &value) )
                cx = atof(value.c_str());
            if (find_string("cy", &value) )
                cy = atof(value.c_str());
            if (find_string("r", &value) )
                r = atof(value.c_str());
/*
            if (FindString("fx", &value) )
            {
                fx = atof(value.c_str());
                if (FindString("fy", &value) )
                    fy = atof(value.c_str());
                SetType(GRADIENT_CIRCULAR_FOCAL);
            }
*/


            // the transformed parallelogram
            double parl[6];
            parl[0] = cx - r;
            parl[1] = cy - r;
            parl[2] = cx + r;
            parl[3] = cy - r;
            parl[4] = cx + r;
            parl[5] = cy + r;

            trans_affine transform(-lut_range, -lut_range, lut_range, lut_range, parl);
            m_transform.premultiply(transform);
            m_gradient_lut.build_lut(1.0);
        }
        template<>
        template<>
        gradient_lut_opaque<color_interpolator<gray8>, 256u>::gradient_lut_opaque(const gradient::color_func_type& rgb_lut)
          : m_color_lut(256)
        {
          for (int i=0; i<256; i++)
            m_color_lut[i] = rgb_lut[i];
        }


    } // namespace svg
} // namespace agg
