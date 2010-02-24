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

#ifndef HAVE_BFD
# warning Binary file descriptor (bfd) package is missing, \
please install the GNU binutils.
#else
# include <bfd.h>
# ifdef USE_THREADS
#  include <pthread.h>
# endif
#endif

namespace LOFAR
{
  // \ingroup Common
  // @{

  // Functor class for translating return addresses to function name,
  // filename, and line number. 
  //
  // \note The code is based on the utility addr2line.c in the GNU binutils
  // 2.16.
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

    // Find the source code line nearest to 
    void do_find_address_in_section(bfd*, asection*);

    // Local variables used by translate_addresses() and
    // do_find_address_in_section().
    // @{
    bfd_vma pc;
    const char* filename;
    const char* functionname;
    unsigned int line;
    bool found;
    // @}
#endif
  };

  // @}

} // namespace LOFAR

#endif
