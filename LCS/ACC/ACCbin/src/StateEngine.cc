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

const uint16	CMD_SEQ_LENGTH	=	4;

typedef struct CmdSeq {
	ACCmd		userCmd;
	ACState		cmdSeq [CMD_SEQ_LENGTH];
} CmdSeq_t;

static CmdSeq_t theirCmdSeqTable[] = { 
{ ACCmdNone,	
	{ StateNone,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdBoot,	
	{ StateInitController, StatePowerUpNodes, StateCreatePSubset, StateNone } },
{ ACCmdDefine,	
	{ StateStartupAppl,	   StateDefineCmd,	  StateNone,          StateNone } },
{ ACCmdInit,	
	{ StateInitCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdRun,		
	{ StateRunCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdPause,	
	{ StatePauseCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdSnapshot,{ 
	StateSnapshotCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdRecover,	
	{ StateRecoverCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdReinit,	// TODO
	{ StateReinitCmd,	   StateNone,		  StateNone,          StateNone } },
{ ACCmdReplace,	// TODO
	{ StateNone,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdInfo,	
	{ StateInfoCmd,		   StateNone,		  StateNone,          StateNone } },
{ ACCmdQuit,	
	{ StateQuitCmd,		   StateKillAppl,	  StateNone,          StateNone } },
{ ACCmdShutdown,
	{ StatePowerDownNodes, StateNone,		  StateNone,          StateNone } }
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
	StateLifeTime[StateInitController] = 900;	// should be long enough
}

StateEngine::~StateEngine()
{}

void StateEngine::init(const ParameterSet*	aPS)
{
	// (constant) timer values for the states
	StateLifeTime[StatePowerUpNodes]  = aPS->getTime("AC.timeout.powerup");
	StateLifeTime[StatePowerDownNodes]= aPS->getTime("AC.timeout.powerdown");
	StateLifeTime[StateCreatePSubset] = aPS->getTime("AC.timeout.createsubsets");
    StateLifeTime[StateStartupAppl]	  = aPS->getTime("AC.timeout.startup");
	StateLifeTime[StateDefineCmd]	  = aPS->getTime("AC.timeout.define");
	StateLifeTime[StateInitCmd]		  = aPS->getTime("AC.timeout.init");
    StateLifeTime[StateRunCmd]		  = aPS->getTime("AC.timeout.run");
	StateLifeTime[StatePauseCmd]	  = aPS->getTime("AC.timeout.pause");
	StateLifeTime[StateRecoverCmd]	  = aPS->getTime("AC.timeout.recover");
	StateLifeTime[StateSnapshotCmd]	  = aPS->getTime("AC.timeout.snapshot");
	StateLifeTime[StateReinitCmd]	  = aPS->getTime("AC.timeout.reinit");
	StateLifeTime[StateInfoCmd]		  = aPS->getTime("AC.timeout.info");
	StateLifeTime[StateQuitCmd]		  = aPS->getTime("AC.timeout.quit");
	StateLifeTime[StateKillAppl]	  = aPS->getTime("AC.timeout.kill");
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
	
	THROW (Exception, "No command sequences start with command " << aStartPoint);

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

//	setStateLifeTime(StateLifeTime[stateNr]);	TODO: do this here?

	return (stateNr);
}

string StateEngine::stateStr(uint16	stateNr) const
{
	static char* const stateNames[] = {
		"Idle",
		"InitController",
		"PowerUpNodes",
		"PowerDownNodes",
		"CreatePSubset",
		"StartupAppl",
		"DefineCmd",
		"InitCmd",
		"RunCmd",
		"PauseCmd",
		"RecoverCmd",
		"SnapshotCmd",
		"ReinitCmd",
		"InfoCmd",
		"QuitCmd",
		"KillAppl"
	};

	if (stateNr >= NR_OF_STATES) {
		return ("");
	}

	return (stateNames[stateNr]);
}

  } // namespace ACC
} // namespace LOFAR
