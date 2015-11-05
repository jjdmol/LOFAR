//#  -*- mode: c++ -*-
//#
//#  beamctl.h: class definition for the beamctl program
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

#ifndef BEAMCTL_H_
#define BEAMCTL_H_

#include <Common/LofarTypes.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <GCF/TM/GCF_Control.h>
#include <APL/IBS_Protocol/Pointing.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFPortInterface;
  namespace BS {

class beamctl : public GCFTask
{
public:
	// The constructor of the beamctl task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	beamctl(const string& name);
	~beamctl();

	/*@{*/
	// The states.
	GCFEvent::TResult checkUserInput (GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult con2calserver  (GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult con2beamserver (GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult create_subarray(GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult create_beam	 (GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult sendPointings	 (GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult final		     (GCFEvent& e, GCFPortInterface &p);
	/*@}*/

private:
	// Return the seleted rcus as bitset<MAX_N_RCUS>
	bool					parseOptions(int myArgc, char** myArgv);
	bool					checkOptions();
	bitset<LOFAR::MAX_RCUS> getRCUMask() const;
	list<int> 				strtolist(const char* str, int max) const;
	void 			 		printList(list<int>&		theList) const;
	void 					usage() const;
	void send_direction(double	longitude, double	latitude, const string&	dirType, bool	isAnalogue);

	// ports
	GCFTCPPort*		itsCalServer;
	GCFTCPPort*		itsBeamServer;

	// handles
	string			itsBeamHandle;

	// place to store the argument values.
	string			itsAntSet; 			// name of the antennaSet
	list<int>		itsRCUs;   			// selection of rcus
	list<int>		itsSubbands; 		// selection of subbands
	list<int>		itsBeamlets; 		// selection of beamlets
	int				itsRCUmode;
	list<Pointing>	itsDigPointings;
	list<Pointing>	itsAnaPointings;

	int 	itsSkyScanTotalTime;
	int 	itsSkyScanPointTime;
	int 	itsSkyScanWaitTime;
};

  }; // namespace BS
}; // namespace LOFAR
     
#endif /* BEAMCTL_H_ */
