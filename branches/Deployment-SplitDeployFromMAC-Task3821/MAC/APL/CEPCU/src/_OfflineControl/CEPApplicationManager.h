//#  CEPApplicationManager.h: factory class for Virtual Backends.
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

#ifndef CEPAPPLICATIONMANAGER_H
#define CEPAPPLICATIONMANAGER_H

//# Includes
#include <ALC/ACAsyncClient.h>
#include <GCF/TM/GCF_Handler.h>

//# local includes
//# Common Includes

// forward declaration

namespace LOFAR
{  
  namespace CEPCU
  {

class CEPApplicationManagerInterface
{
  protected:
    CEPApplicationManagerInterface() {}

  public:
    virtual ~CEPApplicationManagerInterface() {}
    
  public:
    virtual void    appBooted(const string& procName, uint16 result) = 0;
    virtual void    appDefined(const string& procName, uint16 result) = 0;
    virtual void    appInitialized(const string& procName, uint16 result) = 0;
    virtual void    appRunDone(const string& procName, uint16 result) = 0;
    virtual void    appPaused(const string& procName, uint16 result) = 0;
    virtual void    appQuitDone(const string& procName, uint16 result) = 0;
    virtual void    appSnapshotDone(const string& procName, uint16 result) = 0;
    virtual void    appRecovered(const string& procName, uint16 result) = 0;
    virtual void    appReinitialized(const string& procName, uint16 result) = 0;
    virtual void    appReplaced(const string& procName, uint16 result) = 0;
    virtual string  appSupplyInfo(const string& procName, const string& keyList) = 0;
    virtual void    appSupplyInfoAnswer(const string& procName, const string& answer) = 0;    

  private:
    // protected copy constructor
    CEPApplicationManagerInterface(const CEPApplicationManagerInterface&);
    // protected assignment operator
    CEPApplicationManagerInterface& operator=(const CEPApplicationManagerInterface&);
};

class CEPApplicationManager : public ACC::ALC::ACClientFunctions,
                                     GCF::TM::GCFHandler
{
  public:
    CEPApplicationManager(CEPApplicationManagerInterface& interface, const string& appName);
    virtual ~CEPApplicationManager();
    
  public: // methods may be called from specialized CEPApplicationManagerInterface
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
    ACC::ALC::ACCmd getLastOkCmd()                 const;
     
  private: // implemenation of abstract GCFHandler methods
    friend class GCF::TM::GCFHandler;
    void workProc();    
    void stop();
  
  private: // implemenation of abstract ACClientFunctions methods
    friend class ACC::ALC::ACClientFunctions;
    void  handleAckMsg      (ACC::ALC::ACCmd    cmd, 
                             uint16        result,
                             const string& info);
  
    void  handleAnswerMsg   (const string& answer);
  
    string  supplyInfoFunc  (const string& keyList);
    
  protected:
    // protected copy constructor
    CEPApplicationManager(const CEPApplicationManager&);
    // protected assignment operator
    CEPApplicationManager& operator=(const CEPApplicationManager&);

  private:
    CEPApplicationManagerInterface& _interface;
    ACC::ALC::ACAsyncClient         _acClient;
    bool                            _continuePoll;
    ACC::ALC::ACCmd                 _lastOkCmd;
	string                          _procName;
    
    ALLOC_TRACER_CONTEXT  
};

inline CEPApplicationManager::CEPApplicationManager(
                                    CEPApplicationManagerInterface& interface, 
                                    const string& appName) :
      _interface(interface),
	  _acClient(this, appName, 10, 100, 1, 0),
      _continuePoll(false),
	  _lastOkCmd(ACC::ALC::ACCmdNone),
      _procName(appName)
{
  LOG_DEBUG(formatString("constructing CEPApplicationManager(%s)",_procName.c_str()));
  use(); // to avoid that this object will be deleted in GCFTask::stop;
}

inline CEPApplicationManager::~CEPApplicationManager()
{
  LOG_DEBUG(formatString("destructing CEPApplicationManager(%s)",_procName.c_str()));
  GCFTask::deregisterHandler(*this);
}

inline bool  CEPApplicationManager::boot     (const time_t    scheduleTime,
                                              const string&   configID)      
{
  _continuePoll = true;  
  return _acClient.boot(scheduleTime, configID);
}
                                              
inline bool  CEPApplicationManager::define   (const time_t    scheduleTime)  const
{
  return _acClient.define(scheduleTime);
}
 
inline bool  CEPApplicationManager::init     (const time_t    scheduleTime)  const
{
  return _acClient.init(scheduleTime);  
}
 
inline bool  CEPApplicationManager::run      (const time_t    scheduleTime)  const
{
  return _acClient.run(scheduleTime);  
}
 
inline bool  CEPApplicationManager::pause    (const time_t    scheduleTime,
                                              const time_t    maxWaitTime,
                                              const string&   condition)     const
{
  return _acClient.pause(scheduleTime, maxWaitTime, condition);  
}
 
inline bool  CEPApplicationManager::quit     (const time_t    scheduleTime)  const
{
  return _acClient.quit(scheduleTime);
}
 
inline bool  CEPApplicationManager::shutdown (const time_t    scheduleTime)  const
{
  return _acClient.shutdown(scheduleTime);
}
 
inline bool  CEPApplicationManager::snapshot (const time_t    scheduleTime,
                                              const string&   destination)   const
{
  return _acClient.snapshot(scheduleTime, destination);
}
 
inline bool  CEPApplicationManager::recover  (const time_t    scheduleTime,
                                              const string&   source)        const
{
  return _acClient.recover(scheduleTime, source);
}
 
inline bool  CEPApplicationManager::reinit   (const time_t    scheduleTime,
                                              const string&   configID)      const
{
  return _acClient.reinit(scheduleTime, configID);
}
 
inline bool  CEPApplicationManager::release  (const time_t    scheduleTime) const
{
  return _acClient.release(scheduleTime);
}
 
inline bool  CEPApplicationManager::cancelCmdQueue ()                       const
{
  return _acClient.cancelCmdQueue();
}
 
inline ACC::ALC::ACCmd CEPApplicationManager::getLastOkCmd() const 
{
  return _lastOkCmd;
}

inline void CEPApplicationManager::stop()
{
}

  } // namespace CEPCU
} // namespace LOFAR
#endif
