//#  StateEngine.h: (internal) sequence definitions of the command states.
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_STATEENGINE_H
#define LOFAR_ACCBIN_STATEENGINE_H

// \file
// (internal) sequence definitions of the command states.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Exception.h>
#include <ALC/DH_ApplControl.h>
#include <Common/ParameterSet.h>

using namespace LOFAR::ACC::ALC;

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// define application controller states.
enum ACState {  StateNone = 0, StateInitController,
		StateCreatePSubset, StateStartupAppl,  StateDefineCmd,
		StateInitCmd,		StateRunCmd,       StatePauseCmd,
		StateReleaseCmd,	StateRecoverCmd,   StateSnapshotCmd,  
		StateReinitCmd, 	StateInfoCmd,	   StateQuitCmd,
		StateKillAppl,
		NR_OF_STATES
};

// The execution of an AC command is cut into several states that must be
// performed consecutive. The StateEngine manages these sequences of states.
class StateEngine
{
public:
	StateEngine();
	~StateEngine();

	// Initialize the Engine with timer values.
	void init(const ParameterSet* aPS);
	
	// Reset to original state.
	void reset();

	// Start a new state sequence. When the requested state is not a start of
	// a sequence an exception is thrown.
	ACState  startSequence(ACCmd  aStartPoint)		throw (Exception);

	// Get current state.
	ACState getState();

	// Current state is finished, hop the next state and return its value.
	ACState nextState();

	// Report the current state is ready.
	void	ready();

	// Ask if the next state is waiting.
	bool	isStateFinished();

	// Command for handling the state expire timer
	void setStateLifeTime	 (time_t		anInterval);
	void resetStateExpireTime();
	bool IsStateExpired      ();

	// return name of state
	string stateStr(uint16	stateNr) const;

	friend std::ostream& operator<< (std::ostream& os, 
									 const StateEngine& anEngine);
private:
	uint16		itsSequence;
	uint16		itsStepNr;
	bool		itsStateFinished;
	
	// GMT time the current state expires
	time_t	itsStateExpireTime;		

	// (constant) timer values for the states
	time_t	itsCreatePSubsetTime;
    time_t	itsStartupApplTime;
	time_t	itsDefineCmdTime;
	time_t	itsInitCmdTime;
    time_t	itsRunCmdTime;
	time_t	itsPauseCmdTime;
	time_t	itsReleaseCmdTime;
	time_t	itsRecoverCmdTime;
	time_t	itsSnapshotCmdTime;
	time_t	itsReinitCmdTime;
	time_t	itsInfoCmdTime;
	time_t	itsQuitCmdTime;
	time_t	itsKillApplTime;
};

inline void StateEngine::ready()
{
//	LOG_TRACE_STAT ("StateEngine:ready");
	LOG_DEBUG ("StateEngine:ready");
	itsStateFinished   = true;
	itsStateExpireTime = 0;
}

inline bool StateEngine::isStateFinished()
{
	return (itsStateFinished);
}

inline void StateEngine::setStateLifeTime(time_t		anInterval)
{
	itsStateExpireTime = time(0) + anInterval;
}

inline void StateEngine::resetStateExpireTime()
{
	itsStateExpireTime = 0;
}

inline bool StateEngine::IsStateExpired()
{
	return (itsStateExpireTime && (itsStateExpireTime < time(0)));
}

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
