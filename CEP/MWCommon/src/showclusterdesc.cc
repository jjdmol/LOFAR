//# showclusterdesc.cc: show the (expanded) ClusterDesc
//# Copyright (C) 2009
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
