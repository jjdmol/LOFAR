//#  DH_ApplControl.h: DataHolder for Application Control commands.
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
//#  $Id$

#ifndef ACC_DH_APPLCONTROL_H
#define ACC_DH_APPLCONTROL_H

#include <lofar_config.h>

//# Includes
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Transport/DataHolder.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;

// Make list of supported commands.
enum ACCmd { CmdBoot = 1, CmdShutdown, CmdQuit, 
				CmdDefine, CmdInit,
				CmdPause, CmdRun, 
				CmdSnapshot, CmdRecover, 
				CmdReinit, CmdReplace,
				CmdInfo, CmdAnswer,
				CmdReport, CmdAsync,
				CmdResult = 99
};


//# Description of class.
// The DH_ApplControl class is responsible for packing and unpacking
// Application Control commands.
//
class DH_ApplControl : public DataHolder
{
public:
	// Constructor
	DH_ApplControl();

	// Destructor
	virtual ~DH_ApplControl();

	// Copying is allowed.
	DH_ApplControl*		clone() const;

	// Redefines the preprocess function.
	virtual void 	preprocess();

	// The real data-accessor functions
	void	setCommand		(const ACCmd		theCmd);
	void	setScheduleTime	(const time_t		theTime);
	void	setWaitTime		(const time_t		theWaitTime);
	void	setOptions		(const string&		theOptions);
	void	setProcList		(const string&		theProcList);
	void	setNodeList		(const string&		theNodeList);
	void	setResult		(const uint16		theResult);

	ACCmd	getCommand		() const;
	time_t	getScheduleTime	() const;
	time_t	getWaitTime		() const;
	string	getOptions		() ;
	string	getProcList		() ;
	string	getNodeList		() ;
	uint16	getResult		() const;

private:
	// forbit default construction and assignment operator
	DH_ApplControl(const DH_ApplControl& that);
	DH_ApplControl& 	operator=(const DH_ApplControl& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	// fields transferred between the server and the client
	uint16		*itsVersionNumber;
	int16		*itsCommand;
	time_t		*itsScheduleTime;
	time_t		*itsWaitTime;
	uint16		*itsResult;
};

// The real data-accessor functions
inline void	DH_ApplControl::setScheduleTime	(const time_t		theTime)
{
	*itsScheduleTime = theTime;
}

inline void	DH_ApplControl::setWaitTime	(const time_t		theWaitTime)
{
	*itsWaitTime = theWaitTime;
}

inline void	DH_ApplControl::setCommand		(const ACCmd		theCmd)
{
	*itsCommand = theCmd;
}

inline void	DH_ApplControl::setOptions		(const string&		theOptions)
{
	BlobOStream&	bos = createExtraBlob();	// attached to dataholder
	bos << theOptions;
}

inline void	DH_ApplControl::setProcList		(const string&		theProcList)
{
	// TODO
}

inline void	DH_ApplControl::setNodeList		(const string&		theNodeList)
{
	// TODO
}

inline void	DH_ApplControl::setResult		(const uint16			theResult)
{
	*itsResult = theResult;
}


inline time_t	DH_ApplControl::getScheduleTime	() const
{
	// no version support necc. yet.
	return (*itsScheduleTime);
}

inline time_t	DH_ApplControl::getWaitTime	() const
{
	// no version support necc. yet.
	return (*itsWaitTime);
}

inline ACCmd	DH_ApplControl::getCommand		() const
{
	// no version support necc. yet.
	return static_cast<ACCmd>(*itsCommand);
}

inline string	DH_ApplControl::getOptions		()
{
	// no version support necc. yet.
	int32			version;
	bool			found;
	BlobIStream&	bis = getExtraBlob(found, version);
	if (!found) {
		return (string(""));
	}

	string	theOptions;
	bis >> theOptions;
	bis.getEnd();

	return (theOptions);
}

inline string	DH_ApplControl::getProcList		()
{
	return ("TODO: proclist");
}

inline string	DH_ApplControl::getNodeList		()
{
	return ("TODO: nodelist");
}

inline uint16	DH_ApplControl::getResult		() const
{
	// no version support necc. yet.
	return (*itsResult);
}


} // namespace ACC
} // namespace LOFAR

#endif
