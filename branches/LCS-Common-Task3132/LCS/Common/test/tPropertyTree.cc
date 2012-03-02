//# tPropertyTree.cc: Simple testprogrm to test the PropertyTree class.
//#
//# Copyright (C) 2002-2003
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

//# Always #inlcude <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/PropertyTree.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>

using namespace LOFAR;

int main()
{
  PropertyTree pt;
  pt.readParset("tPropertyTree.in_parset");
  ofstream ofs("tPropertyTree.out_parset");
  pt.writeParset(ofs);
  return 0;
}
