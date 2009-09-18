//# testRunOnNode.cc: Program to test the interface to the LofarLogger.
//#
//# Copyright (C) 2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <Common/RunOnNode.h>

using namespace std;
using namespace LOFAR;

int main (int, const char**) {

  cout << "Program started" << endl;
  
  cout << "Initialise RunOnNode; tell this process to be (0,0)" << endl;
  SETNODE(0,0);
  
  cout << "Check for conditional execution" << endl;
  RUNINPROCESS(0,0)  cout << "Executing code for (0,0)" << endl;
  RUNINPROCESS(3,7)  cout << "ERROR: Executing code for (3,7)" << endl;

// todo:test reset of node (not implemented yet)
// 	cout << "Now re-init to (3,7)" << endl;
// 	SETNODE(3,7);
// 	cout << "Check for conditional execution" << endl;
// 	RUNINPROCESS(0,0)  cout << "ERROR: Executing code for (0,0)" << endl;
// 	RUNINPROCESS(3,7)  cout << "Executing code for (3,7)" << endl;

	
}

