//# StreamUtil.h: useful stream manipulation methods.
//#
//# Copyright (C) 2002-2003
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_STREAMUTIL_H
#define LOFAR_COMMON_STREAMUTIL_H

// \file
// Useful stream manipulation methods.

//# Includes
#include <utility>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  // Handles indentation of text lines. Every time an Indent object is
  // constructed, the static member \c level is incremented by one, and every
  // time an Indent object is destructed \c level is decremented by one. To
  // increment the amount of indentation you simply create an Indent
  // object. When this object goes out of scope, the amount of indentation is
  // automagically decremented.
  class Indent {
  public:
    // Constructor. Increments indentation level.
    Indent() { lvl++; }
    // Destructor. Decrements indentation level.
    ~Indent() { lvl--; }
    // Return the amount of indentation.
    static uint level() { return lvl; }
    // Return the token to be printed per indentation level
    static const string& token() { return tok; }
  private:
    // Indentation level.
    static uint lvl;
    // Token to be printed per indentation level.
    static const string tok;
  };

  // Print an indentation that depends on the number of Indent objects
  // currently in existence.
  inline ostream& indent(ostream& os) 
  {
    for (uint i = 0; i < Indent::level(); ++i) {
      os << Indent::token();
    }
    return os;
  }


  // Declare the functions.
  // Write a std::pair.
  template <typename T, typename U>
  ostream& operator<< (ostream& os, const std::pair<T,U>& p);
  // Write any container to the given ostream.
  template<typename ITER>
  void print (ostream& os, ITER begin, ITER end, 
              const char* separator=",",
              const char* prefix="[", const char* postfix="]");
  // Write a vector to an ostream with a given separator, prefix and postfix.
  template<class T>
  void writeVector (std::ostream& os, const std::vector<T>& vec,
		    const char* separator=",",
		    const char* prefix="[", const char* postfix="]");
  // Print the contents of a vector enclosed in square brackets, using a comma
  // as separator.
  // \note operator<<() must be defined for type \c T.
  template<typename T>
  ostream& operator<<(ostream& os, const vector<T>& v);
  // Print the contents of a map enclosed in braces, using a comma
  // as separator.
  // \note operator<<() must be defined for type \c T.
  template<typename T, typename U>
  ostream& operator<<(ostream& os, const map<T,U>& m);


  // Write a std::pair.
  template <typename T, typename U>
  inline ostream& operator<< (ostream& os, const std::pair<T,U>& p)
  {
    os << '<' << p.first << ',' << p.second << '>';
    return os;
  }

  // Write any container to the given ostream.
  template<typename ITER>
  inline void print (ostream& os, ITER begin, ITER end, 
                     const char* separator,
                     const char* prefix, const char* postfix)
  {
    os << prefix;
    if (begin != end) {
      os << *begin;
      ++begin;
    }
    for (; begin!=end; ++begin) {
      os << separator << *begin;
    }
    os << postfix;
  }


  // Write a vector to an ostream with a given separator, prefix and postfix.
  template<class T>
  inline void writeVector (std::ostream& os, const std::vector<T>& vec,
                           const char* separator,
                           const char* prefix, const char* postfix)
  {
    print (os, vec.begin(), vec.end(), separator, prefix, postfix);
  }


  // Print the contents of a vector enclosed in square brackets, using a comma
  // as separator.
  // \note operator<<() must be defined for type \c T.
  template<typename T>
  inline ostream& operator<<(ostream& os, const vector<T>& v)
  {
    writeVector(os, v, ",", "[", "]");
    return os;
  }


  // Print the contents of a map enclosed in braces, using a comma
  // as separator.
  // \note operator<<() must be defined for type \c T.
  template<typename T, typename U>
  inline ostream& operator<<(ostream& os, const map<T,U>& m)
  {
    print (os, m.begin(), m.end(), ", ", "{", "}");
    return os;
  }


} // namespace LOFAR

#endif
