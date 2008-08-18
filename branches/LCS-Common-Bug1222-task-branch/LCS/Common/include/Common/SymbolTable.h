//#  SymbolTable.h: one line description
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

#ifndef LOFAR_COMMON_SYMBOLTABLE_H
#define LOFAR_COMMON_SYMBOLTABLE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// one line description.

//# Includes
#include <bfd.h>

namespace LOFAR
{

  // \addtogroup Common
  // @{
  //# Forward Declarations
  // Description of class.
  class SymbolTable {

  public:
    // Destructor.
    ~SymbolTable();

    // Return an instance of SymbolTable. When called for the first time
    // an instance of SymbolTable is created. Subsequent calls return the
    // previously created instance.
    static SymbolTable& instance();

    // Return a pointer to the symbol table
    asymbol** getSyms() const
    { return itsSymbols; }

    // Return a pointer to the bfd
    bfd* getBfd() const
    { return itsBfd; }

  private:
    // Default constructor is called from the static member function
    // SymbolTable::instance().
    SymbolTable();

    // Disallow copy constructor.
    SymbolTable(const SymbolTable&);

    // Disallow assignment operator.
    SymbolTable& operator=(const SymbolTable&);

    // Open the bfd-file and check its format.
    // @return True on success.
    bool init();

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

  // @}

} // namespace LOFAR

#endif
