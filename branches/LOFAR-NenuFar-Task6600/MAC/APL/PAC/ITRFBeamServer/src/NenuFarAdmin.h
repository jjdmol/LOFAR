//#  NenuFarAdmin.h: interface of the NenuFarAdmin class
//#
//#  Copyright (C) 2014
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
//#  $Id: NenuFarAdmin.h 22248 2012-10-08 12:34:59Z overeem $

#ifndef BEAMSERVER_NENUFARADMIN_H_
#define BEAMSERVER_NENUFARADMIN_H_

#include <lofar_config.h>
#include <Common/lofar_list.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationDatatypes.h>
#include <APL/IBS_Protocol/Pointing.h>

namespace LOFAR {
  namespace BS {

// This class does all the 'clever' bookkeeping of the beams that should be communicated to NenuFar.
// The bookkeeping is based on the beam_state and the comm_state: the beam_state is the actual state
// of the beam (= the state the beam has in LOFAR) and the comm_state is the state that has been communicated
// to NenuFar. Normally these states would be in sync but when the communication with NenuFar is lost 
// this could be different.
// The LOFAR task uses the addBeam and abort(All)Beam(s) calls to update its state.
// The NenuFar task uses the firstBeam call to let this class decide what must be communicated next,
// and setCommState to update its state in the NenuFarAdmin.
//
// Cleanup of the administration is included in the algorithms.
//
// It finally boils down to: as long a NenuFar is not informed about a beam it can be removed without notice
// thus as soon as NenuFar does know about a beam it must be kept until it was handled properly.
//
// Extra safety is that NenuFar gets the start- and endTime in the 'newbeam' command so that it is capable
// to end the beam in time even when the communication is lost afterwards.
//
class NenuFarAdmin {
public:
	// Default constructor
	NenuFarAdmin(): itsAllBeamsAborted(false),itsIsActive(false) {};

	// Default destructor.
	virtual ~NenuFarAdmin();

	// Add a beam to the admin
	void addBeam(const string& 					beamName, 
				const string& 					antennaSet, 
		 		const RCUmask_t&				RCUselection,
				int 							rank, 
				const IBS_Protocol::Pointing& 	pointing, 
				const vector<string>&			extraInfo);

	// Abort a beam from the admin
	bool abortBeam(const string& beamName);
	void abortAllBeams();

	// define a class that holds all information of one beam (= one pointing)
	class BeamInfo {
	  public:
		static const int BS_NONE  = 0;
		static const int BS_NEW   = 1;
		static const int BS_ENDED = 2;
		static const int BS_ABORT = 3;
		BeamInfo() :
			startTime    (0),
			endTime      (0),
			beamName     (""),
			antennaSet   (""),
			RCUselection (bitset<192>(0)),
			rankNr	     (0),
			angle2Pi     (0.0),
			anglePi	     (0.0),
			beam_state	 (BS_NONE),
			comm_state	 (BS_NONE) {};
		BeamInfo(const string& nameOfBeam, const string& antennaSet, const RCUmask_t& RCUselect, int rank,
				 const IBS_Protocol::Pointing& pointing, const vector<string>& extraInfo) :
			startTime    (pointing.time().sec()),
			endTime      (pointing.endTime().sec()),
			beamName     (nameOfBeam),
			antennaSet   (antennaSet),
			RCUselection (RCUselect),
			rankNr	     (rank),
			angle2Pi     (pointing.angle0()),
			anglePi	     (pointing.angle1()),
			coordFrame	 (pointing.getType()),
			extra		 (extraInfo),
			beam_state	 (BS_NEW),
			comm_state	 (BS_NONE) {};
		ParameterSet	asParset()  const ;
		ParameterSet	nameAsKV()  const { ParameterSet ps; ps.add("beamName", beamName); return(ps); }
		string			name()	    const { return (beamName); }
		int				beamState() const { return (beam_state); }
		int				commState() const { return (comm_state); }
		static string	stateName(int state) { switch (state) {
												 case 0: return "NONE";  break;
												 case 1: return "NEW";   break;
												 case 2: return "ENDED"; break;
												 case 3: return "ABORT"; break; }; 
												 return (""); }
	  private:
		time_t			startTime;
		time_t			endTime;
		string			beamName;
		string			antennaSet;
		RCUmask_t		RCUselection;
		int				rankNr;
		double			angle2Pi;
		double			anglePi;
		string			coordFrame;
		vector<string>	extra;
		int				beam_state;
		int				comm_state;

		friend class NenuFarAdmin;
	};
		
	// @return Current pointing.
	BeamInfo	firstCommand(time_t	atTime);

	// set communication state of a beam
	bool	setCommState(const string& beamName, int new_comm_state);

	// For testing only
	size_t	nrBeams() const { return (itsBeams.size()); }

	// Check for abortion of all beams.
	bool	allBeamsAborted() { bool result = itsAllBeamsAborted; itsAllBeamsAborted = false; return result; }

	// Enable administration
	void	activateAdmin() { itsIsActive = true; };

	// print function for operator<<
	ostream& print (ostream& os) const;

private:
	//# ----- DATAMEMBERS -----
	// queue of future pointings as delivered by the user.
	list<BeamInfo>			itsBeams;

	bool					itsAllBeamsAborted;

	bool					itsIsActive;	// if not activated all functions are idle.
};

//# -------------------- inline functions --------------------
//
// operator <<
//
inline ostream& operator<<(ostream& os, const NenuFarAdmin& nfa)
{
	return (nfa.print(os));
}


  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAMSERVER_NENUFARADMIN_H_ */
