//#  ApplCtrlFunctions.h: Implements the service I/F of the Application Controller.
//#
//#  Copyright (C) 2004
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
//#  Abstract:
//#	 This class implements the client API for managing an Application 
//#  Controller. 
//#
//#  $Id$

#ifndef ACC_APPLCTRLFUNCTIONS_H
#define ACC_APPLCTRLFUNCTIONS_H

#include <lofar_config.h>

//# Includes

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;

//# Description of class.
// The ApplControl class implements the service the Application Controller
// will support.
//
class ApplCtrlFunctions 
{
public:
	typedef bool (*Par0Func) 	();
	typedef void (*PingFunc) 	();
	typedef bool (*Par1Func)	(const time_t time);
	typedef bool (*Par2Func)	(const time_t time, const string& options);
	typedef string (*InfoFunc)  (const string& keylist);

	ApplCtrlFunctions(Par2Func		theBootFunc,
					  Par2Func		theDefineFunc,
					  Par1Func		theInitFunc,
					  Par1Func		theRunFunc,
					  Par2Func		thePauseFunc,
					  Par0Func		theQuitFunc,
					  Par2Func		theSnapshotFunc,
					  Par2Func		theRecoverFunc,
					  Par2Func		theReinitFunc,
					  PingFunc		thePingFunc,
					  InfoFunc		theInfoFunc);
	~ApplCtrlFunctions() {};

private:
	bool	(*boot) 	 (const time_t		scheduleTime,
						  const string&		configID);
	bool	(*define) 	 (const time_t		scheduleTime,
						  const string&		configID);
	bool	(*init)  	 (const time_t		scheduleTime);
	bool	(*run)  	 (const time_t		scheduleTime);
	bool	(*pause)  	 (const time_t		scheduleTime,
						  const string&		condition);
	bool	(*quit)		 ();
	bool	(*snapshot)  (const time_t		scheduleTime,
						  const string&		destination);
	bool	(*recover)   (const time_t		scheduleTime,
						  const string&		source);
	bool	(*reinit)	 (const time_t		scheduleTime,
						  const string&		configFile);
	void	(*ping)		 ();

	string	(*supplyInfo)(const string& 	keylist);

	void	(*handleAckMessage)(void);

	friend class ApplControlServer;
	ApplCtrlFunctions() {};
};

inline ApplCtrlFunctions::ApplCtrlFunctions(Par2Func	theBootFunc,
											Par2Func	theDefineFunc,
											Par1Func	theInitFunc,
											Par1Func	theRunFunc,
											Par2Func	thePauseFunc,
											Par0Func	theQuitFunc,
											Par2Func	theSnapshotFunc,
											Par2Func	theRecoverFunc,
											Par2Func	theReinitFunc,
											PingFunc	thePingFunc,
											InfoFunc	theInfoFunc) :
	boot	  (theBootFunc),
	define	  (theDefineFunc),
	init	  (theInitFunc),
	run		  (theRunFunc),
	pause	  (thePauseFunc),
	quit	  (theQuitFunc),
	snapshot  (theSnapshotFunc),
	recover	  (theRecoverFunc),
	reinit 	  (theReinitFunc),
	ping	  (thePingFunc),
	supplyInfo(theInfoFunc)
{
}

} // namespace ACC
} // namespace LOFAR

#endif
