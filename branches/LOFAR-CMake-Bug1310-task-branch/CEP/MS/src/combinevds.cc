//# combinevds.cc: Program to combine the description of MS parts
//#
//# Copyright (C) 2008
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
#include <MWCommon/VdsDesc.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <casa/OS/Path.h>
#include <stdexcept>
#include <iostream>
#include <fstream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::CEP;

int main(int argc, const char* argv[])
{
  try {
    if (argc < 3) {
      cout << "Run as:  combinevds outName in1 in2 ..." << endl;
      return 0;
    }

    // Form the global desc.
    VdsPartDesc globalvpd;
    globalvpd.setName (argv[1], string());
    vector<double> sfreq(1);
    vector<double> efreq(1);

    // Read all parts.
    // Add a band, but with only its start and end freq (not all freqs).
    vector<VdsPartDesc*> vpds;
    vpds.reserve (argc-2);
    for (int i=2; i<argc; ++i) {
      VdsPartDesc* vpd = new VdsPartDesc(ParameterSet(argv[i]));
      casa::Path path(argv[i]);
      // File name gets the original MS name.
      // Name gets the name of the VDS file.
      vpd->setFileName (vpd->getName());
      vpd->setName (path.absoluteName(), vpd->getFileSys());
      vpds.push_back (vpd);
      vpd->clearParms();
      const vector<int>& chans = vpd->getNChan();
      const vector<double>& sf = vpd->getStartFreqs();
      const vector<double>& ef = vpd->getEndFreqs();
      int inxf=0;
      for (uint i=0; i<chans.size(); ++i) {
	int nchan = chans[i];
	sfreq[0] = sf[inxf];
	// A band can be given with individual freqs or a single freq range.
	if (chans.size() == sf.size()) {
	  ++inxf;
	} else {
	  inxf += nchan;
	}
	efreq[0] = ef[inxf-1];
	globalvpd.addBand (nchan, sfreq, efreq);
      }
    }

    // Set the times in the global desc (using the first part).
    // Form the global desc.
    globalvpd.setTimes (vpds[0]->getStartTime(),
			vpds[0]->getEndTime(),
			vpds[0]->getStepTime(),
                        vpds[0]->getStartTimes(),
                        vpds[0]->getEndTimes());
    VdsDesc gdesc(globalvpd);

    // Now add all parts to the global desc and write it.
    // Print a warning if times differ.
    // Also cleanup the objects.
    for (uint i=0; i<vpds.size(); ++i) {
      gdesc.addPart (*vpds[i]);
      if (vpds[i]->getStartTime() != globalvpd.getStartTime()  ||
	  vpds[i]->getEndTime()   != globalvpd.getEndTime()    ||
	  vpds[i]->getStepTime( ) != globalvpd.getStepTime()) {
	cerr << "The times of part " << i << " (" << vpds[i]->getName()
	     << " are different" << endl;
      }
      delete vpds[i];
      vpds[i] = 0;

    }
    ofstream ostr(argv[1]);
    ASSERTSTR (ostr, "File " << argv[1] << " could not be created");
    gdesc.write (ostr);

  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}
