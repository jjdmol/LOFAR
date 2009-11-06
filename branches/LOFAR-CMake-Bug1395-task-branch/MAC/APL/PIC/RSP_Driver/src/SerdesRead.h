//#  -*- mode: c++ -*-
//#
//#  SerdesRead.h: Write to the serdes registers
//#
//#  Copyright (C) 2009
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

#ifndef SERDESREAD_H_
#define SERDESREAD_H_

#include <Common/LofarTypes.h>
#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

class SerdesRead : public SyncAction
{
public:
	// Constructors for a SerdesRead object.
	SerdesRead(GCFPortInterface& board_port, int board_id, int	nrCycles);

	// Destructor for SerdesRead. */
	virtual ~SerdesRead();

	// Write subband selection info.
	virtual void sendrequest();

	// Read the board status.
	virtual void sendrequest_status();

	// Handle the READRES message.
	virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

private:
	EPA_Protocol::MEPHeader itsHeader;
	int						itsSeqNr;
};

  }; // namespace RSP
}; // namespace LOFAR

#endif /* SERDESREAD_H_ */
