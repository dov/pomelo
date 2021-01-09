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
//----------------------------------------------------------------------------
//
// SVG parser.
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "agg_svg_parser.h"
#include "expat.h"
#include "agg_svg_gradient.h"

namespace agg
{
namespace svg
{
    struct named_color
    {
        char  name[22];
        int8u r, g, b, a;
    };

    named_color colors[] = 
    {
        { "aliceblue",240,248,255, 255 },
        { "antiquewhite",250,235,215, 255 },
        { "aqua",0,255,255, 255 },
        { "aquamarine",127,255,212, 255 },
        { "azure",240,255,255, 255 },
        { "beige",245,245,220, 255 },
        { "bisque",255,228,196, 255 },
        { "black",0,0,0, 255 },
        { "blanchedalmond",255,235,205, 255 },
        { "blue",0,0,255, 255 },
        { "blueviolet",138,43,226, 255 },
        { "brown",165,42,42, 255 },
        { "burlywood",222,184,135, 255 },
        { "cadetblue",95,158,160, 255 },
        { "chartreuse",127,255,0, 255 },
        { "chocolate",210,105,30, 255 },
        { "coral",255,127,80, 255 },
        { "cornflowerblue",100,149,237, 255 },
        { "cornsilk",255,248,220, 255 },
        { "crimson",220,20,60, 255 },
        { "cyan",0,255,255, 255 },
        { "darkblue",0,0,139, 255 },
        { "darkcyan",0,139,139, 255 },
        { "darkgoldenrod",184,134,11, 255 },
        { "darkgray",169,169,169, 255 },
        { "darkgreen",0,100,0, 255 },
        { "darkgrey",169,169,169, 255 },
        { "darkkhaki",189,183,107, 255 },
        { "darkmagenta",139,0,139, 255 },
        { "darkolivegreen",85,107,47, 255 },
        { "darkorange",255,140,0, 255 },
        { "darkorchid",153,50,204, 255 },
        { "darkred",139,0,0, 255 },
        { "darksalmon",233,150,122, 255 },
        { "darkseagreen",143,188,143, 255 },
        { "darkslateblue",72,61,139, 255 },
        { "darkslategray",47,79,79, 255 },
        { "darkslategrey",47,79,79, 255 },
        { "darkturquoise",0,206,209, 255 },
        { "darkviolet",148,0,211, 255 },
        { "deeppink",255,20,147, 255 },
        { "deepskyblue",0,191,255, 255 },
        { "dimgray",105,105,105, 255 },
        { "dimgrey",105,105,105, 255 },
        { "dodgerblue",30,144,255, 255 },
        { "firebrick",178,34,34, 255 },
        { "floralwhite",255,250,240, 255 },
        { "forestgreen",34,139,34, 255 },
        { "fuchsia",255,0,255, 255 },
        { "gainsboro",220,220,220, 255 },
        { "ghostwhite",248,248,255, 255 },
        { "gold",255,215,0, 255 },
        { "goldenrod",218,165,32, 255 },
        { "gray",128,128,128, 255 },
        { "green",0,128,0, 255 },
        { "greenyellow",173,255,47, 255 },
        { "grey",128,128,128, 255 },
        { "honeydew",240,255,240, 255 },
        { "hotpink",255,105,180, 255 },
        { "indianred",205,92,92, 255 },
        { "indigo",75,0,130, 255 },
        { "ivory",255,255,240, 255 },
        { "khaki",240,230,140, 255 },
        { "lavender",230,230,250, 255 },
        { "lavenderblush",255,240,245, 255 },
        { "lawngreen",124,252,0, 255 },
        { "lemonchiffon",255,250,205, 255 },
        { "lightblue",173,216,230, 255 },
        { "lightcoral",240,128,128, 255 },
        { "lightcyan",224,255,255, 255 },
        { "lightgoldenrodyellow",250,250,210, 255 },
        { "lightgray",211,211,211, 255 },
        { "lightgreen",144,238,144, 255 },
        { "lightgrey",211,211,211, 255 },
        { "lightpink",255,182,193, 255 },
        { "lightsalmon",255,160,122, 255 },
        { "lightseagreen",32,178,170, 255 },
        { "lightskyblue",135,206,250, 255 },
        { "lightslategray",119,136,153, 255 },
        { "lightslategrey",119,136,153, 255 },
        { "lightsteelblue",176,196,222, 255 },
        { "lightyellow",255,255,224, 255 },
        { "lime",0,255,0, 255 },
        { "limegreen",50,205,50, 255 },
        { "linen",250,240,230, 255 },
        { "magenta",255,0,255, 255 },
        { "maroon",128,0,0, 255 },
        { "mediumaquamarine",102,205,170, 255 },
        { "mediumblue",0,0,205, 255 },
        { "mediumorchid",186,85,211, 255 },
        { "mediumpurple",147,112,219, 255 },
        { "mediumseagreen",60,179,113, 255 },
        { "mediumslateblue",123,104,238, 255 },
        { "mediumspringgreen",0,250,154, 255 },
        { "mediumturquoise",72,209,204, 255 },
        { "mediumvioletred",199,21,133, 255 },
        { "midnightblue",25,25,112, 255 },
        { "mintcream",245,255,250, 255 },
        { "mistyrose",255,228,225, 255 },
        { "moccasin",255,228,181, 255 },
        { "navajowhite",255,222,173, 255 },
        { "navy",0,0,128, 255 },
        { "oldlace",253,245,230, 255 },
        { "olive",128,128,0, 255 },
        { "olivedrab",107,142,35, 255 },
        { "orange",255,165,0, 255 },
        { "orangered",255,69,0, 255 },
        { "orchid",218,112,214, 255 },
        { "palegoldenrod",238,232,170, 255 },
        { "palegreen",152,251,152, 255 },
        { "paleturquoise",175,238,238, 255 },
        { "palevioletred",219,112,147, 255 },
        { "papayawhip",255,239,213, 255 },
        { "peachpuff",255,218,185, 255 },
        { "peru",205,133,63, 255 },
        { "pink",255,192,203, 255 },
        { "plum",221,160,221, 255 },
        { "powderblue",176,224,230, 255 },
        { "purple",128,0,128, 255 },
        { "red",255,0,0, 255 },
        { "rosybrown",188,143,143, 255 },
        { "royalblue",65,105,225, 255 },
        { "saddlebrown",139,69,19, 255 },
        { "salmon",250,128,114, 255 },
        { "sandybrown",244,164,96, 255 },
        { "seagreen",46,139,87, 255 },
        { "seashell",255,245,238, 255 },
        { "sienna",160,82,45, 255 },
        { "silver",192,192,192, 255 },
        { "skyblue",135,206,235, 255 },
        { "slateblue",106,90,205, 255 },
        { "slategray",112,128,144, 255 },
        { "slategrey",112,128,144, 255 },
        { "snow",255,250,250, 255 },
        { "springgreen",0,255,127, 255 },
        { "steelblue",70,130,180, 255 },
        { "tan",210,180,140, 255 },
        { "teal",0,128,128, 255 },
        { "thistle",216,191,216, 255 },
        { "tomato",255,99,71, 255 },
        { "turquoise",64,224,208, 255 },
        { "violet",238,130,238, 255 },
        { "wheat",245,222,179, 255 },
        { "white",255,255,255, 255 },
        { "whitesmoke",245,245,245, 255 },
        { "yellow",255,255,0, 255 },
        { "yellowgreen",154,205,50, 255 },
        { "zzzzzzzzzzz",0,0,0, 0 }
    }; 

    //-------------------------------------------------------------
    double parse_double(const char* str)
    {
        while(*str == ' ') ++str;
        double value = atof(str);
        // handle percent
        int32 length = strlen(str);
        if (str[length - 1] == '%')
        value /= 100.0;
        return value;
    }
  
    // parse_url
    char*
    parse_url(const char* str)
    {
        const char* begin = str;
        while (*begin != '#')
            begin++;
    
        begin++;
        const char* end = begin;
        while (*end != ')')
            end++;
    
        end--;
    
        int32 length = end - begin + 2;
        char* result = new char[length];
        memcpy(result, begin, length - 1);
        result[length - 1] = 0;
    
        return result;
    }

    //------------------------------------------------------------------------
    parser::~parser()
    {
        delete [] m_attr_value;
        delete [] m_attr_name;
        delete [] m_buf;
        delete [] m_title;
    }

    //------------------------------------------------------------------------
    parser::parser(path_renderer& path) :
        m_path(path),
        m_tokenizer(),
        m_buf(new char[buf_size]),
        m_title(new char[256]),
        m_title_len(0),
        m_title_flag(false),
        m_path_flag(false),
        m_attr_name(new char[128]),
        m_attr_value(new char[1024]),
        m_attr_name_len(127),
        m_attr_value_len(1023),
        m_swap_red_blue(false),
        m_width_in_mm(-1),
        m_height_in_mm(-1),
        m_tags_ignored(false)
    {
        m_title[0] = 0;
        for (int i=0; i<4; i++)
            m_view_box[i] = -1;
    }

    //------------------------------------------------------------------------
    void parser::parse(const char* fname)
    {
        char msg[1024];
        XML_Parser p = XML_ParserCreate(NULL);
        if(p == 0) 
        {
            throw exception("Couldn't allocate memory for parser");
        }

        XML_SetUserData(p, this);
        XML_SetElementHandler(p, start_element, end_element);
        XML_SetCharacterDataHandler(p, content);

        FILE* fd = fopen(fname, "r");
        if(fd == 0)
        {
            sprintf(msg, "Couldn't open file %s", fname);
                    throw exception(msg);
        }

        bool done = false;
        do
        {
            size_t len = fread(m_buf, 1, buf_size, fd);
            done = len < buf_size;
            if(!XML_Parse(p, m_buf, len, done))
            {
                sprintf(msg,
                        "%s at line %d\n",
                        XML_ErrorString(XML_GetErrorCode(p)),
                        (int)XML_GetCurrentLineNumber(p));
                throw exception(msg);
            }
        }
        while(!done);
        fclose(fd);
        XML_ParserFree(p);

        char* ts = m_title;
        while(*ts)
        {
            if(*ts < ' ') *ts = ' ';
            ++ts;
        }
    }

    void parser::parse_string(const char* svg_string)
    {
        char msg[1024];
        XML_Parser p = XML_ParserCreate(NULL);
        if(p == 0) 
        {
            throw exception("Couldn't allocate memory for parser");
        }

        XML_SetUserData(p, this);
        XML_SetElementHandler(p, start_element, end_element);
        XML_SetCharacterDataHandler(p, content);

        size_t len = strlen(svg_string);
        if(!XML_Parse(p, svg_string, len, true))
        {
          sprintf(msg,
                  "%s at line %d\n",
                  XML_ErrorString(XML_GetErrorCode(p)),
                  (int)XML_GetCurrentLineNumber(p));
          throw exception(msg);
        }
        XML_ParserFree(p);
    }

    //------------------------------------------------------------------------
    void parser::start_element(void* data, const char* el, const char** attr)
    {
        parser& self = *(parser*)data;

        if (strcmp(el, "svg") == 0)
        {
          self.parse_svg(attr);
        }
        else
        if(strcmp(el, "title") == 0)
        {
            self.m_title_flag = true;
        }
        else
        if(strcmp(el, "svg") == 0)
        {
            self.parse_svg(attr);
        }
        else
        if(strcmp(el, "g") == 0)
        {
            self.m_path.push_attr();
            self.parse_attr(attr);
        }
        else
        if(strcmp(el, "path") == 0)
        {
            if(self.m_path_flag)
            {
                throw exception("start_element: Nested path");
            }
            self.m_path.begin_path();
            self.parse_path(attr);
            self.m_path.end_path();
            self.m_path_flag = true;
        }
        else
        if(strcmp(el, "rect") == 0) 
        {
            self.parse_rect(attr);
        }
        else
        if(strcmp(el, "circle") == 0) 
        {
            self.parse_circle(attr);
        }
        else
        if (strcmp(el, "ellipse") == 0)
        {
          self.parse_ellipse(attr);
        }
        else
        if(strcmp(el, "line") == 0) 
        {
            self.parse_line(attr);
        }
        else
        if(strcmp(el, "polyline") == 0) 
        {
            self.parse_poly(attr, false);
        }
        else
        if(strcmp(el, "polygon") == 0) 
        {
            self.parse_poly(attr, true);
        }
        else
        if (strcmp(el, "linearGradient") == 0 || strcmp(el, "radialGradient") == 0)
        {
            self.parse_gradient(attr, strcmp(el, "radialGradient") == 0);
        }
        else
        if (strcmp(el, "stop") == 0)
        {
            self.parse_gradient_stop(attr);
        }
        //else
        //if(strcmp(el, "<OTHER_ELEMENTS>") == 0) 
        //{
        //}
        // . . .
        else
        {
            // fprintf(stderr, "SVGParser ignoring tag: \"%s\"\n", el);
            self.m_tags_ignored = true;
        }
    } 


    //------------------------------------------------------------------------
    void parser::end_element(void* data, const char* el)
    {
        parser& self = *(parser*)data;

        if(strcmp(el, "title") == 0)
        {
            self.m_title_flag = false;
        }
        else
        if(strcmp(el, "g") == 0)
        {
            self.m_path.pop_attr();
        }
        else
        if(strcmp(el, "path") == 0)
        {
            self.m_path_flag = false;
        }
        else
        if (strcmp(el, "linearGradient") == 0 || strcmp(el, "radialGradient") == 0)
        {
            self.m_path.end_gradient();
        }
        //else
        //if(strcmp(el, "<OTHER_ELEMENTS>") == 0) 
        //{
        //}
        // . . .
    }


    //------------------------------------------------------------------------
    void parser::content(void* data, const char* s, int len)
    {
        parser& self = *(parser*)data;

        // m_title_flag signals that the <title> tag is being parsed now.
        // The following code concatenates the pieces of content of the <title> tag.
        if(self.m_title_flag)
        {
            if(len + self.m_title_len > 255) len = 255 - self.m_title_len;
            if(len > 0) 
            {
                memcpy(self.m_title + self.m_title_len, s, len);
                self.m_title_len += len;
                self.m_title[self.m_title_len] = 0;
            }
        }
    }

    //------------------------------------------------------------------------
    void parser::parse_svg(const char** attr)
    {
        int i;
        for(i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "width")==0)
            {
                m_width_in_mm = parse_distance_to_mm(attr[i+1]);;
                m_path.set_width_in_mm(m_width_in_mm);
            }
            if (strcmp(attr[i], "height")==0)
            {
                m_height_in_mm = parse_distance_to_mm(attr[i+1]);;
                m_path.set_height_in_mm(m_height_in_mm);
            }
            if (strcmp(attr[i], "viewBox")==0)
                parse_view_box(attr[i+1], m_view_box);
        }
    }

    //-------------------------------------------------------------
    static bool is_numeric(char c)
    {
        return c!=0 && strchr("0123456789+-.eE", c) != 0;
    }

    double parser::parse_distance_to_mm(const char *distance_string)
    {
        // parse under the assumption that the number may only
        // contain digits and decimal point.
        const char *p = distance_string;
        while(*p == ' ')
            p++;
        const char *start_pos = p;
        while(is_numeric(*p) || *p=='.')
            p++;
        const char * end_pos=p;
        double distance = strtod(start_pos, (char**)&end_pos);
        // Get the unit
        while(*p == ' ')
            p++;
        double multiplier = 1.0;

        // Treat all these as a point
        if (strlen(p)==0
            || strcmp(p,"px")==0
            || strcmp(p,"pt")==0
            // The following three don't have an absolute meaning
            || strcmp(p,"%")==0
            || strcmp(p,"em")==0
            || strcmp(p,"ex")==0
            )
            multiplier = 0.35277778;
        else if (strcmp(p, "pc")==0)
            multiplier = 4.2175176;
        else if (strcmp(p, "cm")==0)
            multiplier = 10.;
        else if (strcmp(p, "m")==0)
            multiplier = 1000.;
        else if (strcmp(p, "in")==0)
            multiplier = 25.4;

        return distance * multiplier;
    }

    void parser::parse_view_box(const char *str,
                                  // output
                                  double *vbox
                                  )
    {
        const char *p=str;
        for (int i=0; i<4; i++) {
            if (!(is_numeric(*p) || *p=='-' || *p=='.'))
                throw exception("parse_view_box: Malformatted viewbox: %s",str);

            vbox[i] = atof(p);
            if (i==3)
                break;
            while(*p && *p != ' ')
                ++p;
            while(*p == ' ')
                ++p;
        }
    }

    //------------------------------------------------------------------------
    void parser::parse_attr(const char** attr)
    {
        int i;
        for(i = 0; attr[i]; i += 2)
        {
            if(strcmp(attr[i], "style") == 0)
            {
                parse_style(attr[i + 1]);
            }
            else
            {
                parse_attr(attr[i], attr[i + 1]);
            }
        }
    }

    //-------------------------------------------------------------
    void parser::parse_path(const char** attr)
    {
        int i;

        for(i = 0; attr[i]; i += 2)
        {
            // The <path> tag can consist of the path itself ("d=") 
            // as well as of other parameters like "style=", "transform=", etc.
            // In the last case we simply rely on the function of parsing 
            // attributes (see 'else' branch).
            if(strcmp(attr[i], "d") == 0)
            {
                m_tokenizer.set_path_str(attr[i + 1]);
                m_path.parse_path(m_tokenizer);
            }
            else
            {
                // Create a temporary single pair "name-value" in order
                // to avoid multiple calls for the same attribute.
                const char* tmp[4];
                tmp[0] = attr[i];
                tmp[1] = attr[i + 1];
                tmp[2] = 0;
                tmp[3] = 0;
                parse_attr(tmp);
            }
        }
    }


    //-------------------------------------------------------------
    int cmp_color(const void* p1, const void* p2)
    {
        return strcmp(((named_color*)p1)->name, ((named_color*)p2)->name);
    }

    //-------------------------------------------------------------
    rgba8 parse_color(const char* str, bool swap_red_blue)
    {
        while(*str == ' ') ++str;
        unsigned c = 0;
        if(*str == '#')
        {
            char newclr[7];
            if(strlen(str+1) == 3) {
                newclr[0]=newclr[1]=*(str+1);
                newclr[2]=newclr[3]=*(str+2);
                newclr[4]=newclr[5]=*(str+3);
                newclr[6] = 0;
                sscanf(newclr, "%x", &c);
                if (swap_red_blue)
                    return bgr8_packed(c);
                return rgb8_packed(c);
            } else {
               sscanf(str + 1, "%x", &c);
               if (swap_red_blue)
                 return bgr8_packed(c);
               return rgb8_packed(c);
            }
        }
        // parse rgb and rgba syntax
        else if (strncmp(str,"rgb(",4)==0
                 || strncmp(str,"rgba(",5)==0)
        {
            bool has_alpha = (strncmp(str,"rgba(",5)==0);
            double num[4];

            if (!has_alpha)
                num[3] = 1.0;

            str+=4 + int(has_alpha);

            // extract the numbers and support % notation
            for (int i=0; i<3+int(has_alpha); i++)
            {
                num[i] = atof(str);
                while (isdigit(*str) || *str=='.')
                    str++;

                
                if (*str=='%')
                    num[i]*=0.01;
                // alpha appears to be treated differently in rgba() in svg
                else if (i<3)
                    num[i]/= 255.0;

                while (*str && !isdigit(*str))
                    str++;
            }
            if (swap_red_blue)
                return rgba(num[2],num[1],num[0],num[3]);
            return rgba(num[0],num[1],num[2],num[3]);
        }
        else
        {
            named_color c;
            unsigned len = strlen(str);
            if(len > sizeof(c.name) - 1)
            {
                throw exception("parse_color: Invalid color name '%s'", str);
            }
            strcpy(c.name, str);
            const void* p = bsearch(&c, 
                                    colors, 
                                    sizeof(colors) / sizeof(colors[0]), 
                                    sizeof(colors[0]), 
                                    cmp_color);
            if(p == 0)
            {
                throw exception("parse_color: Invalid color name '%s'", str);
            }
            const named_color* pc = (const named_color*)p;
            if (swap_red_blue)
                return rgba8(pc->b, pc->g, pc->r, pc->a);
            return rgba8(pc->r, pc->g, pc->b, pc->a);
        }
    }


    //-------------------------------------------------------------
    bool parser::parse_attr(const char* name, const char* value)
    {
        if(strcmp(name, "style") == 0)
        {
            parse_style(value);
        }
        else
        if(strcmp(name, "fill") == 0)
        {
            if(strcmp(value, "none") == 0)
            {
                m_path.fill_none();
            }
            else if (strncmp(value, "url", 3) == 0) 
            {
                char* url = parse_url(value);
                m_path.fill_url(url);
                delete[] url;
            }
            else
            {
                m_path.fill(parse_color(value,m_swap_red_blue));
            }
        }
        else
        if(strcmp(name, "fill-opacity") == 0)
        {
            m_path.fill_opacity(parse_double(value));
        }
        else
        if(strcmp(name, "opacity") == 0)
        {
            m_path.opacity(parse_double(value));
        }
        else
        if(strcmp(name, "fill-rule") == 0) 
        {
           m_path.even_odd(strcmp(value, "evenodd") == 0);
        }
        else
        if(strcmp(name, "stroke") == 0)
        {
            if(strcmp(value, "none") == 0)
            {
                m_path.stroke_none();
            }
            else if (strncmp(value, "url", 3) == 0) {
                char* url = parse_url(value);
                m_path.stroke_url(url);
                delete[] url;
            }
            else
            {
                m_path.stroke(parse_color(value,m_swap_red_blue));
            }
        }
        else
        if(strcmp(name, "stroke-width") == 0)
        {
            m_path.stroke_width(parse_double(value));
        }
        else
        if(strcmp(name, "stroke-linecap") == 0)
        {
            if(strcmp(value, "butt") == 0)        m_path.line_cap(butt_cap);
            else if(strcmp(value, "round") == 0)  m_path.line_cap(round_cap);
            else if(strcmp(value, "square") == 0) m_path.line_cap(square_cap);
        }
        else
        if(strcmp(name, "stroke-linejoin") == 0)
        {
            if(strcmp(value, "miter") == 0)      m_path.line_join(miter_join);
            else if(strcmp(value, "round") == 0) m_path.line_join(round_join);
            else if(strcmp(value, "bevel") == 0) m_path.line_join(bevel_join);
        }
        else
        if(strcmp(name, "stroke-miterlimit") == 0)
        {
            m_path.miter_limit(parse_double(value));
        }
        else
        if(strcmp(name, "stroke-opacity") == 0)
        {
            m_path.stroke_opacity(parse_double(value));
        }
        else
        if(strcmp(name, "transform") == 0)
        {
            m_path.transform().premultiply(parse_transform(value));
        }
        else
        if (strcmp(name, "stop-color") == 0) 
        {
            m_gradient_stop_color = parse_color(value, m_swap_red_blue);
        } 
        else
        if (strcmp(name, "stop-opacity") == 0) {
          m_gradient_stop_color.opacity(parse_double(value));
        }
        else
        if (strncmp(value, "url", 3) == 0) 
        {
          char* url = parse_url(value);
          m_path.fill_url(url);
          delete[] url;
        } 
        //else
        //if(strcmp(el, "<OTHER_ATTRIBUTES>") == 0) 
        //{
        //}
        // . . .
        else
        {
            return false;
        }
        return true;
    }



    //-------------------------------------------------------------
    void parser::copy_name(const char* start, const char* end)
    {
        unsigned len = unsigned(end - start);
        if(m_attr_name_len == 0 || len > m_attr_name_len)
        {
            delete [] m_attr_name;
            m_attr_name = new char[len + 1];
            m_attr_name_len = len;
        }
        if(len) memcpy(m_attr_name, start, len);
        m_attr_name[len] = 0;
    }



    //-------------------------------------------------------------
    void parser::copy_value(const char* start, const char* end)
    {
        unsigned len = unsigned(end - start);
        if(m_attr_value_len == 0 || len > m_attr_value_len)
        {
            delete [] m_attr_value;
            m_attr_value = new char[len + 1];
            m_attr_value_len = len;
        }
        if(len) memcpy(m_attr_value, start, len);
        m_attr_value[len] = 0;
    }

    //-------------------------------------------------------------
    void parser::set_swap_red_blue(bool swap_red_blue)
    {
        m_swap_red_blue = swap_red_blue;
    }

    //-------------------------------------------------------------
    bool parser::parse_name_value(const char* nv_start, const char* nv_end)
    {
        const char* str = nv_start;
        while(str < nv_end && *str != ':') ++str;

        const char* val = str;

        // Right Trim
        while(str > nv_start && 
            (*str == ':' || isspace(*str))) --str;
        ++str;

        copy_name(nv_start, str);

        while(val < nv_end && (*val == ':' || isspace(*val))) ++val;
        
        copy_value(val, nv_end);
        return parse_attr(m_attr_name, m_attr_value);
    }



    //-------------------------------------------------------------
    void parser::parse_style(const char* str)
    {
        while(*str)
        {
            // Left Trim
            while(*str && isspace(*str)) ++str;
            const char* nv_start = str;
            while(*str && *str != ';') ++str;
            const char* nv_end = str;

            // Right Trim
            while(nv_end > nv_start && 
                (*nv_end == ';' || isspace(*nv_end))) --nv_end;
            ++nv_end;

            parse_name_value(nv_start, nv_end);
            if(*str) ++str;
        }

    }


    //-------------------------------------------------------------
    void parser::parse_rect(const char** attr)
    {
        int i;
        double x = 0.0;
        double y = 0.0;
        double w = 0.0;
        double h = 0.0;
        double rx = 0.0;
        double ry = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "x") == 0)      x = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y") == 0)      y = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "width") == 0)  w = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "height") == 0) h = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "rx") == 0)     rx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "ry") == 0)     ry = parse_double(attr[i + 1]);
            }
        }


        if(w != 0.0 && h != 0.0)
        {
            if(w < 0.0) throw exception("parse_rect: Invalid width: %f", w);
            if(h < 0.0) throw exception("parse_rect: Invalid height: %f", h);
            if(rx>0 || ry>0) {
                agg::rounded_rect rrect(x,y,x+w,y+h,rx>ry?rx:ry);
                m_path.concat_path(rrect);
            } else {
               m_path.move_to(x,     y);
               m_path.line_to(x + w, y);
               m_path.line_to(x + w, y + h);
               m_path.line_to(x,     y + h);
               m_path.close_subpath();
            }
        }
        m_path.end_path();
    }

    //-------------------------------------------------------------
    void parser::parse_circle(const char** attr)
    {
        int i;
        double x0 = 0.0;
        double y0 = 0.0;
        double radius = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "cx") == 0) x0 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "cy") == 0) y0 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "r") == 0) radius = parse_double(attr[i + 1]);
            }
        }

        // Make a bezier approximation. See: http://spencermortensen.com/articles/bezier-circle/
        double c =  0.55191502449*radius;
        double r = radius;
        m_path.move_to(x0,y0+r);
        m_path.curve4(x0+c,y0+r,  x0+r,y0+c,  x0+r,y0);
        m_path.curve4(x0+r,y0-c,  x0+c,y0-r,  x0,y0-r);
        m_path.curve4(x0-c,y0-r,  x0-r,y0-c,  x0-r,y0);
        m_path.curve4(x0-r,y0+c,  x0-c,y0+r,  x0,y0+r);
        //        agg::ellipse ellipse(x0,y0,radius,radius);
        //        m_path.concat_path(ellipse);
        m_path.end_path();
    }

    // parse_ellipse
    void
    parser::parse_ellipse(const char** attr)
    {
        int i;
        double cx = 0.0;
        double cy = 0.0;
        double rx = 0.0;
        double ry = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2) {
            if (!parse_attr(attr[i], attr[i + 1])) {
                if(strcmp(attr[i], "cx") == 0)    cx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "cy") == 0)    cy = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "rx") == 0)    rx = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "ry") == 0)    ry = parse_double(attr[i + 1]);
            }
        }


        if (rx != 0.0 && ry != 0.0) {
            if (rx < 0.0) throw exception("parse_ellipse: Invalid x-radius: %f", rx);
            if (ry < 0.0) throw exception("parse_ellipse: Invalid y-radius: %f", ry);

            m_path.move_to(cx, cy - ry);
            m_path.curve4(cx + rx * 0.56, cy - ry,
                cx + rx, cy - ry * 0.56,
                cx + rx, cy);
            m_path.curve4(cx + rx, cy + ry * 0.56,
                cx + rx * 0.56, cy + ry,
                cx, cy + ry);
            m_path.curve4(cx - rx * 0.56, cy + ry,
                cx - rx, cy + ry * 0.56,
                cx - rx, cy);
            m_path.curve4(cx - rx, cy - ry * 0.56,
                cx - rx * 0.56, cy - ry,
                cx, cy - ry);
            m_path.close_subpath();
        }
        m_path.end_path();
    }

    //-------------------------------------------------------------
    void parser::parse_line(const char** attr)
    {
        int i;
        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = 0.0;
        double y2 = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "x1") == 0) x1 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y1") == 0) y1 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "x2") == 0) x2 = parse_double(attr[i + 1]);
                if(strcmp(attr[i], "y2") == 0) y2 = parse_double(attr[i + 1]);
            }
        }

        m_path.move_to(x1, y1);
        m_path.line_to(x2, y2);
        m_path.end_path();
    }


    //-------------------------------------------------------------
    void parser::parse_poly(const char** attr, bool close_flag)
    {
        int i;
        double x = 0.0;
        double y = 0.0;

        m_path.begin_path();
        for(i = 0; attr[i]; i += 2)
        {
            if(!parse_attr(attr[i], attr[i + 1]))
            {
                if(strcmp(attr[i], "points") == 0) 
                {
                    m_tokenizer.set_path_str(attr[i + 1]);
                    if(!m_tokenizer.next())
                    {
                        throw exception("parse_poly: Too few coordinates");
                    }
                    x = m_tokenizer.last_number();
                    if(!m_tokenizer.next())
                    {
                        throw exception("parse_poly: Too few coordinates");
                    }
                    y = m_tokenizer.last_number();
                    m_path.move_to(x, y);
                    while(m_tokenizer.next())
                    {
                        x = m_tokenizer.last_number();
                        if(!m_tokenizer.next())
                        {
                            throw exception("parse_poly: Odd number of coordinates");
                        }
                        y = m_tokenizer.last_number();
                        m_path.line_to(x, y);
                    }
                }
            }
        }
        if(close_flag) 
        {
            m_path.close_subpath();
        }
        m_path.end_path();
    }

    //-------------------------------------------------------------
    trans_affine parser::parse_transform(const char* str)
    {
        trans_affine transform;
        while(*str)
        {
            if(islower(*str))
            {
                if(strncmp(str, "matrix", 6) == 0)    str += parse_matrix(str,transform);    else 
                if(strncmp(str, "translate", 9) == 0) str += parse_translate(str,transform); else 
                if(strncmp(str, "rotate", 6) == 0)    str += parse_rotate(str,transform);    else 
                if(strncmp(str, "scale", 5) == 0)     str += parse_scale(str,transform);     else 
                if(strncmp(str, "skewX", 5) == 0)     str += parse_skew_x(str,transform);    else 
                if(strncmp(str, "skewY", 5) == 0)     str += parse_skew_y(str,transform);    else
                {
                    ++str;
                }
            }
            else
            {
                ++str;
            }
        }
        return transform;
    }

    // parse_gradient
    void
        parser::parse_gradient(const char** attr, bool radial)
    {
        // printf("parser::parse_gradient(%s)\n", attr[0]);

        m_path.start_gradient(radial);

        for (int32 i = 0; attr[i]; i += 2)
        {
            /* if(!parse_attr(attr[i], attr[i + 1]))
            {*/
            if (strcmp(attr[i], "id") == 0)
                m_path.current_gradient()->set_id(attr[i + 1]);
            else if(strcmp(attr[i], "gradientTransform") == 0) {
                m_path.current_gradient()->set_transformation(parse_transform(attr[i + 1]));
            } else
                m_path.current_gradient()->add_string(attr[i], attr[i + 1]);
             /*}*/
        }
    }

    // parse_gradient_stop
    void
        parser::parse_gradient_stop(const char** attr)
    {
        // printf("parser::parse_gradient_stop(%s)\n", attr[0]);

        double offset = 0.0;
        rgba8 color;
        for (int32 i = 0; attr[i]; i += 2) {
            if (strcmp(attr[i], "offset") == 0) {
                offset = parse_double(attr[i + 1]);
            } else
                if (strcmp(attr[i], "style") == 0) {
                    parse_style(attr[i + 1]);
                    // here we get a bit hacky, in order not to change too much code at once...
                    // historically, parse_style() was for parsing path attributes only, but
                    // it comes in handy here as well, and I added "stop-color" and "stop-opacity"
                    // to parse_name_value(). It remembers the color in "fGradientStopColor".
                    // The color will of course be broken if the "style" attribute did not contain
                    // any valid stuff.
                    color = m_gradient_stop_color;
                } else
                    if (strcmp(attr[i], "stop-color") == 0) {
                        color = parse_color(attr[i + 1], m_swap_red_blue);
                    } else
                        if (strcmp(attr[i], "stop-opacity") == 0) {
                            color.opacity(parse_double(attr[i + 1]));
                        }
        }

        // printf(" offset: %f, color: %d, %d, %d, %d\n", offset, color.r, color.g, color.b, color.a);

        if (gradient* gradient = m_path.current_gradient()) {
            gradient->add_stop(offset, color);
        } else {
            throw exception("parse_gradient_stop() outside of gradient tag!\n");
        }
    }

    //-------------------------------------------------------------
    static unsigned parse_transform_args(const char* str, 
                                         double* args, 
                                         unsigned max_na, 
                                         unsigned* na)
    {
        *na = 0;
        const char* ptr = str;
        while(*ptr && *ptr != '(') ++ptr;
        if(*ptr == 0)
        {
            throw exception("parse_transform_args: Invalid syntax");
        }
        const char* end = ptr;
        while(*end && *end != ')') ++end;
        if(*end == 0)
        {
            throw exception("parse_transform_args: Invalid syntax");
        }

        while(ptr < end)
        {
            if(is_numeric(*ptr))
            {
                if(*na >= max_na)
                {
                    throw exception("parse_transform_args: Too many arguments");
                }
                args[(*na)++] = atof(ptr);
                while(ptr < end && is_numeric(*ptr)) ++ptr;
            }
            else
            {
                ++ptr;
            }
        }
        return unsigned(end - str);
    }

    //-------------------------------------------------------------
    unsigned parser::parse_matrix(const char* str, trans_affine& transform)
    {
        double args[6];
        unsigned na = 0;
        unsigned len = parse_transform_args(str, args, 6, &na);
        if(na != 6)
        {
            throw exception("parse_matrix: Invalid number of arguments");
        }
        transform.premultiply(trans_affine(args[0], args[1], args[2], args[3], args[4], args[5]));
        return len;
    }

    //-------------------------------------------------------------
    unsigned parser::parse_translate(const char* str, trans_affine& transform)
    {
        double args[2];
        unsigned na = 0;
        unsigned len = parse_transform_args(str, args, 2, &na);
        if(na == 1) args[1] = 0.0;
        transform.premultiply(trans_affine_translation(args[0], args[1]));
        return len;
    }

    //-------------------------------------------------------------
    unsigned parser::parse_rotate(const char* str, trans_affine& transform)
    {
        double args[3];
        unsigned na = 0;
        unsigned len = parse_transform_args(str, args, 3, &na);
        if(na == 1) 
        {
            transform.premultiply(trans_affine_rotation(deg2rad(args[0])));
        }
        else if(na == 3)
        {
            trans_affine t = trans_affine_translation(-args[1], -args[2]);
            t *= trans_affine_rotation(deg2rad(args[0]));
            t *= trans_affine_translation(args[1], args[2]);
            transform.premultiply(t);
        }
        else
        {
            throw exception("parse_rotate: Invalid number of arguments");
        }
        return len;
    }

    //-------------------------------------------------------------
    unsigned parser::parse_scale(const char* str, trans_affine& transform)
    {
        double args[2];
        unsigned na = 0;
        unsigned len = parse_transform_args(str, args, 2, &na);
        if(na == 1) args[1] = args[0];
        transform.premultiply(trans_affine_scaling(args[0], args[1]));
        return len;
    }

    //-------------------------------------------------------------
    unsigned parser::parse_skew_x(const char* str, trans_affine& transform)
    {
        double arg;
        unsigned na = 0;
        unsigned len = parse_transform_args(str, &arg, 1, &na);
        transform.premultiply(trans_affine_skewing(deg2rad(arg), 0.0));
        return len;
    }

    //-------------------------------------------------------------
    unsigned parser::parse_skew_y(const char* str, trans_affine& transform)
    {
        double arg;
        unsigned na = 0;
        unsigned len = parse_transform_args(str, &arg, 1, &na);
        transform.premultiply(trans_affine_skewing(0.0, deg2rad(arg)));
        return len;
    }

}
}


