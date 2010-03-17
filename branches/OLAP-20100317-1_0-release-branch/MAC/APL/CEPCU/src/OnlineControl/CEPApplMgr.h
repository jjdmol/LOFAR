//#  CEPApplMgr.h: factory class for Virtual Backends.
//#
//#  Copyright (C) 2002-2005
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

#ifndef CEPAPPLMGR_H
#define CEPAPPLMGR_H

//# Includes
#include <ALC/ACAsyncClient.h>
#include <GCF/TM/GCF_Handler.h>
#include <APL/APLCommon/CTState.h>

//# local includes
//# Common Includes

// forward declaration

namespace LOFAR {  
  using APLCommon::CTState;
  namespace CEPCU {

// The CEPApplMgrInterface is an abstract baseclass to define the interface
// the CEPApplMgr will call for passing the results of ACC back to the controller.
class CEPApplMgrInterface
{
public:
	virtual ~CEPApplMgrInterface() {}

	virtual void	appSetStateResult  (const string&			procName, 
									    CTState::CTstateNr     	newState, 
									    uint16					result) = 0;
	virtual string  appSupplyInfo	   (const string&			procName, 
										const string&			keyList) = 0;
	virtual void    appSupplyInfoAnswer(const string&			procName, 
										const string&			answer) = 0;    
protected:
	CEPApplMgrInterface() {}

private:
	// protected copy constructor
	CEPApplMgrInterface(const CEPApplMgrInterface&);
	// protected assignment operator
	CEPApplMgrInterface& operator=(const CEPApplMgrInterface&);
};



// The CEPApplMgr class acts as an ACClient for the OnlineController but it
// also an active component because is is also inherited from GCFHandler.
// The GCFHandler-workproc will poll the ACC connection for incomming msgs.
class CEPApplMgr : public ACC::ALC::ACClientFunctions,
						  GCF::TM::GCFHandler
{
public:
	CEPApplMgr(CEPApplMgrInterface& 	interface, 
			   const string& 			appName, 
			   uint32					expectedRuntime,
			   const string& 			acdHost,
			   const string&			paramFile);
	virtual ~CEPApplMgr();

	// method used by the OnlineController to initiate a new command
	void sendCommand (CTState::CTstateNr	newState, const string&		options);

	// methods may be called from specialized CEPApplMgrInterface
	bool  boot     (const time_t    scheduleTime,
					const string&   configID);
	bool  define   (const time_t    scheduleTime)  const;
	bool  init     (const time_t    scheduleTime)  const;
	bool  run      (const time_t    scheduleTime)  const;
	bool  pause    (const time_t    scheduleTime,
					const time_t    maxWaitTime,
					const string&   condition)     const;
	bool  release  (const time_t    scheduleTime)  const;
	bool  quit     (const time_t    scheduleTime)  const;
	bool  shutdown (const time_t    scheduleTime)  const;
	bool  snapshot (const time_t    scheduleTime,
					const string&   destination)   const;
	bool  recover  (const time_t    scheduleTime,
					const string&   source)        const;
	bool  reinit   (const time_t    scheduleTime,
					const string&   configID)      const;
	string  askInfo (const string&  keylist)       const;    

	bool  cancelCmdQueue ()                        const;    

	const string&	getName()					   const;

protected:
	// protected copy constructor
	CEPApplMgr(const CEPApplMgr&);
	// protected assignment operator
	CEPApplMgr& operator=(const CEPApplMgr&);

private: 
	// implemenation of abstract GCFHandler methods
	friend class GCF::TM::GCFHandler;
	void workProc();    
	void stop();

	// implemenation of abstract ACClientFunctions methods
	friend class ACC::ALC::ACClientFunctions;
	void  handleAckMsg      (ACC::ALC::ACCmd    cmd, 
							 uint16        result,
							 const string& info);
	void  handleAnswerMsg   (const string& answer);
	string  supplyInfoFunc  (const string& keyList);

	// --- datamembers ---
	string                          itsProcName;		// my name
	string                          itsParamFile;		// name of paramfile
	CEPApplMgrInterface& 			itsCAMInterface;	// link to OnlineController
	ACC::ALC::ACAsyncClient         itsACclient;		// link to ACC controller
	uint16							itsReqState;		// requested state
	uint16							itsCurState;		// reached state
	bool                            itsContinuePoll;	// for workProc
//	ACC::ALC::ACCmd                 itsLastOkCmd;
};

inline const string&	CEPApplMgr::getName() const
{
	return (itsProcName);
}

inline bool  CEPApplMgr::boot(const time_t    scheduleTime,
							  const string&   configID)      
{
	itsContinuePoll = true;  
	return (itsACclient.boot(scheduleTime, configID));
}
                                              
inline bool  CEPApplMgr::define(const time_t    scheduleTime)  const
{
	return (itsACclient.define(scheduleTime));
}
 
inline bool  CEPApplMgr::init(const time_t    scheduleTime)  const
{
	return (itsACclient.init(scheduleTime));
}
 
inline bool  CEPApplMgr::run(const time_t    scheduleTime)  const
{
	return (itsACclient.run(scheduleTime));
}
 
inline bool  CEPApplMgr::pause (const time_t    scheduleTime,
								const time_t    maxWaitTime,
								const string&   condition)     const
{
	return (itsACclient.pause(scheduleTime, maxWaitTime, condition));
}
 
inline bool  CEPApplMgr::release (const time_t    scheduleTime)  const
{
	return (itsACclient.release(scheduleTime));
}
 
inline bool  CEPApplMgr::quit (const time_t    scheduleTime)  const
{
	return (itsACclient.quit(scheduleTime));
}
 
inline bool  CEPApplMgr::shutdown (const time_t    scheduleTime)  const
{
	return (itsACclient.shutdown(scheduleTime));
}
 
inline bool  CEPApplMgr::snapshot (const time_t    scheduleTime,
								   const string&   destination)   const
{
	return (itsACclient.snapshot(scheduleTime, destination));
}
 
inline bool  CEPApplMgr::recover  (const time_t    scheduleTime,
								   const string&   source)        const
{
	return (itsACclient.recover(scheduleTime, source));
}
 
inline bool  CEPApplMgr::reinit   (const time_t    scheduleTime,
								   const string&   configID)      const
{
	return (itsACclient.reinit(scheduleTime, configID));
}
 
inline bool  CEPApplMgr::cancelCmdQueue ()                       const
{
	return itsACclient.cancelCmdQueue();
}
 
inline void CEPApplMgr::stop()
{ }

  } // namespace CEPCU
} // namespace LOFAR
#endif
