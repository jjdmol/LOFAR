//# Example2.cc: Test program for basic tinyCEPFrame classes
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <iostream>
#include <Transport/DataHolder.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <DH_Example.h>
#include <MySecondExample.h>

#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main(int argc, const char** argv)
{
  
#ifdef HAVE_MPI
  MPI_Init(&argc, (char***)&argv);
#endif

  // set trace levels
  //  ::Debug::initLevels(argc, argv);

  MySecondExample EX1(1, 1);
  EX1.setarg(argc, argv);

#if 0

  // use this branch for interactive tests.
  try {
    LOFAR::SimulatorParse::parse(EX1);
  } catch (LOFAR::SimulatorParseError x) {
    //    cout << x.getMesg() << endl;
    cout << x.what() << endl;
  }
#else
   
  EX1.baseDefine();
  EX1.basePrerun();
  EX1.baseDump();
  EX1.baseRun(10);
  EX1.baseDump();
  EX1.baseQuit();
#endif
  
  if ( 
      ( ((DH_Example*)EX1.itsWHs[0]->getDataManager().getInHolder(0))->getBuffer()[0] == 
        ((DH_Example*)EX1.itsWHs[1]->getDataManager().getOutHolder(0))->getBuffer()[0]) &&
      ( ((DH_Example*)EX1.itsWHs[0]->getDataManager().getInHolder(0))->getCounter() ==
	((DH_Example*)EX1.itsWHs[1]->getDataManager().getOutHolder(0))->getCounter() ) ) {
    
    // success
    return 0;
  } else {
    return -1;
  }
}
