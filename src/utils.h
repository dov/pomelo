//======================================================================
//  utils.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Feb 25 08:02:25 2023
//----------------------------------------------------------------------
#ifndef UTILS_H
#define UTILS_H

#include <string>

void string_to_file(const std::string& text,
                    const std::string& filename);

std::string load_string_from_file(const std::string& filename);

#endif /* UTILS */
