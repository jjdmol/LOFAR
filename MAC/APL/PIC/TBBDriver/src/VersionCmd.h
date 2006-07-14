//#  -*- mode: c++ -*-
//#
//#  GetVersions.h: Get Tbb board versions
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

#ifndef GETVERSIONS_H_
#define GETVERSIONS_H_

#include "BoardAction.h"
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/TBB_Protocol/TP_Protocol.ph>

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace TBB {

		class VersionCmd : public Command 
		{
			public:
				/// Constructors for a GetVersions object.
				GetVersionsCmd(GCFEvent& event);
	  
				/// Destructor for GetVersions.
				virtual ~GetVersionsCmd();
				
				virtual bool isValid(GCFEvent& event);
				
				virtual void sendTpEvent(GCFEvent& event, GCFPortInterface& boardport[]);

				virtual void saveTpAckEvent(GCFEvent& event);

				virtual void sendTbbAckEvent(void);

      
			private:
				GCFPortInterface*		m_ClientPort;
				uint32							m_SendMask;  // mask indicates the boards to communicate with
				uint32							m_RecvMask;  // mask indicates the boards to communicate with
				
				uint32	m_boardid[12];
				uint32	m_swversion[12];
				uint32	m_boardversion[12];
				uint32	m_tpversion[12];
				uint32	m_mpversion[12][4];
		};
	};
};

#endif /* GETVERSIONS_H_ */
