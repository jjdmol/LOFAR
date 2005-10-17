//# Example3.cc: Test program for socket transfer using tinyCEP
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
#include <MySocketExample.h>

#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main (int argc, const char** argv) {

  MySocketExample* EX1;

  //  ::Debug::initLevels(argv, argc);

  if (argc < 2) {
    cout << "Usage " << argv[0] << " -s|-r"<< endl;
    return 1;
  }

  if (!strcmp(argv[1], "-r")) {
    EX1 = new MySocketExample (1, 0, false);

  } else if (!strcmp(argv[1], "-s")) {
    EX1 = new MySocketExample (0, 1, true);
    

  } else {
    cout << "Usage " << argv[0] << " -s|-r"<< endl;
    return 1;
  }

  EX1->setarg(argc, argv);
  EX1->baseDefine();
  EX1->basePrerun();
  EX1->baseDump();
  EX1->baseRun(10);
  EX1->baseDump();
  EX1->baseQuit();

}
