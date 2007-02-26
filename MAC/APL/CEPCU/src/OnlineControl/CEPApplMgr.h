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

//# local includes
//# Common Includes

// forward declaration

namespace LOFAR {  
  namespace CEPCU {

class CEPApplMgrInterface
{
public:
	virtual ~CEPApplMgrInterface() {}

	virtual void    appBooted			(const string& procName, uint16 result) = 0;
	virtual void    appDefined			(const string& procName, uint16 result) = 0;
	virtual void    appInitialized		(const string& procName, uint16 result) = 0;
	virtual void    appRunDone			(const string& procName, uint16 result) = 0;
	virtual void    appPaused			(const string& procName, uint16 result) = 0;
	virtual void    appQuitDone			(const string& procName, uint16 result) = 0;
	virtual void    appSnapshotDone		(const string& procName, uint16 result) = 0;
	virtual void    appRecovered		(const string& procName, uint16 result) = 0;
	virtual void    appReinitialized	(const string& procName, uint16 result) = 0;
	virtual void    appReplaced			(const string& procName, uint16 result) = 0;
	virtual string  appSupplyInfo		(const string& procName, const string& keyList) = 0;
	virtual void    appSupplyInfoAnswer (const string& procName, const string& answer) = 0;    

protected:
	CEPApplMgrInterface() {}

private:
	// protected copy constructor
	CEPApplMgrInterface(const CEPApplMgrInterface&);
	// protected assignment operator
	CEPApplMgrInterface& operator=(const CEPApplMgrInterface&);
};



class CEPApplMgr : public ACC::ALC::ACClientFunctions,
                                     GCF::TM::GCFHandler
{
public:
	CEPApplMgr(CEPApplMgrInterface& interface, const string& appName);
	virtual ~CEPApplMgr();

	// methods may be called from specialized CEPApplMgrInterface
	bool  boot     (const time_t    scheduleTime,
					const string&   configID);
	bool  define   (const time_t    scheduleTime)  const;
	bool  init     (const time_t    scheduleTime)  const;
	bool  run      (const time_t    scheduleTime)  const;
	bool  pause    (const time_t    scheduleTime,
					const time_t    maxWaitTime,
					const string&   condition)     const;
	bool  quit     (const time_t    scheduleTime)  const;
	bool  shutdown (const time_t    scheduleTime)  const;
	bool  snapshot (const time_t    scheduleTime,
					const string&   destination)   const;
	bool  recover  (const time_t    scheduleTime,
					const string&   source)        const;
	bool  reinit   (const time_t    scheduleTime,
					const string&   configID)      const;
	bool  replace  (const time_t    scheduleTime,
					const string&   processList,
					const string&   nodeList,
					const string&   configID)      const;
	string  askInfo (const string&  keylist)       const;    
	bool  cancelCmdQueue ()                        const;    
	ACC::ALC::ACCmd getLastOkCmd()                 const;

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

	CEPApplMgrInterface& 			itsCAMInterface;
	ACC::ALC::ACAsyncClient         itsACclient;
	bool                            itsContinuePoll;
	ACC::ALC::ACCmd                 itsLastOkCmd;
	string                          itsProcName;
};

inline bool  CEPApplMgr::boot     (const time_t    scheduleTime,
                                              const string&   configID)      
{
	itsContinuePoll = true;  
	return itsACclient.boot(scheduleTime, configID);
}
                                              
inline bool  CEPApplMgr::define   (const time_t    scheduleTime)  const
{
	return itsACclient.define(scheduleTime);
}
 
inline bool  CEPApplMgr::init     (const time_t    scheduleTime)  const
{
	return itsACclient.init(scheduleTime);  
}
 
inline bool  CEPApplMgr::run      (const time_t    scheduleTime)  const
{
	return itsACclient.run(scheduleTime);  
}
 
inline bool  CEPApplMgr::pause    (const time_t    scheduleTime,
                                              const time_t    maxWaitTime,
                                              const string&   condition)     const
{
	return itsACclient.pause(scheduleTime, maxWaitTime, condition);  
}
 
inline bool  CEPApplMgr::quit     (const time_t    scheduleTime)  const
{
	return itsACclient.quit(scheduleTime);
}
 
inline bool  CEPApplMgr::shutdown (const time_t    scheduleTime)  const
{
	return itsACclient.shutdown(scheduleTime);
}
 
inline bool  CEPApplMgr::snapshot (const time_t    scheduleTime,
                                              const string&   destination)   const
{
	return itsACclient.snapshot(scheduleTime, destination);
}
 
inline bool  CEPApplMgr::recover  (const time_t    scheduleTime,
                                              const string&   source)        const
{
	return itsACclient.recover(scheduleTime, source);
}
 
inline bool  CEPApplMgr::reinit   (const time_t    scheduleTime,
                                              const string&   configID)      const
{
	return itsACclient.reinit(scheduleTime, configID);
}
 
inline bool  CEPApplMgr::replace  (const time_t    scheduleTime,
                                              const string&   processList,
                                              const string&   nodeList,
                                              const string&   configID)      const
{
	return itsACclient.replace(scheduleTime, processList, nodeList, configID);
}
 
inline bool  CEPApplMgr::cancelCmdQueue ()                       const
{
	return itsACclient.cancelCmdQueue();
}
 
inline ACC::ALC::ACCmd CEPApplMgr::getLastOkCmd() const 
{
	return itsLastOkCmd;
}

inline void CEPApplMgr::stop()
{ }

  } // namespace CEPCU
} // namespace LOFAR
#endif
