//# AddressTranslator.cc:  Translate return addresses to function, file, line.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/AddressTranslator.h>
#include <Common/Backtrace.h>
#include <Common/SymbolTable.h>
#include <map>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_BFD
# ifdef __GNUG__
#  include <cxxabi.h>
# endif
#endif

namespace LOFAR
{

#ifdef HAVE_BFD
  namespace
  {
    // Map of symbol tables.
    // Use the load address of the shared object or executable as key.
    typedef std::map< bfd_vma, boost::shared_ptr<SymbolTable> > SymbolTableMap;

    // The map of symbol tables is implemented as a Meyers singleton.
    // Use a lock to make access thread-safe.
    SymbolTableMap& theSymbolTableMap()
    {
      static SymbolTableMap symTabMap;
      return symTabMap;
    }
  }
#endif

  // Initialize to true, so that backtrace printing stops at main() function.
  bool AddressTranslator::stopAtMain = true;

  AddressTranslator::AddressTranslator()
  {
  }

  AddressTranslator::~AddressTranslator()
  {
  }

  void AddressTranslator::operator()(std::ostream& os, void* const* addr, 
                                     unsigned int size)
  {
    ScopedLock sc(mutex);
    unsigned int lineCount = 0;

    // Save the current fmtflags
    std::ios::fmtflags flags(os.flags());
    os.setf(std::ios::left);

    for (unsigned int i = 0; i < size; i++) {

      found = false;
#ifdef HAVE_BFD
      pc = reinterpret_cast<bfd_vma>(addr[i]);

      // Walk through list of shared objects (ref. man dl_iterate_phdr(3)).
      // The callback function #find_matching_file() will set #bfdFile and
      // #base_addr.
      dl_iterate_phdr(find_matching_file, this);

      // Lookup the SymbolTable using #base_addr as key. Create a new entry
      // if the symbol table of #bfdFile is not yet present in the map.
      SymbolTableMap::iterator it = theSymbolTableMap().find(base_addr);
      if(it == theSymbolTableMap().end()) {
        it = theSymbolTableMap().insert
          (std::make_pair(base_addr, new SymbolTable(bfdFile))).first;
      }

      // Get the BFD-handle of the matching SymbolTable object.
      bfd* abfd = it->second->getBfd();
      if (!abfd) continue;

      // Get the handle to the symbols of the matching SymbolTable.
      syms = it->second->getSyms();
      if (!syms) continue;

      // Calculate offset address inside the matching shared object.
      pc -= base_addr;

      // Call the function #find_address_in_section() for each section
      // attached to the BFD \a abfd. This function will set #filename,
      // #functionname, #line and #found.
      bfd_map_over_sections(abfd, find_address_in_section, this);

      // printTraceLine will return true if hits \c main() \e and #stopAtMain
      // is \c true.
      if (printTraceLine(os, lineCount++, addr[i])) break;

      // Unwind inlined functions.
      while(bfd_find_inliner_info(abfd, &filename, &functionname, &line)) {
        printTraceLine(os, lineCount++);
      }
#endif
    }
    // Restore the fmtflags
    os.flags(flags);
    return;
  }

  //##----  P r i v a t e   f u n c t i o n s  ----##//

#ifdef HAVE_BFD
  Mutex AddressTranslator::mutex;

  int AddressTranslator::find_matching_file(dl_phdr_info* info,
                                            size_t      /*size*/,
                                            void*         data)
  {
    AddressTranslator* obj = static_cast<AddressTranslator*>(data);
    return obj->do_find_matching_file(info);
  }

  int AddressTranslator::do_find_matching_file(dl_phdr_info* info)
  {
    // This code is based on util/backtrace-symbols.c from Cairo
    // (http://cairographics.org).
    const ElfW(Phdr) *phdr = info->dlpi_phdr;
    for (int n = info->dlpi_phnum; --n >= 0; phdr++) {
      if (phdr->p_type == PT_LOAD) {
        ElfW(Addr) vaddr = phdr->p_vaddr + info->dlpi_addr;
        if (pc >= vaddr && pc < vaddr + phdr->p_memsz) {
          // We found a match
          if (info->dlpi_name && *info->dlpi_name) {
            bfdFile = info->dlpi_name;
          } else {
            bfdFile = "/proc/self/exe";
          }
          base_addr = info->dlpi_addr;
          return 1;
        }
      }
    }
    return 0;
  }
    

  void AddressTranslator::find_address_in_section(bfd* abfd,
                                                  asection* section,
                                                  void* data)
  {
    AddressTranslator* obj = static_cast<AddressTranslator*>(data);
    obj->do_find_address_in_section(abfd, section);
  }

  void AddressTranslator::do_find_address_in_section(bfd* abfd,
                                                     asection*  section)
  {
    if (found)
      return;

    if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
      return;

    bfd_vma vma = bfd_get_section_vma (abfd, section);
    if (pc < vma)
      return;

    bfd_size_type size = bfd_get_section_size (section);
    if (pc >= vma + size)
      return;

    found = bfd_find_nearest_line (abfd, section, syms, pc - vma,
                                   &filename, &functionname, &line);
  }


  bool AddressTranslator::printTraceLine(std::ostream& os,
                                         unsigned int count, 
                                         void* const addr)
  {
    if (count > 0)
      os << std::endl;
    os << "#" << std::setw(2) << count;
    if (addr)
      os << " " << addr;
    std::string function = demangle(functionname);
    os << " in " << function;
    if (filename && line)
      os << " at " << filename << ":" << line;
    else
      os << " from " << bfdFile;
    return stopAtMain && function == "main";
  }

  std::string AddressTranslator::demangle(const char* name)
  {
    std::string demangled("??");
    if (name && *name) {
# ifdef __GNUG__
      char* dmgl = abi::__cxa_demangle(name, 0, 0, 0);
      if (dmgl) {
        demangled = dmgl;
        free(dmgl);
      } else {
        demangled = name;
      }
# else
      demangled = name;
# endif
    }
    return demangled;
  }

#endif

} // namespace LOFAR
