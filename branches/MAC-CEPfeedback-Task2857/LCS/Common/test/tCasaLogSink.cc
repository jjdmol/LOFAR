//# tCasaLogSink.h: Test program for class CasaLogSink
//#
//# Copyright (C) 2011
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

// @author Ger van Diepen (gvd AT astron DOT nl)

#include <lofar_config.h>
#include <Common/CasaLogSink.h>
#include <Common/lofar_iostream.h>

#ifdef HAVE_AIPSPP
#include <casa/Logging/LogIO.h>
#endif

using namespace LOFAR;

int main()
{
  CasaLogSink::attach();

#ifdef HAVE_AIPSPP
  cout << "writing to casa logger ..." << endl;
  casa::LogIO logger;
  logger << casa::LogIO::DEBUG1 << "debug  message" << casa::LogIO::POST;
  logger << casa::LogIO::NORMAL << "normal message" << casa::LogIO::POST;
  logger << casa::LogIO::WARN   << "warn   message" << casa::LogIO::POST;
  logger << casa::LogIO::SEVERE << "error  message" << casa::LogIO::POST;
#endif

  return 0;
}
