//#  -*- mode: c++ -*-
//#
//#  StatsRead.h: Synchronize statistics with RSP hardware.
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

#ifndef STATSREAD_H_
#define STATSREAD_H_

#include "SyncAction.h"
#include <Common/LofarTypes.h>

namespace RSP
{
  class StatsRead : public SyncAction
  {
  public:
    /**
     * Constructors for a StatsRead object.
     */
    StatsRead(GCFPortInterface& board_port, int board_id, uint8 type,
	      uint8 nblps, uint8 nfragments);
	  
    /* Destructor for StatsRead. */
    virtual ~StatsRead();

    /**
     * Send the write message.
     */
    virtual void sendrequest();

    /**
     * Send the read request.
     */
    virtual void sendrequest_status();

    /**
     * Handle the read result.
     */
    virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

  private:
    uint8 m_type;       // statistics type
    uint8 m_nfragments; // register is too large for ethernet packet, use this many fragments
    uint8 m_nblsp;      // 1 for beamlet statistics BST, n for subband statistics SST
  };
};
     
#endif /* STATSREAD_H_ */
