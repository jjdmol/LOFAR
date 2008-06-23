//#  ProcessControl.h: Defines the I/F of Process Control.
//#
//#  Copyright (C) 2004-2008
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

#ifndef LOFAR_PLC_PROCESSCONTROL_H
#define LOFAR_PLC_PROCESSCONTROL_H

// \file
// Defines the interface of Process Control commands.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <PLC/DH_ProcControl.h>
#include <Common/lofar_tribool.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

// \addtogroup PLC
// @{

//# Description of class.
// The ProcessControl class defines the command interface that can be used
// to control the processes of an application.<br>
// All functions in this class are abstract and need to be
// implemented on both the client and the server-side. On the client side
// the implementation will only forward the function-call, on the server
// side (= the application process) the real implementation must be done.
//
class ProcessControl 
{
public:
	// Destructor
	virtual ~ProcessControl() { };

	// \name Command to control the processes.
	// There are a dozen commands that can be sent to a application process
	// to control its flow. The return values for these command are:<br>
	// - True   - Command executed succesfully.
	// - False  - Command could not be executed.
	// 
	// @{

	// During the \c define state the process check the contents of the
	// ParameterSet it received during start-up. When everthing seems ok the
	// process constructs the communication channels for exchanging data
	// with the other processes. The connection are NOT made in the stage.
	virtual tribool	define 	 ()  = 0;

	// When a process receives an \c init command it allocates the buffers it
	// needs an makes the connections with the other processes. When the
	// process succeeds in this it is ready for dataprocessing (or whatever
	// task the process has).
	virtual tribool	init 	 ()  = 0;

	// During the \c run phase the process does the work it is designed for.
	// The run phase stays active until another command is send.
	virtual tribool	run 	 ()  = 0;

	// With the \c pause command the process stops its run phase and starts
	// waiting for another command. The \c condition argument contains the
	// contition the process should use for ending the run phase. This
	// condition is a key-value pair that can eg. contain a timestamp or a
	// number of a datasample.
	virtual tribool	pause  	 (const	string&		condition) 	   = 0;

	// The \c release command always comes before the quit command. The process
	// can free the allocated buffers and might disconnect from some services
	// already. After the \c release command the process is in the same state as
	// after the \c define command.
	virtual tribool	release	 ()	   = 0;

	// \c Quit stops the process.
	// The process \b must call \c unregisterAtAC at ProcControlServer during 
	// the execution of this command to pass the final results to the 
	// Application Controller.
	virtual tribool	quit  	 ()  = 0;

	// With the \c snapshot command the process is instructed to save itself
	// in a database is such a way that on another moment in time it can
	// be reconstructed and can continue it task.<br>
	// The \c destination argument contains database info the process
	// must use to save itself.
	virtual tribool	snapshot (const string&		destination)   = 0;

	// \c Recover reconstructs the process as it was saved some time earlier.
	// The \c source argument contains the database info the process must use
	// to find the information it needs.
	virtual tribool	recover  (const string&		source) 	   = 0;

	// With \c reinit the process receives a new parameterset that it must use
	// to reinitialize itself.
	virtual tribool	reinit	 (const string&		configID)	   = 0;
	// @}

	// Define a generic way to exchange info between client and server.
	virtual string	askInfo   (const string& 	keylist)  = 0;

        // Returns whether we're currently in the run state or not.
	bool inRunState() const	{ return itsRunState; }

protected:
	// Default constructor
	ProcessControl()	: itsRunState(false)	{}

        // Routines for handling the run state.
        // @{
	void setRunState()	{ itsRunState = true; }
	void clearRunState()	{ itsRunState = false; }
        // @}

	// The proxy class must be able to set/clear the run state.
	friend class ProcCtrlProxy;

private:
	// Copying is also not allowed
	ProcessControl(const ProcessControl& that);
	ProcessControl& 	operator=(const ProcessControl& that);

        // Run-state flag.
        bool	itsRunState;
};

// @} addgroup


    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
