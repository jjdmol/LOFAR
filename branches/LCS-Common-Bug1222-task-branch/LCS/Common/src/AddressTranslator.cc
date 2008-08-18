//#  AddressTranslator.cc: one line description
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
#include <Common/AddressTranslator.h>
#include <Common/SymbolTable.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <demangle.h>

namespace LOFAR
{

  AddressTranslator::AddressTranslator()
  {
  }

  AddressTranslator::~AddressTranslator()
  {
  }


  TraceLine AddressTranslator::translate(size_t addr)
  {
  }


  void AddressTranslator::find_address_in_section(bfd*      abfd,
                                          asection* section,
                                          void*     data)
  {
    AddressTranslator* obj = static_cast<AddressTranslator*>(data);
    obj->do_find_address_in_section(abfd, section);
  }

  void AddressTranslator::do_find_address_in_section(bfd*       abfd, 
                                             asection*  section)
  {
    bfd_vma vma;
    bfd_size_type size;
      
    if (found)
      return;
      
    if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
      return;
      
    vma = bfd_get_section_vma (abfd, section);
    if (pc < vma)
      return;
      
    size = bfd_get_section_size (section);
    if (pc >= vma + size)
      return;
      
    found = bfd_find_nearest_line (abfd, section, 
                                   SymbolTable::instance().getSyms(), 
                                   pc - vma, &filename, &functionname, &line);
  }
    


  bool AddressTranslator::translate_addresses()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  
    for (unsigned i = 0; i < addr.size(); i++) {
      ostringstream oss;
      oss << hex << addr[i];
      pc = bfd_scan_vma(oss.str().c_str(), 0, 16);

      found = false;
      bfd* abfd = SymbolTable::instance().getBfd();
      bfd_map_over_sections(abfd, find_address_in_section, this);
    
      //     for (asection* p = abfd->sections; p != 0; p = p->next) {
      //       find_address_in_section(abfd, p, 0);
      //     }
    
      TraceLine aTraceLine;
      aTraceLine.address = addr[i];
    
      if (!found) {
        aTraceLine.function = "??";
        aTraceLine.file = "??";
        aTraceLine.line = 0;
      } 
      else {
        if (functionname == 0 || *functionname == '\0') {
          aTraceLine.function = "??";
        } 
        else {
          char* res = cplus_demangle(functionname, DMGL_ANSI | DMGL_PARAMS);
          if (res == 0) {
            aTraceLine.function = functionname;
          }
          else {
            aTraceLine.function = res;
            free(res);
          }
        }
        if (filename == 0) {
          aTraceLine.file = "??";
        }
        else {
          char* h = strrchr(filename,'/');
          if (h != 0)
            filename = h+1;
          aTraceLine.file = filename;
        }
        aTraceLine.line = line;
      }
      itsTrace.push_back(aTraceLine);
      //     if (aTraceLine.function == "main")
      //       break;
    }
    //  printf("level = %d\n",level);
    //   return level;
    return (level != 0);
  }

} // namespace LOFAR
