//#  Backtrace.cc: one line description
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
#include <Common/Backtrace.h>
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

  // FUDGE defines the number of longs that is supposed to be present in a
  // stack frame before the pointer to the next stack frame appears.
#if defined(linux)
  static const unsigned int FUDGE = 0;
#elif defined(__CYGWIN__)
  static const unsigned int FUDGE = 0;
#elif defined(sun)
  static const unsigned int FUDGE = 14;
#else
#error "ERROR: Backtrace is not supported for this OS"
#endif
  
  // This implementation assumes a stack layout that matches the defaults
  // used by gcc's `__builtin_frame_address' and `__builtin_return_address'
  // (FP is the frame pointer register):
  //
  //          +-----------------+     +-----------------+
  //    FP -> | previous FP --------> | previous FP ------>...
  //          |                 |     |                 |
  //          | return address  |     | return address  |
  //          +-----------------+     +-----------------+
  //
  //
  struct Frame
  {
    long fudge[FUDGE];
    Frame* next;
    void* return_address;
  };
  

  Backtrace::Backtrace()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    get_addresses() && translate_addresses();
  }

  Backtrace::~Backtrace()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  }


#if 1
  bool Backtrace::get_addresses()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    // This is a global variable set at program start time.
    // It marks the highest used stack address.
    extern void *__libc_stack_end;

    // === A nifty trick borrowed from backtrace.c in glibc-2.2.x ===
    // Get some notion of the current stack.  Need not be exactly the top
    // of the stack, just something somewhere in the current frame.
    void* top_stack = ({ char __csf; &__csf; });

    // Initialize ebp to the next frame address; we skip the call to
    // this function, it makes no sense to record it.
    Frame* ebp = static_cast<Frame*>(__builtin_frame_address(1));

    // Check if the address of the frame pointer is in range.
    // Here we assume that the stack grows downward.
    while (static_cast<void*>(ebp) >= top_stack  
#if defined(linux) && defined(__i386__)
           // __libc_stack_end seems to be defined only in 32-bit libc.a on linux.
           && static_cast<void*>(ebp) < __libc_stack_end
#endif
           ) 
    {
      // Add the return address to the vector addr
      addr.push_back(reinterpret_cast<unsigned long>(ebp->return_address));

      // Advance ebp to the next stack frame.
      ebp = ebp->next;
    }

    return (!addr.empty());
  }

#else

  /* This is just debugging/development stuff to see how backtrace() works */
  bool Backtrace::get_addresses()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    void* array[maxStackAddr];
    int len = backtrace(array, maxStackAddr);
  
    for (int i=0; i<len; ++i) {
      addr.push_back(reinterpret_cast<unsigned long>(array[i]));
    }
    return (!addr.empty());
  }

#endif

  void Backtrace::find_address_in_section(bfd*      abfd,
                                          asection* section,
                                          void*     data)
  {
    Backtrace* obj = static_cast<Backtrace*>(data);
    obj->do_find_address_in_section(abfd, section);
  }

  void Backtrace::do_find_address_in_section(bfd*       abfd, 
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
    


  bool Backtrace::translate_addresses()
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

  ostream& operator<<( ostream& os, const Backtrace& st )
  {
    // Save the current fmtflags
    std::ios::fmtflags flags(os.flags());

    for(unsigned i = 0; i < st.itsTrace.size(); ++i) {
      Backtrace::TraceLine aLine = st.itsTrace[i];
      os.setf(std::ios::showbase);
      os.setf(std::ios::left, std::ios::adjustfield);
      os << "#" << setw(3) << i;
      os.setf(std::ios::internal, std::ios::adjustfield);
      os << hex << setfill('0') << setw(18) << aLine.address 
         << dec << setfill(' ');
      os << " in " << aLine.function;
      os << " at " << aLine.file;
      os << ":"    << aLine.line << endl;
    }

    // Restore the fmtflags
    os.flags(flags);

    return os;
  }

} // namespace LOFAR
