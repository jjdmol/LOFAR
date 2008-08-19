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
#include <Common/AddressTranslator.h>
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

  Backtrace::Backtrace() :
    itsNrAddr(0)
  {
    memset(itsAddr, 0, maxNrAddr*sizeof(void*));
    itsNrAddr = backtrace(itsAddr, maxNrAddr);
  }

  Backtrace::~Backtrace()
  {
  }


  void Backtrace::init()
  {
  }


  int Backtrace::get_addresses()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    return itsNrAddr;
  }

  bool Backtrace::translate_addresses()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    if (itsTrace.empty()) {
      AddressTranslator()(itsTrace, itsAddr, itsNrAddr);
    }
  }

  void Backtrace::print(ostream& os) const
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
#if 0
    for (int i = 0; i < itsNrAddr; ++i) {
      printf("%-16p\n", itsAddr[i]);
    }
#endif
    if (itsTrace.empty()) {
      AddressTranslator()(itsTrace, itsAddr, itsNrAddr);
    }
      
//     translate_addresses();

    // Save the current fmtflags
    std::ios::fmtflags flags(os.flags());

    os.setf(std::ios::showbase);
    for(int i = 0; i < itsNrAddr; ++i) {
      os << "#" << left << setw(4) << i;
// 	 << ">>>";
//       os.setf(std::ios::internal, std::ios::adjustfield);
      os << right << setw(2+2*sizeof(void*)) << itsAddr[i]
//       os << "<<<";
	 << " in " << itsTrace[i].function
	 << " at " << itsTrace[i].file
	 << ":"    << itsTrace[i].line << endl;
    }

    // Restore the fmtflags
    os.flags(flags);
  }


  ostream& operator<<(ostream& os, const Backtrace& st)
  {
    st.print(os);
    return os;
  }

} // namespace LOFAR
