//#  DH_ProcessControl.h: DataHolder for Process Control commands.
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

#ifndef ACC_DH_PROCESSCONTROL_H
#define ACC_DH_PROCESSCONTROL_H

#include <lofar_config.h>

//# Includes
#include <sys/time.h>
#include <sys/socket.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Transport/DataHolder.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;

// Make list of supported commands.
enum PCCmd { PCCmdNone = 0, 
				PCCmdStart = 100, PCCmdQuit, 
				PCCmdDefine, PCCmdInit,
				PCCmdHalt, PCCmdRun,
				PCCmdSnapshot, PCCmdRecover, 
				PCCmdInfo, PCCmdAnswer,
				PCCmdReport, PCCmdAsync,
				PCCmdResult = 199
//				PCCmdCheckParSet, PCCmdLoadParSet
};


//# Description of class.
// The DH_ProcessControl class is responsible for packing and unpacking
// Process Control commands.
//
class DH_ProcessControl : public DataHolder
{
public:
	// Constructor
	DH_ProcessControl();

	// Destructor
	virtual ~DH_ProcessControl();

	// Copying only by cloning
	DH_ProcessControl*		clone() const;

	// Redefines the preprocess function.
	virtual void 	preprocess();

	// The real data-accessor functions
	void	setCommand		(const PCCmd		theCmd);
	void	setScheduleTime	(const time_t		theTime);
	void	setWaitTime	    (const time_t		theWaitTime);
	void	setOptions		(const string&		theOptions);
	void	setProcList		(const string&		theProcList);
	void	setNodeList		(const string&		theNodeList);
	void	setResult		(const uint16		theResult);

	PCCmd	getCommand		() const;
	time_t	getScheduleTime () const;
	time_t	getWaitTime     () const;
	string	getOptions		() ;
	string	getProcList		() ;
	string	getNodeList		() ;
	uint16	getResult		() const;

private:
	// forbit default construction and assignment operator
	DH_ProcessControl& 	operator=(const DH_ProcessControl& that);
	DH_ProcessControl(const DH_ProcessControl& that);

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
inline void	DH_ProcessControl::setScheduleTime (const time_t theTime)
{
	*itsScheduleTime = theTime;
}

inline void	DH_ProcessControl::setWaitTime (const time_t theWaitTime)
{
	*itsWaitTime = theWaitTime;
}

inline void	DH_ProcessControl::setCommand (const PCCmd theCmd)
{
	*itsCommand = theCmd;
}

inline void	DH_ProcessControl::setOptions (const string& theOptions)
{
	BlobOStream&	bos = createExtraBlob();	// attached to dataholder
	bos << theOptions;
}

inline void	DH_ProcessControl::setProcList (const string& theProcList)
{
	// TODO
}

inline void	DH_ProcessControl::setNodeList (const string& theNodeList)
{
	// TODO
}

inline void	DH_ProcessControl::setResult (const uint16 theResult)
{
	*itsResult = theResult;
}


inline time_t	DH_ProcessControl::getScheduleTime () const
{
	// no version support necc. yet.
	return (*itsScheduleTime);
}

inline time_t	DH_ProcessControl::getWaitTime () const
{
	// no version support necc. yet.
	return (*itsWaitTime);
}

inline PCCmd	DH_ProcessControl::getCommand () const
{
	// no version support necc. yet.
	return static_cast<PCCmd>(*itsCommand);
}

inline string	DH_ProcessControl::getOptions ()
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

inline string	DH_ProcessControl::getProcList ()
{
	return ("TODO: proclist");
}

inline string	DH_ProcessControl::getNodeList ()
{
	return ("TODO: nodelist");
}

inline uint16	DH_ProcessControl::getResult () const
{
	// no version support necc. yet.
	return (*itsResult);
}

} // namespace ACC
} // namespace LOFAR

#endif
