//#  GPA_RequestManager.h: manages a FIFO queue of requests of the PA
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

#ifndef GPA_REQUESTMANAGER_H
#define GPA_REQUESTMANAGER_H

#include <GPA_Defines.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFPortInterface;
class GCFEvent;
  }
  namespace PAL 
  {

/**
   This class manages a FIFO queue of requests, which 
   can be received from a PML and not handled immediately.
*/
class GPARequestManager 
{
	public:
		GPARequestManager();
		virtual ~GPARequestManager();
		
		void registerRequest(const TM::GCFEvent& e, TM::GCFPortInterface& requestPort);
		TM::GCFEvent* getOldestRequest();
		TM::GCFPortInterface* getOldestRequestPort();
    
		void deleteOldestRequest();
    void deleteRequestsOfPort(const TM::GCFPortInterface& requestPort);
    void deleteAllRequests();
	
	private: // helper methods

	private: // data members
		typedef struct 
		{
			TM::GCFPortInterface* pRPort;
			const TM::GCFEvent* pEvent;
		} TRequest;
		
		list<TRequest> 	_requests;
};

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
