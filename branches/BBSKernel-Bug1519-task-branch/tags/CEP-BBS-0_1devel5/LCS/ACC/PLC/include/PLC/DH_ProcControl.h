//#  DH_ProcControl.h: DataHolder for Process Control commands.
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

#ifndef LOFAR_PLC_DH_PROCCONTROL_H
#define LOFAR_PLC_DH_PROCCONTROL_H

// \file
// DataHolder for Process Control commands.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <sys/time.h>
#include <sys/socket.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Transport/DataHolder.h>
#include <PLC/PCCmd.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {
// \addtogroup PLC
// @{

//# Description of class.
// The DH_ProcControl class is responsible for packing and unpacking
// Process Control commands.
class DH_ProcControl : public DataHolder
{
public:
	// Constructor
	DH_ProcControl();
	DH_ProcControl(const PCCmd	aCommand);

	// Destructor
	virtual ~DH_ProcControl();

	// \name libTransport methods
	// @{

	// Copying only by cloning
	DH_ProcControl*		clone() const;

	// Implements the init function.
	virtual void 	init();
	// @}

	// \name Data-accessor methods
	// @{

	void	setCommand		(const PCCmd		theCmd);
	void	setOptions		(const string&		theOptions);
	void	setResult		(const uint16		theResult);

	PCCmd	getCommand		() const;
	string	getOptions		() ;
	uint16	getResult		() const;
	// @}

private:
	// Copying is not allowed this way.
	DH_ProcControl& 	operator=(const DH_ProcControl& that);

	// Copying is not allowed this way.
	DH_ProcControl(const DH_ProcControl& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	//# --- DataMembers ---
	uint16		*itsVersionNumber;
	int16		*itsCommand;
	uint16		*itsResult;
};

//#
//# setCommand(command)
//#
inline void	DH_ProcControl::setCommand (const PCCmd theCmd)
{
	*itsCommand = theCmd;
}

//#
//# setOptions(options)
//#
inline void	DH_ProcControl::setOptions (const string& theOptions)
{
	BlobOStream&	bos = createExtraBlob();	//# attached to dataholder
	bos << theOptions;
}

//#
//# setResult(result)
//#
inline void	DH_ProcControl::setResult (const uint16 theResult)
{
	*itsResult = theResult;
}

//#
//# getCommand()
//#
inline PCCmd	DH_ProcControl::getCommand () const
{
	//# no version support necc. yet.
	return static_cast<PCCmd>(*itsCommand);
}

//#
//# getOptions()
//#
inline string	DH_ProcControl::getOptions ()
{
	//# no version support necc. yet.
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

//#
//# getResult()
//#
inline uint16	DH_ProcControl::getResult () const
{
	//# no version support necc. yet.
	return (*itsResult);
}


// @} addgroup
    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
