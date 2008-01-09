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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <BBSTestLogger.h>
#include <BBS3/ParmData.h>
#include <sstream>

namespace LOFAR{

  namespace BBSTest {

    bool Logger::theirIsInitted = false;
    int Logger::theirRank = -1;

    Logger::Logger() {
      init();
    }

    Logger::~Logger() {
    }

    void Logger::log(NSTimer& timer)
    { 
      std::ostringstream ss;
      ss << "Timer ";
      timer.print(ss);
      doLog(ss.str());
    }
    void Logger::log(ScopedTimer& timer)
    { 
      std::ostringstream ss;
      ss << "Timer ";
      timer.print(ss);
      doLog(ss.str());
    }
    void Logger::log(ScopedUSRTimer& timer)
    { 
      std::ostringstream ss;
      ss << "Timer "<<timer.getName();
      timer.show(ss);
      doLog(ss.str());
    }
    void Logger::log(const string& name, const vector<ParmData>& parms)
    { 
      vector<ParmData>::const_iterator it = parms.begin();
      for (; it != parms.end(); it++) {
	log(name, *it);
      }
    }
    void Logger::log(const string& name, const ParmData& parm)
    { 
      std::ostringstream ss;
      ss << "parm " << name << " " <<parm.getName() << " " << parm.getValues();
      doLog(ss.str());
    }
    void Logger::log(const ParmData& parm)
    { 
      log("", parm);
    }
    void Logger::log(const string& text)
    { doLog(text);}
    void Logger::doLog(const string& text)
    { 
      if (theirIsInitted) {
#if 0
	LOG_INFO_STR("BBSTest rank " << theirRank << ": " << text); 
#else
	std::cout<<"BBSTest rank " << theirRank << ": " << text<< std::endl;
#endif
      }
    }
  }
} // namespace LOFAR
