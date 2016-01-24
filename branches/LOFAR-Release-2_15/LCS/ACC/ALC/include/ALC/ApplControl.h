//# ApplControl.h: Defines the I/F of the Application Controller.
//#
//# Copyright (C) 2004
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

#ifndef LOFAR_ALC_APPLCONTROL_H
#define LOFAR_ALC_APPLCONTROL_H

// \file
// Defines the I/F of the Application Controller.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ALC/DH_ApplControl.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{

//# Description of class.
// The ApplControl class defines the interface the Application Controller
// will support. All functions in this class are abstract and need to be
// implemented on both the client- and the server-side.
//
class ApplControl 
{
public:
	// \name Construction and Destruction
	// The \b ApplControl class is an abstract base class that defines the 
	// commands that are implemented for Application Control by the
	// ApplControlClient and ApplControlServer.<br>
	// Since this is an abstract base class the (copy)constructors are private
	// and the destructor is virtual.
	// @{
	// Destructor;
	virtual ~ApplControl() { };
	// @}
	// <br>

	// \name Commands to control the application
	// There are a dozen commands that an ACuser can use to control the
	// processes of an application. Although these command do a lot of different
	// things they have some arguments and return values in common:
	//
	// <b> Immediate commands </b><br>
	// The scheduleTime parameter used in all commands may be set to 0 to 
	// indicate immediate execution. The Application Controller will instruct
	// the processes to execute the command and returns a message containing the
	// result of the execution. This will result in the return value of the
	// command sent.
	//
	// Return values for \b immediate commands: 
	//	- True  - Command executed succesfully
	//	- False - Command could not be executed
	// 
	// <b> Future commands </b><br>
	// When the scheduleTime lays in the future the Application Controller will
	// schedule the command and first return a result message for the scheduling
	// of the command.
	//
	// Return values for \b scheduled commands:
	//	- True  - Command is scheduled succesfully
	//	- False - Command could not be scheduled.
	// 
	// When the scheduling of the command was succesfull the command will be 
	// executed some time later. The AC will then send a result message 
	// containing the execution result. The programmer needs some extra actions
	// to use this result (see ApplControlClient documentation).
	// 
	// @{
	
	// During the \c boot command the machines on which the processes are going
	// to run are powered up. The \c configID argument refers to a parameterset 
	// that contains the runtime parameters for all application processes
	// including the metadata for the application.<br>
	// The Application Controller constructs for each process its own 
	// parameterset.
	virtual bool	boot 	 (const time_t		scheduleTime,
							  const string&		configID) 	  const = 0;

	// The \c define command starts up all application processes. The processes
	// will check their parameterset and will define their communication
	// structure.
	virtual bool	define 	 (const time_t		scheduleTime) const = 0;

	// During the \c init command the process will try to connect to each other
	// and they will allocate their buffers. If the \c init state is reached
	// succesfully the \e application is ready for dataprocesssing (or whatever
	// task the application has).
	virtual bool	init 	 (const time_t		scheduleTime) const = 0;

	// During the \c run phase the application does the work it is designed 
	// for. The run phase stays active until another command is send.
	virtual bool	run 	 (const time_t		scheduleTime) const = 0;

	// With the \c pause command the processes will stop their run phase and
	// start waiting for another command. The \c condition argument contains
	// the condition the processes should use for ending the run phase. This 
	// condition is a key-value pair that can eg. contain a timestamp or a 
	// number of a datasample.<br>
	// The \c waitTime argument tells the Application Controller how long it
	// should wait for the result of the processes. Since the condition 
	// determines when the run phase ends it is not possible to use a default 
	// value for this command as is used for all other commands.
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const time_t		waitTime,
							  const	string&		condition) 	  const = 0;

	// During the \c release command the application has to free all used 
	// resources and it may disconnect from several services. When the
	// application has reached the \c release state it is in the same state
	// as after executing the \c init  command.
	virtual bool	release	 (const time_t		scheduleTime) const = 0;

	// \c Quit stops all application processes and cleans up the parameterset
	// of these processes. After reporting the result of this command to the
	// ACuser the Application Controller starts a countdown timer to powerdown
	// the nodes. When the \c shutdown command is not received within this time
	// the Application Controller will execute it by itself.
	virtual bool	quit  	 (const time_t		scheduleTime) const = 0;

	// \c Shutdown will powerdown the machines on which the application was
	// running. This will only be done ofcourse if the node-manager knows that
	// there are no other programs running on the machines.<br>
	// After the powerdown of the machines the Application Controller 
	// terminates.
	virtual bool	shutdown (const time_t		scheduleTime) const = 0;

	// With the \c snapshot command the application can be instructed to save
	// itself in a database so that on a later moment in time it will be able
	// to recreate itself in the same state and continue the calculation it
	// was doing.<br>
	// The \c destination argument contains the database info the application
	// must use to save itself.
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination)  const = 0;

	// \c Recover reconstructs the processes as they were saved some time
	// earlier. The \c source argument contains the database info the 
	// application must use to find the information it needs.
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source) 	  const = 0;


	// With \c reinit the processes will receive a new parameterfile that they
	// must use to reinitialize themselves. The parametersets for the 
	// application processes are constructed from the parameterset referred to
	// by the \c configID argument.
	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID)	  const = 0;

	// with the \c cancelCmdQueue command the AC user can instruct the 
	// Application Controller to cancel all command on the command stack.
	// Note: This command has NO effect on the command that is currently
	// running, is it handled intern in the ApplicationController.
	virtual bool	cancelCmdQueue 	 () const = 0;

	// @}

	// \par
	// \name Exchanging information
	// Although it is not implemented yet, there is a way defined through which
	// the ACuser and the Application Controller exchange information. With
	// the \c askInfo command the ACuser can send a key(-value) list to the
	// Application Controller to ask for the values of these keys.<br>
	// If the ACuser uses the asyncClient the Application Controller will be
	// able to do the same towards the ACuser. See the ApplControlClient 
	// documentation for more info.

	// @{

	// Defines a generic way to exchange info between client and server. The
	// \c keylist argument must be of the format of key-values lists. The 
	// Application Controller will try to fill in the corresponding values.
	virtual string	askInfo   (const string& 	keylist) const = 0;
	// @}

protected:
	// Not default constructable
	ApplControl() {};

	// Copying is not allowed
	ApplControl(const ApplControl& that);

	// Copying is not allowed
	ApplControl& 	operator=(const ApplControl& that);
};


// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
