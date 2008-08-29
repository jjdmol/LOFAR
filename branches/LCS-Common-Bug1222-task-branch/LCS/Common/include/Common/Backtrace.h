//#  Backtrace.h: one line description
//#
//#  Copyright (C) 2002-2008
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

#ifndef LOFAR_COMMON_BACKTRACE_H
#define LOFAR_COMMON_BACKTRACE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// one line description.

//# Includes
#include <bfd.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  //# Forward declarations
  class SymbolTable;

  // \addtogroup Common
  // @{

  // Description of class.

  class Backtrace {
  public:
    // Layout of a trace line
    struct TraceLine {
      string function;
      string file;
      unsigned line;
    };
    // Maximum number of return addresses that we are willing to handle.
    static const unsigned maxNrAddr = 50;

    // Constructor. Calls get_addresses() to fill \c itsAddr with the return
    // addresses of the current program state.
    Backtrace();

    ~Backtrace();

    void print(ostream& os) const;

  private:
//     // disallow copying and assingment
//     Backtrace(const Backtrace&);
//     Backtrace& operator=(const Backtrace&);

//     // Initialize our data structures by zeroing them.
//     void init();

//     // Get the return addresses of the current program state, and store them
//     // in \c itsAddr.
//     // \return The number of return addresses.
//     int get_addresses();

    // Translate each return addresses into function name, filename and line
    // number. Uses helper class AddressTranslator to do the real work.
    bool translate_addresses();


    // C-array of return addresses. It will be filled by get_addresses().
    void* itsAddr[maxNrAddr];

    // Actual number of return addresses returned by get_addresses().
    int itsNrAddr;

    // Traceback info containing function name, filename, and line number.
    // This vector will be filled by AddressTranslator.translate().
    mutable vector<TraceLine> itsTrace;

  };

  ostream& operator<<(ostream& os, const Backtrace& st);

  // @}

} // namespace LOFAR

#endif
