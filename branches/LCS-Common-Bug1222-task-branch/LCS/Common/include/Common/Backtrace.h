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
    // Maximum number of levels backtrace will return
    static const unsigned maxStackAddr = 50;
    Backtrace();
    ~Backtrace();
    friend std::ostream& operator<<( std::ostream& os, const Backtrace& st );
  private:
    // disallow copying and assingment
    Backtrace(const Backtrace&);
    Backtrace& operator=(const Backtrace&);
    // Layout of a trace line
    struct TraceLine {
      long address;
      std::string function;
      std::string file;
      long line;
    };
    // private member functions
    bool get_addresses();
    bool translate_addresses();
    static void find_address_in_section(bfd*, asection*, void*);
    void do_find_address_in_section(bfd*, asection*);

    // private member data
    unsigned long level;
    std::vector<unsigned long> addr;
    std::vector<TraceLine> itsTrace;

    // Local variables used by translate_addresses() and find_address_in_section()
    bfd_vma pc;
    const char* filename;
    const char* functionname;
    unsigned int line;
    bool found;

  };

  // @}

} // namespace LOFAR

#endif
