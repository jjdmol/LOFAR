// tDFTServer.cc: Test program for the DFTServer
//
//  Copyright (C) 2004
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

#include <DFTServer/DFTServer.h>
#include <DFTServer/DFTAll.h>
#include <Common/LofarLogger.h>
#include <stdexcept>

using namespace LOFAR;
using namespace std;


void doIt()
{
  DFTAll dft;
  int nant = 4;
  int nbaseline = 6;
  int nfreq = 10;
  int ntime = 5;
  dft.setLM (0.1, 0.5);
  dft.set (100., 1., nfreq,     // start,step,n freq
	   50., 2., ntime,      // start,step,n time
	   nant, nbaseline);
  double* uvw = dft.accessUVW();
  int32* ant = dft.accessAnt();
  int32* ant1 = dft.accessAnt1();
  int32* ant2 = dft.accessAnt2();
  for (int i=0; i<ntime; i++) {
    *uvw++ = i;
    *uvw++ = i+0.3;
    *uvw++ = i+0.6;
  }
  for (int i=0; i<nant; i++) {
    ant[i] = i;
    for (int j=i+1; j<nant; j++) {
      *ant1++ = i;
      *ant2++ = j;
    }
  }
  dft.send();
  dft.receive();
  const double* val = dft.getValues();
  for (int i=0; i<2*nfreq*ntime*nbaseline; i++) {
    ASSERT (val[i] == i);
  }
}

int main()
{
  try {
    doIt();
  } catch (std::exception& x) {
    cout << "Unexpected esception: " << x.what() << endl;
  }
}
