//#  DH_ProcessControl.h: Implements the Process Controller command protocol.
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
//#	 This class implements the command protocol between an application 
//#  Controller and the Application processes. The division of the roles
//#  (who is server, who is client) is done during runtime.
//#
//#  $Id$

#ifndef ACC_DH_PROCESSCONTROL_H
#define ACC_DH_PROCESSCONTROL_H

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
enum PCCmd { PCCmdNone = 0, 
				PCCmdStart = 100, PCCmdQuit, 
				PCCmdHalt, PCCmdResume, 
				PCCmdSnapshot, PCCmdRecover, 
				PCCmdDefine, PCCmdCheckParSet, PCCmdLoadParSet
};


//# Description of class.
// The ProcessControl class implements the communication protocol between the
// Application Controller and the Application processes.
//
class DH_ProcessControl : public DataHolder
{
public:
	// Constructor
	explicit DH_ProcessControl(const string	hostID);

	// Destructor
	virtual ~DH_ProcessControl();

	// Copying is allowed.
	DH_ProcessControl(const DH_ProcessControl& that);
	DH_ProcessControl*		clone() const;

	// Redefines the preprocess function.
	virtual void 	preprocess();

	// The real data-accessor functions
	void	setCommand		(const PCCmd		theCmd);
	void	setOptions		(const string		theOptions);
	void	setResult		(const int32		theResult);

	PCCmd	getCommand		();
	string	getOptions		();
	int32	getResult		();

private:
	// forbit default construction and assignment operator
	DH_ProcessControl();
	DH_ProcessControl& 	operator=(const DH_ProcessControl& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	// fields transferred between the server and the client
	int32		*itsVersionNumber;
	PCCmd		*itsCommand;
	string		*itsOptions;
	int32		*itsResult;

	// local administration
	in_addr_t	itsServerIP;
	in_port_t	itsServerPort;

};


} // namespace ACC
} // namespace LOFAR

#endif
