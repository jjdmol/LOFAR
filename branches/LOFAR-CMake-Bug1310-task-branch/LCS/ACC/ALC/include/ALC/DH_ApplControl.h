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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ALC_DH_APPLCONTROL_H
#define LOFAR_ALC_DH_APPLCONTROL_H

// \file
// DataHolder for Application Control commands.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Transport/DataHolder.h>
#include <ALC/ACCmd.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{

//# Description of class.
// The DH_ApplControl class is responsible for packing and unpacking
// Application Control commands.
class DH_ApplControl : public DataHolder
{
public:
	// Constructor
	DH_ApplControl();

	// Destructor
	virtual ~DH_ApplControl();

	// \name libTransport methods
	// @{ 

	// Register the fixed size variables to the dataholderblob.
	virtual void 	init();

	// Tries to fill its buffer with new data. Returns \c true is a complete
	// message is received.
	void	unpack();

	// Write the current contents to the network.
	void	pack();

	// The clone function is neccesary to meet the libTransport requirements,
	// it copies everything but the data. Something we never need.
	DH_ApplControl*		clone()        const;
	// @}

	// \name Additional methods
	// @{

	// \c makeDataCopy is the counterpart of clone: it copies the data.
	DH_ApplControl*		makeDataCopy() const;

	// @}

	// \name Data-accessor methods
	// @{

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
	string	getOptions		() const;
	string	getProcList		() const;
	string	getNodeList		() const;
	uint16	getResult		() const;
	// @}

private:
	// Copying is not allowed this way.
	DH_ApplControl(const DH_ApplControl& that);

	// Copying is not allowed this way.
	DH_ApplControl& 	operator=(const DH_ApplControl& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	//# --- DataMembers ---
	uint16		*itsVersionNumber;
	int16		*itsCommand;
	int32		*itsScheduleTime;
	int32		*itsWaitTime;
	uint16		*itsResult;
	string		itsOptions;
	string		itsProcList;
	string		itsNodeList;
};

//#
//# setScheduleTime(GMTtime)
//#
inline void	DH_ApplControl::setScheduleTime	(const time_t		theTime)
{
	*itsScheduleTime = theTime;
}

//#
//# setWaitTime(waittime)
//#
inline void	DH_ApplControl::setWaitTime	(const time_t		theWaitTime)
{
	*itsWaitTime = theWaitTime;
}

//#
//# setCommnad(command)
//#
inline void	DH_ApplControl::setCommand		(const ACCmd		theCmd)
{
	*itsCommand = theCmd;
}

//#
//# setOptions(options)
//#
inline void	DH_ApplControl::setOptions		(const string&		theOptions)
{
	itsOptions = theOptions;
}

//#
//# setProcList(procList)
//#
inline void	DH_ApplControl::setProcList		(const string&		theProcList)
{
	itsProcList = theProcList;
}

//#
//# setNodeList(nodeList)
//#
inline void	DH_ApplControl::setNodeList		(const string&		theNodeList)
{
	itsNodeList = theNodeList;
}

//#
//# setResult(result)
//#
inline void	DH_ApplControl::setResult		(const uint16			theResult)
{
	*itsResult = theResult;
}


//#
//# getScheduleTime()
//#
inline time_t	DH_ApplControl::getScheduleTime	() const
{
	//# no version support necc. yet.
	return (*itsScheduleTime);
}

//#
//# getWaitTime()
//#
inline time_t	DH_ApplControl::getWaitTime	() const
{
	//# no version support necc. yet.
	return (*itsWaitTime);
}

//#
//# getCommand()
//#
inline ACCmd	DH_ApplControl::getCommand		() const
{
	//# no version support necc. yet.
	return static_cast<ACCmd>(*itsCommand);
}

//#
//# getOptions()
//#
inline string	DH_ApplControl::getOptions		() const
{
	return (itsOptions);
}

//#
//# getProcList()
//#
inline string	DH_ApplControl::getProcList		() const
{
	return (itsProcList);
}

//#
//# getNodeList()
//#
inline string	DH_ApplControl::getNodeList		() const
{
	return (itsNodeList);
}

//#
//# getResult()
//#
inline uint16	DH_ApplControl::getResult		() const
{
	//# no version support necc. yet.
	return (*itsResult);
}

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
