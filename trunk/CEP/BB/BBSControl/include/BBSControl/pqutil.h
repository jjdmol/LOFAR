//#  pqutil.h: PostgreSQL related utilities; for internal use only.
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_BBSCONTROL_PQUTIL_H
#define LOFAR_BBSCONTROL_PQUTIL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// PostgreSQL related utilities; for internal use only.

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{
      
    // \internal
    // Convert a Postgres-generated C-string to a ParameterSet string. For
    // vectors, this means replacing '{' with '[' and '}' with ']'.
    // @{
    string toPSstring(const char* in);
    string toPSstring(const string& in);
    // @}

    // \internal
    // Convert a ParameterSet string to a Postgres string:
    // - single quotes will be escaped by doubling them;
    // - curly braces and double quotes will be escaped using a backslash.
    // - vector delimiters \c [ and \c ] will be replaced with \c { and \c }.
    // @{
    string toPQstring(const string& in);
    string toPQstring(const vector<string>& in);
    // @}

    // @}

    //##----   I n l i n e   m e t h o d s   ----##//

    inline string toPSstring(const string& in)
    {
      return toPSstring(in.c_str());
    }

  } // namespace BBS
  
} // namespace LOFAR

#endif
