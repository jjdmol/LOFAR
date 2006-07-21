//#  StreamFormatting.h: useful stream formatting methods.
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBSCONTROL_STREAMFORMATTING_H
#define LOFAR_BBSCONTROL_STREAMFORMATTING_H

// \file
// Useful stream formatting methods.

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace BBS
  {
    // Handles indentation of text lines. Every time an Indent object is
    // constructed, the static member \c level is incremented by one, and
    // every time an Indent object is destructed \c level is decremented by
    // one. To increment the amount of indentation you simply create an Indent
    // object. When this object goes out of scope, the amount of indentation
    // is automagically decremented.
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
      for (uint i = 0; i < Indent::level(); ++i) os << Indent::token();
      return os;
    }


    // Print the contents of a vector of \c T. 
    // \note operator<<() must be defined for type \c T.
    template<typename T>
    inline ostream& operator<<(ostream& os, const vector<T>& v)
    {
      os << "[";
      for (uint i = 0; i < v.size(); ++i) os << " " << v[i];
      os << " ]";
      return os;
    }

  } // namespace BBS

} // namespace LOFAR

#endif
