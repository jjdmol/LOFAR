//#  -*- mode: c++ -*-
//#  ACMProxy.h: class definition for the ACMProxy task.
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
//#  $Id: ACMProxy.h 10791 2008-01-08 14:12:35Z overeem $

#ifndef ACMPROXY_H_
#define ACMPROXY_H_

#include <Common/LofarConstants.h>
#include <GCF/TM/GCF_Control.h>	// The lot: Task, Port, Fsm, Timer etc.
#include <APL/RTCCommon/Timestamp.h>
#include <APL/ICAL_Protocol/SubArray.h>				// for RCUmask_t
#include <APL/LBA_Calibration/lba_calibration.h>	// the matlab stuff
//#include <APL/RSP_Protocol/XCStatistics.h>
#include "ACCcache.h"
#include "RequestPool.h"

namespace LOFAR {
  typedef		void*		memptr_t;
  using GCF::TM::GCFEvent;
  using GCF::TM::GCFTask;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFPortInterface;
  using RTC::Timestamp;
  namespace ICAL {

// The ACMProxy class is a 'independant' task that connects to the RSPDriver and
// collects the cross-correlation information of each subband and stores it in the
// back-cache of the ACCcache. Start of the collectioncycle is trigger with the
// 'wating4start' flag, end of the cycle is marked by setting the 'ready' flag.
class ACMProxy : public GCFTask
{
public:
	// The constructor of the ACMProxy task.
	// @param name The name of the task. 
	// @param theACC Reference to a cache with two MWArray's for storing ACM information.
	ACMProxy(const string& name, ACCcache&	theACCs);
	~ACMProxy();

	/*@{*/
	// States
	GCFEvent::TResult con2RSPDriver		(GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult idle				(GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult getXCsubscription (GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult collectACinfo		(GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult unsubscribing		(GCFEvent& e, GCFPortInterface& port);
	/*@}*/

private:
	ACCcache&		itsACCs;			// pointer to a pointer to a mwArray containing the MatlabACC
										// the ACMProxy may use.
	// Port to the RSPDriver.
	GCFTCPPort*				itsRSPDriver;		// connection to the RSPDriver
	GCFTimerPort*			itsTimerPort;		// connection to the RSPDriver
	memptr_t				itsSubscrHandle; 	// handle for the UPDXCSTATS events
	RCUmask_t				itsRCUmask;
	RequestPool*			itsRequestPool;		// pool with outstanding XCstat requests.

	// Subband administration
	int				itsFirstSubband;	// lowest subband to calibrate.
	int				itsLastSubband;		// highest subband to calibrate.
	int				itsRequestSubband;	// Subband that must be requested
	int				itsReceiveSubband;	// Subband expected to receive.

	Timestamp  		itsStartTime;		// first ACM will be received at this time
};

  }; // namespace CAL
}; // namespace LOFAR
     
#endif /* ACMPROXY_H_ */
