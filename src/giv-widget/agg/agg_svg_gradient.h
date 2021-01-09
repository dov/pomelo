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

#ifndef SVG_GRADIENTS_H
#define SVG_GRADIENTS_H


#include "agg_color_rgba.h"
#include "agg_trans_affine.h"
#include <string.h>
#include <string>
#include <map>
#include "agg_gradient_lut.h"


namespace agg {
    namespace svg {

        template<class ColorInterpolator, 
            unsigned ColorLutSize=256> class gradient_lut_opaque
        {
        public:
            typedef ColorInterpolator interpolator_type;
            typedef typename interpolator_type::color_type color_type;
            enum { color_lut_size = ColorLutSize };

            //--------------------------------------------------------------------
            gradient_lut_opaque() : m_color_lut(color_lut_size) {}

            // Declare an assignment operator from another type.
            template<typename W>
            gradient_lut_opaque(const W& w);

            // Build Gradient Lut
            // First, call remove_all(), then add_color() at least twice, 
            // then build_lut(). Argument "offset" in add_color must be 
            // in range [0...1] and defines a color stop as it is described 
            // in SVG specification, section Gradients and Patterns. 
            // The simplest linear gradient is:
            //    gradient_lut.add_color(0.0, start_color);
            //    gradient_lut.add_color(1.0, end_color);
            //--------------------------------------------------------------------
            void remove_all();
            void add_color(double offset, const color_type& color);
            void build_lut(double opaque );

            // Size-index Interface. This class can be used directly as the 
            // ColorF in span_gradient. All it needs is two access methods 
            // size() and operator [].
            //--------------------------------------------------------------------
            static unsigned size() 
            { 
                return color_lut_size; 
            }
            const color_type& operator [] (unsigned i) const 
            { 
                return m_color_lut[i]; 
            }

        private:
            //--------------------------------------------------------------------
            struct color_point
            {
                double     offset;
                color_type color;

                color_point() {}
                color_point(double off, const color_type& c) : 
                offset(off), color(c)
                {
                    if(offset < 0.0) offset = 0.0;
                    if(offset > 1.0) offset = 1.0;
                }
            };
            typedef agg::pod_bvector<color_point, 4> color_profile_type;
            typedef agg::pod_array<color_type>       color_lut_type;

            static bool offset_less(const color_point& a, const color_point& b)
            {
                return a.offset < b.offset;
            }
            static bool offset_equal(const color_point& a, const color_point& b)
            {
                return a.offset == b.offset;
            }

            //--------------------------------------------------------------------
            color_profile_type  m_color_profile;
            color_lut_type      m_color_lut;

        };


        //------------------------------------------------------------------------
        template<class T, unsigned S>
        void gradient_lut_opaque<T,S>::remove_all()
        { 
            m_color_profile.remove_all(); 
        }

        //------------------------------------------------------------------------
        template<class T, unsigned S>
        void gradient_lut_opaque<T,S>::add_color(double offset, const color_type& color)
        {
            m_color_profile.add(color_point(offset, color));
        }

        //------------------------------------------------------------------------
        template<class T, unsigned S>
        void gradient_lut_opaque<T,S>::build_lut(double opaque)
        {
            quick_sort(m_color_profile, offset_less);
            m_color_profile.cut_at(remove_duplicates(m_color_profile, offset_equal));
            if(m_color_profile.size() >= 2)
            {
                unsigned i;
                unsigned start = uround(m_color_profile[0].offset * color_lut_size);
                unsigned end;
                color_type c = m_color_profile[0].color;
                c.opacity(c.opacity() * opaque);
                for(i = 0; i < start; i++) 
                {
                    m_color_lut[i] = c;
                }
                for(i = 1; i < m_color_profile.size(); i++)
                {
                    end  = uround(m_color_profile[i].offset * color_lut_size);
                    color_type c1 = m_color_profile[i-1].color;
                    c1.opacity(c1.opacity() * opaque);
                    color_type c2 = m_color_profile[i].color;
                    c2.opacity(c2.opacity() * opaque);
                    interpolator_type ci(c1, 
                        c2 ,
                        end - start + 1);
                    while(start < end)
                    {
                        m_color_lut[start] = ci.color();
                        ++ci;
                        ++start;
                    }
                }
                c = m_color_profile.last().color;
                c.opacity(c.opacity() * opaque);
                for(; end < m_color_lut.size(); end++)
                {
                    m_color_lut[end] = c;
                }
            }
        }


        class gradient {
        public:
            static const int lut_range = 128;
            typedef gradient_lut_opaque<agg::color_interpolator<agg::rgba8>, lut_range * 2> color_func_type;
            enum gradients_type {
                GRADIENT_LINEAR = 0,
                GRADIENT_CIRCULAR,
                GRADIENT_CIRCULAR_FOCAL,
                GRADIENT_DIAMOND,
                GRADIENT_CONIC,
                GRADIENT_XY,
                GRADIENT_SQRT_XY
            };
            gradient();
            virtual ~gradient();

            void    set_id(const char* id);
            const char* id() const;
            void set_opaque(double opaque)
            {
                if(opaque != m_opaque)
                {
                    m_opaque = opaque;
                    m_gradient_lut.build_lut(m_opaque);
                }
            }
            virtual void    add_stop(double offset, rgba8 color);
            void    set_transformation(const trans_affine& transform);
            virtual void realize() = 0;
            virtual gradient *clone() = 0;

            
            void add_string(const char *name, const char *string)
            {
                m_mapStrings[name] = string;
            }

            color_func_type &lut()
            {
                return m_gradient_lut;
            }

            void    set_type(gradients_type type)
            {
                m_type = type;
            }
            gradients_type  type() const
            { return m_type; }

            const trans_affine & transform()
            {
                return m_transform;
            }
            unsigned count_colors()
            {
                return m_colcnt;
            }
            std::string xlink()
            {
                std::string xlink;
                find_string("xlink:href",&xlink);
                return xlink;
            }

        protected:
            bool find_string(const char * key,std::string * pVal ) const
            {
                std::map<std::string,std::string>::const_iterator it = m_mapStrings.find(key);
                if(it == m_mapStrings.end())
                    return false;
                *pVal=it->second;
                return true;
            }
            trans_affine m_transform;
            color_func_type m_gradient_lut;
            unsigned m_colcnt;
        private:
            std::map<std::string,std::string> m_mapStrings; 
            gradients_type  m_type;
            double m_opaque;
            std::string m_id;
        };

        class linear_gradient : public gradient {
        public:
            linear_gradient();
            virtual ~linear_gradient();

            void realize() override;
            gradient *clone() override {
                return new linear_gradient(*this);
            }
        };

        class radial_gradient : public gradient {
        public:
            radial_gradient();
            virtual ~radial_gradient();
            void realize() override;
            gradient *clone() override {
                return new radial_gradient(*this);
            }
        };


        template<>
        template<>
        gradient_lut_opaque<color_interpolator<gray8>, 256u>::gradient_lut_opaque(const gradient::color_func_type& rgb_lut);

    } // namespace svg
} // namespace agg

#endif // SVG_GRADIENTS_H
