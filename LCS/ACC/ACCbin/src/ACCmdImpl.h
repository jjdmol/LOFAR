//#  ACCmdImpl.h: the implementation of the AC commands
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

#ifndef ACC_ACCMDIMPL_H
#define ACC_ACCMDIMPL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <ACC/ApplControl.h>
#include <ACC/ParameterSet.h>

namespace LOFAR {
  namespace ACC {

// Description of class.
class ACCmdImpl : public ApplControl
{
public:
	// Default constructable
	ACCmdImpl(ParameterSet*		aPS);

	// Destructor
	virtual ~ACCmdImpl();

	// Commands to control the application
	virtual bool	boot 	 (const time_t		scheduleTime,
							  const string&		configID);
	virtual bool	define 	 (const time_t		scheduleTime);
	virtual bool	init 	 (const time_t		scheduleTime);
	virtual bool	run 	 (const time_t		scheduleTime);
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const time_t		waitTime,
							  const	string&		condition);
	virtual bool	quit  	 (const time_t		scheduleTime);
	virtual bool	shutdown (const time_t		scheduleTime);
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination);
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source);

	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID);
	virtual bool	replace	 (const time_t		scheduleTime,
							  const string&		processList,
							  const string&		nodeList,
							  const string&		configID);

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

	// Command for handling the Cmd expire timer.
	void setCmdLifeTime  (time_t		aInterval);
	void resetCmdExpireTime();
	bool IsCmdExpired      ();

private:
	// Copying is not allowed
	ACCmdImpl();
	ACCmdImpl(const ACCmdImpl& that);
	ACCmdImpl& 	operator=(const ACCmdImpl& that);

	time_t		itsCmdExpireTime;

	mutable time_t		itsDefineLifeTime;
	time_t		itsInitLifeTime;
	time_t		itsRunLifeTime;
	time_t		itsPauseLifeTime;
	time_t		itsQuitLifeTime;
	time_t		itsSnapshotLifeTime;
	time_t		itsRecoverLifeTime;
	time_t		itsReinitLifeTime;
};

inline void ACCmdImpl::setCmdLifeTime  (time_t		anInterval)
{
	itsCmdExpireTime = time(0) + anInterval;
}

inline void ACCmdImpl::resetCmdExpireTime()
{
	itsCmdExpireTime = 0;
}

inline bool ACCmdImpl::IsCmdExpired      ()
{
	return (itsCmdExpireTime && (itsCmdExpireTime < time(0)));
}

  } // namespace ACC
} // namespace LOFAR

#endif
