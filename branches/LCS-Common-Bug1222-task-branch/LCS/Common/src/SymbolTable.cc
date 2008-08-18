//#  SymbolTable.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/SymbolTable.h>
#include <Common/LofarLogger.h>
#include <cstdlib>

namespace LOFAR
{

#if defined(linux)
  static const char* bfdFile = "/proc/self/exe";
#elif defined(__CYGWIN__)
  static const char* bfdFile = 0;
#elif defined(sun)
  static const char* bfdFile = "/proc/self/object/a.out";
#endif


  SymbolTable::SymbolTable() :
    itsBfd(0),
    itsSymbols(0)
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    init() && read();
  }


  SymbolTable::~SymbolTable()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    cleanup();
  }


  SymbolTable& SymbolTable::instance()
  {
    static SymbolTable symTab;
    return symTab;
  }


  bool SymbolTable::init()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    bfd_init();
    if ((itsBfd = bfd_openr(bfdFile,0)) == 0) {
      cerr << __PRETTY_FUNCTION__ << ": " << flush;
      bfd_perror(bfdFile);
      return false;
    }
    if (!bfd_check_format(itsBfd, bfd_object)) {
      cerr << __PRETTY_FUNCTION__ << ": " << flush;
      bfd_perror(bfdFile);
      return false;
    }
    return true;
  }
 

  bool SymbolTable::read()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    if ((bfd_get_file_flags(itsBfd) & HAS_SYMS) == 0) {
      cerr << __PRETTY_FUNCTION__ << ": " << bfdFile 
           << ": no symbols" << endl;
      return true;
    }
    unsigned int size;
    long symcount;
    symcount = bfd_read_minisymbols(itsBfd, false, (void**) &itsSymbols, &size);
    if (symcount == 0) {
      symcount = bfd_read_minisymbols(itsBfd, true, (void**) &itsSymbols, &size);
    }
    if (symcount < 0) {
      cerr << __PRETTY_FUNCTION__ << ": bfd_read_minisymbols() failed" 
           << endl;
      return false;
    }
    return true;
  }


  void SymbolTable::cleanup()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    if (itsSymbols) {
      free(itsSymbols);
      itsSymbols = 0;
    }
    if (itsBfd) {
      bfd_close(itsBfd);
      itsBfd = 0;
    }
  }

} // namespace LOFAR
