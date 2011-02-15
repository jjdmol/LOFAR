//# SymbolTable.h: Class holding the symbol table of an object file.
//#
//# Copyright (C) 2002-2011
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
please install the GNU binutils development package.
#else
# include <bfd.h>

namespace LOFAR
{

  // \addtogroup Common
  // @{

  // Class holding the symbol table of an object file. 
  //
  // \note The code is based on the utility addr2line.c in GNU binutils 2.16.
  class SymbolTable {

  public:
    // Default constructor. Doesn't read any symbol table into memory.
    SymbolTable();

    // Read the symbol table from \a filename into memory.
    SymbolTable(const char* filename);

    // Destructor.
    ~SymbolTable();

    // Return a pointer to the symbol table.
    asymbol** getSyms() const
    { return itsSymbols; }

    // Return a pointer to the bfd.
    bfd* getBfd() const
    { return itsBfd; }

  private:
    // Disallow copy constructor.
    SymbolTable(const SymbolTable&);

    // Disallow assignment operator.
    SymbolTable& operator=(const SymbolTable&);

    // Open the object file \c filename and check its format.
    // @return True on success.
    bool init(const char* filename);

    // Read the symbol table from the object file.
    // @return True when read was successful.
    bool read();

    // Free dynamically allocated memory; close object file.
    void cleanup();

    // Pointer to the binary file descriptor.
    bfd* itsBfd;

    // Pointer to the internal representation of the symbol table.
    asymbol** itsSymbols;
  };

  // @}

} // namespace LOFAR

#endif /* HAVE_BFD */

#endif
