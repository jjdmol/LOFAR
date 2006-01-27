//# readmsdesc.cc: Program to read the description of an MS
//#
//# Copyright (C) 2006
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
#include <MS/MSDesc.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Common/LofarLogger.h>

#include <iostream>
#include <fstream>

using namespace LOFAR;
using namespace std;


int main(int argc, char** argv)
{
  try {
    if (argc < 2) {
      cout << "Run as:  readmsdesc filename" << endl;
      return 0;
    }
    ifstream istr(argv[1]);
    ASSERTSTR (istr, "File " << argv[1] << " could not be opened");
    BlobIBufStream bbs(istr);
    BlobIStream bis(bbs);
    MSDesc msd;
    bis >> msd;
    cout << msd;
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}
