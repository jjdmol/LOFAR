//#  KeyPrefix.h: Handle prefixing of ParameterSet keys.
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

#ifndef LOFAR_APS_KEYPREFIX_H
#define LOFAR_APS_KEYPREFIX_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Handle prefixing of ParameterSet keys.

//# Includes
#include <APS/Exceptions.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace APS 
    {
      // \addtogroup APS
      // @{

      // Handle prefixing of ParameterSet keys. Whenever a KeyPrefix object is
      // constructed, a field, consisting of \a str + <tt>"."</tt>, is
      // appended to the static member \c theirPrefix; and whenever a
      // KeyPrefix object goes out of scope the last field is stripped off of
      // \c theirPrefix.
      class KeyPrefix
      {
      public:
        // Constructor. Add a field \a str + <tt>"."</tt> to the static key
        // prefix.
        KeyPrefix(const string& str);

        // Destructor. Remove the last field from the static key prefix.
        ~KeyPrefix();

        // Return the current key prefix.
        static const string& get() { return theirPrefix; }

      private:
        // Print the current key prefix into the output stream \a os.
        friend ostream& operator<<(ostream& os, const KeyPrefix& kp);

        // Static key prefix. Keeps track of the current key prefix.
        static string theirPrefix;
      };

      // @}

    } // namespace APS

  }  // namespace ACC

} // namespace LOFAR

#endif
