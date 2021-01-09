#!/usr/bin/python
######################################################################
# Fixed erroneous gob2 output.
######################################################################

import sys
import re

with open(sys.argv[1]) as inp:
    with open(sys.argv[2],'w') as out:
        in_area = 0
        for line in inp.readlines():
            if in_area:
                line = re.sub('G_TYPE_OBJECT',
                              'GTK_TYPE_ADJUSTMENT',
                              line)
                if re.search(r'\);', line):
                    in_area=0
            else:
                if re.search(r"object_signals\[SET_SCROLL_ADJUSTMENTS_SIGNAL\]",line):
                    in_area=1
    
            line = re.sub('GtkScrollableInterfaceIface',
                          'GtkScrollableInterface',
                          line)
            line = re.sub('GTK_TYPE_SCROLLABLEINTERFACE',
                          'GTK_TYPE_SCROLLABLE',
                          line)
            line = re.sub('scroll_policy"',
                          'scroll-policy"',
                          line)

            out.write(line)
            
    
