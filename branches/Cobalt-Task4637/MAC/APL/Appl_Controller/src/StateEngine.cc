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
#include "StateEngine.h"

namespace LOFAR {
  namespace ACC {

const uint16	CMD_SEQ_LENGTH	=	4;

typedef struct CmdSeq {
	ACCmd		userCmd;
	ACState		cmdSeq [CMD_SEQ_LENGTH];
} CmdSeq_t;

static CmdSeq_t theirCmdSeqTable[] = { 
{ ACCmdNone,	
	{ StateNone,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdBoot,	
	{ StateInitController, StateCreatePSubset,StateStartupAppl,   StateNone } },
{ ACCmdDefine,	
	{ StateDefineCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdInit,	
	{ StateInitCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdRun,		
	{ StateRunCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdPause,	
	{ StatePauseCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdRelease,	
	{ StateReleaseCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdSnapshot,{ 
	StateSnapshotCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdRecover,	
	{ StateRecoverCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdReinit,	// TODO
	{ StateReinitCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdInfo,	
	{ StateInfoCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdQuit,	
	{ StateQuitCmd,		   StateKillAppl,	  StateNone,          StateNone } }
};

// Array with the max lifetime times of the states
time_t	StateLifeTime [NR_OF_STATES];

#define	CMD_SEQ_TABLE_SIZE	((sizeof(theirCmdSeqTable)/sizeof(CmdSeq_t)) - 1)

StateEngine::StateEngine() :
	itsSequence       (0),
	itsStepNr         (0),
	itsStateFinished  (false),
	itsStateExpireTime(0)		
{
	// The state InitController runs before any timevalues are known.
	StateLifeTime[StateInitController] = 900;
}

StateEngine::~StateEngine()
{}

void StateEngine::init(const ParameterSet*	aPS)
{
	// (constant) timer values for the states
	StateLifeTime[StateCreatePSubset]= aPS->getTime("ApplCtrl.timeout_createsubsets");
    StateLifeTime[StateStartupAppl]	 = aPS->getTime("ApplCtrl.timeout_startup");
	StateLifeTime[StateDefineCmd]	 = aPS->getTime("ApplCtrl.timeout_define");
	StateLifeTime[StateInitCmd]		 = aPS->getTime("ApplCtrl.timeout_init");
    StateLifeTime[StateRunCmd]		 = aPS->getTime("ApplCtrl.timeout_run");
	StateLifeTime[StatePauseCmd]	 = aPS->getTime("ApplCtrl.timeout_pause");
	StateLifeTime[StateReleaseCmd]	 = aPS->getTime("ApplCtrl.timeout_release");
	StateLifeTime[StateRecoverCmd]	 = aPS->getTime("ApplCtrl.timeout_recover");
	StateLifeTime[StateSnapshotCmd]	 = aPS->getTime("ApplCtrl.timeout_snapshot");
	StateLifeTime[StateReinitCmd]	 = aPS->getTime("ApplCtrl.timeout_reinit");
	StateLifeTime[StateInfoCmd]		 = aPS->getTime("ApplCtrl.timeout_info");
	StateLifeTime[StateQuitCmd]		 = aPS->getTime("ApplCtrl.timeout_quit");
	StateLifeTime[StateKillAppl]	 = aPS->getTime("ApplCtrl.timeout_kill");
}

void StateEngine::reset()
{
	LOG_TRACE_STAT ("StateEngine:reset");

	itsSequence        = 0;
	itsStepNr          = 0;
	itsStateFinished   = false;
	itsStateExpireTime = 0;
}

ACState StateEngine::startSequence(ACCmd	aStartPoint) throw (Exception)
{
	// loop through SeqTable to find the given ACCmd.
	for (int i = CMD_SEQ_TABLE_SIZE; i > 0; i--) {
		if (theirCmdSeqTable[i].userCmd == aStartPoint) {
			// startpoint of sequence found, start with first state
			itsSequence     = i;
			itsStepNr       = 0;
			itsStateFinished= false;
			ACState	stateNr = theirCmdSeqTable[itsSequence].cmdSeq[itsStepNr];

			LOG_DEBUG (formatString("Start stateSequence[%d][%d]=%s, time=%d",
							itsSequence, itsStepNr, 
							stateStr(stateNr).c_str(),
							StateLifeTime[stateNr]));

			// start timer for this state
			setStateLifeTime(StateLifeTime[stateNr]);
			return (stateNr);
		}
	}
	
	THROW (Exception, "No command sequences start with command " << ACCmdName(aStartPoint));

}

ACState StateEngine::getState()
{
	return (theirCmdSeqTable[itsSequence].cmdSeq[itsStepNr]);
}

ACState StateEngine::nextState()
{
	itsStepNr++;
	itsStateFinished = false;

	ASSERTSTR(itsStepNr < CMD_SEQ_LENGTH, "itsStepNr became " << itsStepNr);

	// end of sequence reached? Reset indices.
	if (theirCmdSeqTable[itsSequence].cmdSeq[itsStepNr] == StateNone) {
		reset();
	}

	ACState	stateNr = theirCmdSeqTable[itsSequence].cmdSeq[itsStepNr];

	LOG_DEBUG (formatString("Next state[%d][%d]=%s, time=%d",
							itsSequence, itsStepNr, 
							stateStr(stateNr).c_str(),
							StateLifeTime[stateNr]));

	setStateLifeTime(StateLifeTime[stateNr]);

	return (stateNr);
}

string StateEngine::stateStr(uint16	stateNr) const
{
	static const char* const stateNames[] = {
		"Idle",
		"InitController",
		"CreatePSubset",
		"StartupAppl",
		"DefineState",
		"InitState",
		"RunState",
		"PauseState",
		"ReleaseState",
		"RecoverState",
		"SnapshotState",
		"ReinitState",
		"InfoState",
		"QuitState",
		"KillAppl"
	};

	if (stateNr >= NR_OF_STATES) {
		return ("");
	}

	return (stateNames[stateNr]);
}

//
// operator<<
//
std::ostream&	operator<< (std::ostream& os, const StateEngine& anEngine)
{
	if (anEngine.itsStateExpireTime) {
		os << "Timer  : " << timeString(anEngine.itsStateExpireTime) << endl;
	    os << "State  : " << 
			anEngine.stateStr(theirCmdSeqTable[anEngine.itsSequence].cmdSeq[anEngine.itsStepNr]) << endl;

	}
	else {
		os << "Timer  : off" << endl;
	}

	return (os);
}

  } // namespace ACC
} // namespace LOFAR
