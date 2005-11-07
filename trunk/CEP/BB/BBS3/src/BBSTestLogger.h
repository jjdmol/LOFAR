//  BBSTestLogger.h: Writes BBSTest loglines to the right file
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

#ifndef LOFAR_BBS3_BBSTESTLOGGER_H
#define LOFAR_BBS3_BBSTESTLOGGER_H

// \file BBSTestLogger.h
// This is a class that writes BBSTest loglines to the correct file

#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Transport/TH_MPI.h>
#include <fstream>
#include <BBS3/ParmData.h>
#include <Common/Timer.h>
#include <casa/OS/Timer.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

  namespace BBSTest 
  {

    class ScopedUSRTimer;
    class ScopedTimer;

    class Logger
    {
    public:
      // init the logger
      // there is no output until this function is called
      static void init();
      
      // log a timer
      static void log(ScopedTimer& timer);
      static void log(NSTimer& timer);
      static void log(ScopedUSRTimer& timer);
      // log a parm or a vector of parms
      static void log(const string& name, const vector<ParmData>& parms);
      static void log(const string& name, const ParmData& parm);
      static void log(const ParmData& parm);
      static void log(const string& text);
      
    private:
      Logger();
      ~Logger();
      static bool theirIsInitted;
      
      static int theirRank;
      static void doLog(const string& text);
    };
    
    inline void Logger::init(){
      theirIsInitted = true;
#ifdef HAVE_MPI
      if (theirRank == -1) {
	theirRank = TH_MPI::getCurrentRank();
      }
#endif
    }

    class ScopedTimer : private NSTimer
    {
    public:
      ScopedTimer(const string& name): NSTimer(name.c_str(), false), itsIsPrinted(false) {start();};
      ~ScopedTimer()                 { stop(); if (!itsIsPrinted) Logger::log(*this);};
      void end()                     { stop(); Logger::log(*this); itsIsPrinted = true;};
    private:
      friend class Logger;
      ScopedTimer();
      bool itsIsPrinted;
    };
    class ScopedUSRTimer : private casa::Timer
    {
    public:
      ScopedUSRTimer(const string& name): itsName(name), Timer(), itsIsPrinted(false) {};
      ~ScopedUSRTimer()                 { if (!itsIsPrinted) Logger::log(*this);};
      void end()                        { Logger::log(*this); itsIsPrinted = true;};
      const string& getName()           { return itsName;};
    private:
      friend class Logger;
      string itsName;
      ScopedUSRTimer();
      bool itsIsPrinted;
    };
  }

} // end namespace LOFAR

#endif
