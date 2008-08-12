//# makevds.cc: Program to write the description of an MS
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>
#include <MS/VdsMaker.h>
#include<stdexcept>
#include <iostream>

using namespace std;


int main(int argc, const char* argv[])
{
  try {
    if (argc < 3) {
      cout << "Run as:  makevds clusterdesc ms [msvds]" << endl;
      return 0;
    }
    string msvds = string(argv[2]) + ".vds";
    if (argc > 3) {
      msvds = argv[3];
    }
    LOFAR::VdsMaker::create (argv[2], msvds, argv[1]);
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}
