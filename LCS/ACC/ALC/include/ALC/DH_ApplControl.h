//#  DH_ApplControl.h: Implements the Application Controller command protocol.
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
//#	 Abstract:
//#	 This class implements the command protocol between an application manager
//#	 (MAC for instance) and an Application Controller (=ACC package).
//#	 The AM has the client role, the AC the server role.
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
enum ACCmd { CmdBoot = 1, CmdQuit, 
				CmdDefine, CmdInit,
				CmdPause, CmdRun, 
				CmdSnapshot, CmdRecover, 
				CmdReinit, CmdInfo, CmdAnswer,
				CmdPing, CmdReport,
				CmdResult = 99, CmdAsync
};


//# Description of class.
// The ApplControl class implements the C/S communication of the service the
// the Application Controller supports.
//
class DH_ApplControl : public DataHolder
{
public:
	// Constructor
	DH_ApplControl();

	// Destructor
	virtual ~DH_ApplControl();

	// Copying is allowed.
	DH_ApplControl(const DH_ApplControl& that);
	DH_ApplControl*		clone() const;

	// Redefines the preprocess function.
	virtual void 	preprocess();

	// The real data-accessor functions
	void	setScheduleTime	(const time_t		theTime);
	void	setCommand		(const ACCmd		theCmd);
	void	setOptions		(const string&		theOptions);
	void	setResult		(const int	 		theResult);

	time_t	getScheduleTime	() const;
	ACCmd	getCommand		() const;
	string	getOptions		() ;
	uint16	getResult		() const;

private:
	// forbit default construction and assignment operator
	DH_ApplControl& 	operator=(const DH_ApplControl& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	// fields transferred between the server and the client
	uint16		*itsVersionNumber;
	int16		*itsCommand;
	time_t		*itsScheduleTime;
	uint16		*itsResult;

	// local administration
	in_addr_t		itsServerIP;
	in_port_t		itsServerPort;

};

// The real data-accessor functions
inline void	DH_ApplControl::setScheduleTime	(const time_t		theTime)
{
	*itsScheduleTime = theTime;
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

inline void	DH_ApplControl::setResult		(const int			theResult)
{
	*itsResult = theResult;
}


inline time_t	DH_ApplControl::getScheduleTime	() const
{
	// no version support necc. yet.
	return (*itsScheduleTime);
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

inline uint16	DH_ApplControl::getResult		() const
{
	// no version support necc. yet.
	return (*itsResult);
}


} // namespace ACC
} // namespace LOFAR

#endif
