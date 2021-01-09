//======================================================================
//  giv_agg_arrowhead.h - Symmetric arrow heads an tails for giv
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue May  6 23:21:37 2008
//----------------------------------------------------------------------

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
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
// Simple arrowhead/arrowtail generator 
//
//----------------------------------------------------------------------------
#ifndef GIVAGG_ARROWHEAD_INCLUDED
#define GIVAGG_ARROWHEAD_INCLUDED

#include "agg/agg_basics.h"

namespace givagg
{

    //===============================================================arrowhead
    //
    // See implementation agg_arrowhead.cpp 
    //
    class arrowhead
    {
    public:
        arrowhead();

        void set_arrow(double d1, double d2, double d3, double d4, double d5)
        {
            m_d1 = d1;
            m_d2 = d2;
            m_d3 = d3;
            m_d4 = d4;
            m_d5 = d5;
        }
        
        void head()    { m_head_flag = true; }
        void no_head() { m_head_flag = false; }

        void tail()    { m_tail_flag = true;  }
        void no_tail() { m_tail_flag = false; }

        void rewind(unsigned path_id);
        unsigned vertex(double* x, double* y);

    private:
        double   m_d1;
        double   m_d2;
        double   m_d3;
        double   m_d4;
        double   m_d5;
        bool     m_head_flag;
        bool     m_tail_flag;
        double   m_coord[16];
        unsigned m_cmd[8];
        unsigned m_curr_id;
        unsigned m_curr_coord;
    };

}

#endif
