//# SymbolTable.h: one line description
//#
//# Copyright (C) 2002-2008
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

#ifndef LOFAR_COMMON_SYMBOLTABLE_H
#define LOFAR_COMMON_SYMBOLTABLE_H

// \file
// one line description.

//# Includes
#ifndef HAVE_BFD
# warning Binary file descriptor (bfd) package is missing, \
please install the GNU binutils.
#else
# include <bfd.h>
# include <map>
# include <boost/shared_ptr.hpp>

namespace LOFAR
{

  // \addtogroup Common
  // @{

  // Class holding the symbol table of the current executable program. It is
  // implemented as a Singleton. So, any overhead will only be paid when this
  // class is instantiated, which will normally happen when the first
  // Backtrace is printed (\e not when the first exception is thrown).
  //
  // \note The code is based on the utility addr2line.c in GNU binutils 2.16.
  class SymbolTable {

  public:
    // Default constructor. Doesn't read any symbol table into memory.
    SymbolTable();

    // Read the symbol table from \c filename into memory.
    SymbolTable(const char* filename);

    // Destructor.
    ~SymbolTable();

    // Return a pointer to the symbol table
    asymbol** getSyms() const
    { return itsSymbols; }

    // Return a pointer to the bfd
    bfd* getBfd() const
    { return itsBfd; }

  private:
    // Disallow copy constructor.
    SymbolTable(const SymbolTable&);

    // Disallow assignment operator.
    SymbolTable& operator=(const SymbolTable&);

    // Open the bfd-file \c filename and check its format.
    // @return True on success.
    bool init(const char* filename);

    // Read the symbol table from the bfd-file.
    // @return True when read was successful.
    bool read();

    // Free dynamically allocated memory; close bfd-file.
    void cleanup();

    // Pointer to the bfd
    bfd* itsBfd;

    // Pointer to the symbol table.
    asymbol** itsSymbols;
  };


  // Map of symbol tables.
  // Use the load address of the shared object or executable as key.
  typedef std::map< bfd_vma, boost::shared_ptr<SymbolTable> > SymbolTableMap;

  // Return the map of symbol tables.
  // This method is implemented as a Meyers singleton. Use a guard lock to
  // make access thread-safe.
  SymbolTableMap& theSymbolTableMap();

  // @}

} // namespace LOFAR

#endif /* HAVE_BFD */

#endif
