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

#include <BBSTestLogger.h>
#include <sstream>

namespace LOFAR{

  ofstream* BBSTestLogger::theirOutFile = 0;

  BBSTestLogger::BBSTestLogger() {
    getLogFile();    
  }

  ofstream& BBSTestLogger::getLogFile() {
    if (theirOutFile == 0) {
      string oFileName = "BBSTest.out";
#ifdef HAVE_MPI
      int isInitialized = 0;
      MPI_Initialized(&isInitialized); 
      // this class can also be used by parmdb (which can be built with HAVE_MPI)
      // parmdb does not call MPI_INIT, so we can't get our rank
      if (isInitialized) {
	std::stringstream rankss;
	rankss << TH_MPI::getCurrentRank();
	oFileName = "BBSTest." + rankss.str() + ".out";
      }
#endif
      theirOutFile = new ofstream(oFileName.c_str(), ofstream::trunc);
    }
    return *theirOutFile;
  }    

  BBSTestLogger::~BBSTestLogger() {
  }

  void BBSTestLogger::log(const string& name, NSTimer& timer)
  { getLogFile() << "BBSTest: timer "<<name; timer.print(getLogFile()); getLogFile()<<endl;}
  void BBSTestLogger::log(const string& name, const MeqMatrix& mat)
  { getLogFile() << "BBSTest: parm "<<name<<" "<<mat<<endl;}
  void BBSTestLogger::log(const string& text)
  { getLogFile() << "BBSTest: "<<text<<endl;}

} // namespace LOFAR
