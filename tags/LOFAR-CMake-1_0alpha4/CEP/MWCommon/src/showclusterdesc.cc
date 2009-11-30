//# showclusterdesc.cc: show the (expanded) ClusterDesc
//# Copyright (C) 2009
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

#include <lofar_config.h>
#include <MWCommon/ClusterDesc.h>

using namespace LOFAR::CEP;
using namespace std;

int main (int argc, char* argv[])
{
  try {
    if (argc < 2) {
      cerr << "Run as:  showclusterdesc clusterdesc-file" << endl;
      return 1;
    }
    ClusterDesc cdesc (argv[1]);
    cdesc.write (cout);
  } catch (const std::exception& x) {
    cerr << "Error: " << x.what() << endl;
    return 1;
  }
  return 0;
}
