//  BBSTestLogger.cc: Writes BBSTest loglines to the right file
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////////

#include <Common/LofarLogger.h>
#include <BBSTestLogger.h>
#include <sstream>

namespace LOFAR{

int BBSTestLogger::theirRank = -1;

  BBSTestLogger::BBSTestLogger() {
    init();
  }

  BBSTestLogger::~BBSTestLogger() {
  }

  void BBSTestLogger::log(const string& name, NSTimer& timer)
  { 
    std::ostringstream ss;
    ss << "timer "<<name;
    timer.print(ss);
    doLog(ss.str());
  }
  void BBSTestLogger::log(const string& name, const MeqMatrix& mat)
  { 
    std::ostringstream ss;
    ss << "parm " << name << " " << mat;
    doLog(ss.str());
  }
  void BBSTestLogger::log(const string& text)
  { doLog(text);}
  void BBSTestLogger::doLog(const string& text)
  { 
    init();
    LOG_INFO_STR("BBSTest rank " << theirRank << ": " << text); 
  }

} // namespace LOFAR
