//#  GCF_ITCPort.h: Direct communication between two tasks in the same process
//#
//#  Copyright (C) 2002-2003
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

#ifndef LOFAR_GCF_TM_GCFITCPORT_H
#define LOFAR_GCF_TM_GCFITCPORT_H

#include <Common/lofar_set.h>
#include <GCF/TM/GCF_RawPort.h>

namespace LOFAR {
  namespace GCF {
    namespace TM {
    
// forward declaration
class GCFTask;

/**
 * This class represents a protocol port that is used to exchange events defined 
 * in a protocol betweem two tasks in the same process. Events are dispatched
 * to the state machine of the client task.
 */
class GCFITCPort : public GCF::TM::GCFRawPort
{
public:
	// constructor. Note argument 'protocol' is not used.
	GCFITCPort (GCF::TM::GCFTask&	containerTask, 
				GCF::TM::GCFTask&	slaveTask, 
				const string& 		name, 
				TPortType 			type, 
				int 				protocol);


	// destructor.
	virtual ~GCFITCPort ();

	// open and close functions
	virtual bool open ();
	virtual bool close ();

	// send and recv functions
	virtual ssize_t send 	(GCFEvent& event);                          
	virtual ssize_t sendBack(GCFEvent& event);
	virtual ssize_t recv 	(void* buf, size_t count);

protected:
	virtual GCFEvent::TResult   dispatch (GCFEvent& event);

private:
	// Copying is not allowed
	GCFITCPort ();
	GCFITCPort (const GCFITCPort&);
	GCFITCPort& operator= (const GCFITCPort&);

	// datamembers
	GCFTask&			itsSlaveTask;
	std::set<long> 		itsToSlaveTimerId;
	std::set<long> 		itsToContainerTimerId;
};

    }; // TM
  }; // GCF
}; // LOFAR
#endif
