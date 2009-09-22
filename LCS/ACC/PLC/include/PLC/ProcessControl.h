//# ProcessControl.h: Defines the I/F of Process Control.
//#
//# Copyright (C) 2004-2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_PLC_PROCESSCONTROL_H
#define LOFAR_PLC_PROCESSCONTROL_H

// \file
// Defines the interface of Process Control commands.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <PLC/DH_ProcControl.h>
#include <Common/lofar_tribool.h>

namespace LOFAR {
  class ParameterSet;
  namespace ACC {
    namespace PLC {

//# Forward declarations
class ProcCtrlProxy;

// \addtogroup PLC
// @{

//# Description of class.
// The ProcessControl class defines the command interface that can be used
// to control the processes of an application.
// 
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

protected:
	// Default constructor
	explicit ProcessControl(const string&	theProcessID): 
		itsProcID(theProcessID), itsRunState(false), itsControlProxy(0)	{}

	// \name Commands to control the processes.
	//
	// There are a dozen commands that can be sent to an application
	// process to control its flow.
	//
	// \return \c true : command execution succesfull.
	// \return \c false : command execution failed.
	// \return \c indeterminate : command not supported or implemented.
	// 
	// @{

	// During the \c define state the process check the contents of the
	// ParameterSet it received during start-up. When everthing seems ok
	// the process constructs the communication channels for exchanging
	// data with the other processes. The connection are NOT made in the
	// stage.
	virtual tribool	define 	 ()  = 0;

	// When a process receives an \c init command it allocates the buffers
	// it needs an makes the connections with the other processes. When
	// the process succeeds in this it is ready for dataprocessing (or
	// whatever task the process has).
	virtual tribool	init 	 ()  = 0;

	// During the \c run phase the process does the work it is designed
	// for.  The run phase stays active until another command is send.
	virtual tribool	run 	 ()  = 0;

	// With the \c pause command the process stops its run phase and
	// starts waiting for another command. The \c condition argument
	// contains the contition the process should use for ending the run
	// phase. This condition is a key-value pair that can eg. contain a
	// timestamp or a number of a datasample.
	virtual tribool	pause  	 (const	string&		condition) 	   = 0;

	// The \c release command always comes before the quit command. The
	// process can free the allocated buffers and might disconnect from
	// some services already. After the \c release command the process is
	// in the same state as after the \c define command.
	virtual tribool	release	 ()	   = 0;

	// \c Quit stops the process. When the process is running under
	// control of ACC, it \b must call \c unregisterAtAC at
	// ProcControlServer during the execution of this command to pass the
	// final results to the Application Controller. Note that this is now
	// handled by the ProcCtrlRemote class.
	virtual tribool	quit  	 ()  = 0;

	// With the \c snapshot command the process is instructed to save
	// itself in a database is such a way that on another moment in time
	// it can be reconstructed and can continue it task.<br> The \c
	// destination argument contains database info the process must use to
	// save itself.
	virtual tribool	snapshot (const string&		destination)   = 0;

	// \c Recover reconstructs the process as it was saved some time
	// earlier.  The \c source argument contains the database info the
	// process must use to find the information it needs.
	virtual tribool	recover  (const string&		source) 	   = 0;

	// With \c reinit the process receives a new parameterset that it must
	// use to reinitialize itself.
	virtual tribool	reinit	 (const string&		configID)	   = 0;
	// @}

	// Define a generic way to ask questions to the server.
	virtual string	askInfo   (const string& 	keylist)  = 0;

	// Define a generic way to send metadata to the server.
	void	sendResultParameters (const string& 		keylist);
	void	sendResultParameters (const ParameterSet& 	aParSet);

    // Routines for handling the run state.
    // @{
	void setRunState()		{ itsRunState = true; }
	void clearRunState()	{ itsRunState = false; }
	bool inRunState() const	{ return itsRunState; }
    // @}

	// My unique name (use to communicate with the server).
	string		itsProcID;

	// The proxy class must be able to set/clear the run state.
	friend class ProcCtrlProxy;

private:
	// Copying is also not allowed
	ProcessControl(const ProcessControl& that);
	ProcessControl& 	operator=(const ProcessControl& that);

	// Run-state flag.
	bool				itsRunState;

	// Pointer to controlproxy for passing metadata from the process to the server
	ProcCtrlProxy*		itsControlProxy;

};

// @} addgroup


    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
