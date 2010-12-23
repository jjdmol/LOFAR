//# AddressTranslator.h: Translate return addresses to function, file, line.
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

#ifndef LOFAR_COMMON_ADDRESSTRANSLATOR_H
#define LOFAR_COMMON_ADDRESSTRANSLATOR_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Translate return addresses to function, file, line.

//# Includes
#include <Common/Backtrace.h>
#include <vector>
#include <map>

#ifdef HAVE_BOOST
# include <boost/shared_ptr.hpp>
#endif

#ifndef HAVE_BFD
# warning Binary file descriptor (bfd) package is missing, \
please install the GNU binutils.
#else
# include <bfd.h>
# include <link.h>
# ifdef USE_THREADS
#  include <pthread.h>
# endif
#endif

namespace LOFAR
{
  // \ingroup Common
  // @{
  
  //# Forward declarations
  class SymbolTable;
  
  // Functor class for translating return addresses to function name,
  // filename, and line number. 
  //
  // \note The code is based on the utility addr2line.c in the GNU binutils
  // 2.16.
  // \note For details on handling of shared libraries, please refer to
  // - the Linux man page \c dl_iterate_phdr(3)
  // - \c util/backtrace-symbols.c in cairo (http://cairographics.org/)
  // - \c src/dwarf/Gfind_proc_info-lsb.c:callback() in libunwind
  //   (http://www.nongnu.org/libunwind/)

  class AddressTranslator {
  public:
    AddressTranslator();
    ~AddressTranslator();

    // Translate the \a size addresses specified in \a addr to \a size
    // trace lines, containing function name, filename, and line number, iff
    // that information can be retrieved from the program's symbol table.
    // \param[out] trace vector containing the trace lines
    // \param[in]  addr C-array of return addresses
    // \param[in]  size number of return addresses in \a addr
    void operator()(std::vector<Backtrace::TraceLine>& trace, 
                    void* const* addr, int size);

  private:
#ifdef HAVE_BFD
# ifdef USE_THREADS
    // Simple scoped lock class. Used to guard access to the BFD routines,
    // which are not thread-safe.
    class ScopedLock {
    public:
      ScopedLock() { pthread_mutex_lock(&mutex); }
      ~ScopedLock() { pthread_mutex_unlock(&mutex); }
    private:
      ScopedLock(const ScopedLock&);
      ScopedLock& operator=(const ScopedLock&);
      static pthread_mutex_t mutex;
    };
# endif
    // Helper function to "convert" the member function
    // do_find_addresss_in_section() to a void(*), which can be passed as
    // argument to bfd_map_over_sections().
    // \see BFD documentation for details (<tt>info bfd</tt>).
    static void find_address_in_section(bfd*, asection*, void*);

    // Look for an address in a section. If present, find source code filename,
    // line number and function name nearest to that address.
    void do_find_address_in_section(bfd*, asection*);

    // Helper function to "convert" the member function
    // do_find_matching_file() to a static callback function that can be
    // passed as argument to dl_iterate_phdr().
    // \see Refer to <tt>man dl_iterate_phdr</tt> for details.
    static int find_matching_file(dl_phdr_info*, size_t, void*);

    // Look for an address in one of the currently loaded shared objects.
    int do_find_matching_file(dl_phdr_info*);

    // Local variables used by do_find_matching_file() and
    // do_find_address_in_section().
    // @{
    const char* bfdFile;
    bfd_vma base_addr;
    bfd_vma pc;
    const char* filename;
    const char* functionname;
    unsigned int line;
    bool found;
    // @}

    // Map of symbol tables.
    // Use the base address of the shared object or executable as key.
    typedef std::map< bfd_vma, boost::shared_ptr<SymbolTable> > SymbolTableMap;

    // This map of symbol tables is shared between all AddressTranslator
    // objects. Use a lock to make access thread-safe.
    static SymbolTableMap symTabMap;
#endif
  };

  // @}

} // namespace LOFAR

#endif
