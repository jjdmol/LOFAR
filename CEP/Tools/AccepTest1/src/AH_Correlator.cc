//#  AH_Correlator.cc: Round robin correlator based on the premise that 
//#  BlueGene is a hard real-time system.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <AH_Correlator.h>

extern "C" void traceback (void);

using namespace LOFAR;

AH_Correlator::AH_Correlator(int elements, int samples, int channels, 
		       char* ip, int baseport, int targets):
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels), 
  itsIP       (ip),
  itsBaseport (baseport),
  itsNtargets (targets)
{
}  

AH_Correlator::~AH_Correlator() {
}

void AH_Correlator::define(const KeyValueMap& /*params*/) {

#ifdef HAVE_MPI
  sleep(TH_MPI::getCurrentRank());
  cout << "defining node number " << TH_MPI::getCurrentRank() << endl;
#endif


  // create the primary WorkHolder to do the actual work
  itsWH = (WorkHolder*) new WH_Correlator("noname",
					  itsNelements, 
					  itsNsamples,
					  itsNchannels, 
					  itsNtargets);
  itsWH->runOnNode(TH_MPI::getCurrentRank());
  
  // now create two dummy workholders to connect to
  // these will not exist outside the scope of this method
  WH_Random myWHRandom("noname",
		       itsNelements, 
		       itsNsamples,
		       itsNchannels);
  
  WH_Dump myWHDump("noname",
		   itsNelements,
		   itsNchannels);
  
  // now connect to the dummy workholders. 
#ifdef HAVE_MPI
  myWHRandom.getDataManager().getOutHolder(0)->connectTo 
    ( *itsWH->getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+TH_MPI::getCurrentRank(), true) );

  cout << "reading from port number: " << itsBaseport+TH_MPI::getCurrentRank() << " " << endl;

  itsWH->getDataManager().getOutHolder(0)->connectTo
    ( *myWHDump.getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+itsNtargets+TH_MPI::getCurrentRank(), false));
  cout << "writing to port number: " << itsBaseport+itsNtargets+TH_MPI::getCurrentRank() << endl;
#endif

}

void AH_Correlator::undefine() {
  delete itsWH;
}


void AH_Correlator::init() {
  itsWH->basePreprocess();
}

void AH_Correlator::run(int nsteps) {
  for (int i = 0; i < nsteps; i++) {
    itsWH->baseProcess();
  }
}

void AH_Correlator::dump () {
  itsWH->dump();
}

void AH_Correlator::postrun() {
  itsWH->basePostprocess();
}

void AH_Correlator::quit() {
}
