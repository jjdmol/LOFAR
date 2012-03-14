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

using namespace LOFAR;

int main()
{
  PropertyTree pt;
  pt.readParset("tPropertyTree.in_parset");
  pt.writeParset("tPropertyTree.out_parset");
  pt.writeJSON("tPropertyTree.out_json");
  pt.writeXML("tPropertyTree.out_xml");
  return 0;
}
