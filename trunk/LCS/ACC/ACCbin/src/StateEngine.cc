//#  StateEngine.cc: (internal) command sequence definitions
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <ACC/StateEngine.h>

namespace LOFAR {
  namespace ACC {

const uint16	CMD_SEQ_LENGTH	=	3;

typedef struct CmdSeq {
	ACCmd		userCmd;
	ACState		cmdSeq [CMD_SEQ_LENGTH];
} CmdSeq_t;

static CmdSeq_t theirCmdSeqTable[] = { 
	{ ACCmdNone,	{ StateNone,		   StateNone,		   StateNone } },
	{ ACCmdBoot,	{ StatePowerUpNodes,   StateCreatePSubset, StateNone } },
	{ ACCmdDefine,	{ StateStartupAppl,	   StateDefineCmd,	   StateNone } },
	{ ACCmdInit,	{ StateInitCmd,		   StateNone,		   StateNone } },
	{ ACCmdRun,		{ StateRunCmd,		   StateNone,		   StateNone } },
	{ ACCmdPause,	{ StatePauseCmd,	   StateNone,		   StateNone } },
	{ ACCmdSnapshot,{ StateSnapshotCmd,	   StateNone,		   StateNone } },
	{ ACCmdRecover,	{ StateRecoverCmd,	   StateNone,		   StateNone } },
	{ ACCmdReinit,	{ StateReinitCmd,	   StateNone,		   StateNone } },//TODO
	{ ACCmdReplace,	{ StateNone,		   StateNone,		   StateNone } },//TODO
	{ ACCmdInfo,	{ StateInfoCmd,		   StateNone,		   StateNone } },
	{ ACCmdQuit,	{ StateQuitCmd,		   StateKillAppl,	   StateNone } },
	{ ACCmdShutdown,{ StatePowerDownNodes, StateNone,		   StateNone } }
};

#define	CMD_SEQ_TABLE_SIZE	((sizeof(theirCmdSeqTable)/sizeof(CmdSeq_t)) - 1)

StateEngine::StateEngine() :
	itsSequence     (0),
	itsStateNr      (0),
	itsWantNewState (false)
{}

StateEngine::~StateEngine()
{}

void StateEngine::reset()
{
	itsSequence     = 0;
	itsStateNr      = 0;
	itsWantNewState = false;
}

ACState StateEngine::startSequence(ACCmd	aStartPoint) throw (Exception)
{
	for (int i = CMD_SEQ_TABLE_SIZE; i > 0; i--) {
		if (theirCmdSeqTable[i].userCmd == aStartPoint) {
			itsStateNr  = 0;
			itsSequence = i;
			itsWantNewState = false;
			LOG_TRACE_STAT (formatString("StateEngine:startSeq[%d][%d]=%d",
							itsSequence, itsStateNr,
							theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr]));
			return (theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr]);
		}
	}
	
	THROW (Exception, "No command sequences start with command " << aStartPoint);

}

ACState StateEngine::getState()
{
	return (theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr]);
}

ACState StateEngine::nextState()
{
	itsStateNr++;
	itsWantNewState = false;

	ASSERTSTR(itsStateNr < CMD_SEQ_LENGTH, "itsStateNr became " << itsStateNr);

	// end of sequence reached? Reset indices.
	if (theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr] == StateNone) {
		reset();
	}

	LOG_TRACE_STAT (formatString("StateEngine:nextState[%d][%d]=%d",
							itsSequence, itsStateNr,
							theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr]));

	return (theirCmdSeqTable[itsSequence].cmdSeq[itsStateNr]);
}



  } // namespace ACC
} // namespace LOFAR
