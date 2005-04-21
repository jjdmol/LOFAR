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
#include <ACC/ACAsyncClient.h>
#include <GCF/TM/GCF_Handler.h>

//# local includes
//# Common Includes

// forward declaration

namespace LOFAR
{  
  namespace AVB // A)pplication layer V)irtual B)ackend
  {

class CEPApplicationManagerInterface
{
  protected:
    CEPApplicationManagerInterface() {}

  public:
    virtual ~CEPApplicationManagerInterface() {}
    
  public:
    virtual void    appBooted(uint16 result) = 0;
    virtual void    appDefined(uint16 result) = 0;
    virtual void    appInitialized(uint16 result) = 0;
    virtual void    appRunDone(uint16 result) = 0;
    virtual void    appPaused(uint16 result) = 0;
    virtual void    appQuitDone(uint16 result) = 0;
    virtual void    appSnapshotDone(uint16 result) = 0;
    virtual void    appRecovered(uint16 result) = 0;
    virtual void    appReinitialized(uint16 result) = 0;
    virtual void    appReplaced(uint16 result) = 0;
    virtual string  appSupplyInfo(const string& keyList) = 0;
    virtual void    appSupplyInfoAnswer(const string& answer) = 0;    

  private:
    // protected copy constructor
    CEPApplicationManagerInterface(const CEPApplicationManagerInterface&);
    // protected assignment operator
    CEPApplicationManagerInterface& operator=(const CEPApplicationManagerInterface&);
};

class CEPApplicationManager : public ACC::ACClientFunctions,
                                     GCF::TM::GCFHandler
{
  public:
    CEPApplicationManager(CEPApplicationManagerInterface& interface, const string& appName);
    virtual ~CEPApplicationManager();
    
  public: // methods may be called from specialized CEPApplicationManagerInterface
    bool  boot     (const time_t    scheduleTime,
                    const string&   configID)      ;
    bool  define   (const time_t    scheduleTime)  const ;
    bool  init     (const time_t    scheduleTime)  const ;
    bool  run      (const time_t    scheduleTime)  const ;
    bool  pause    (const time_t    scheduleTime,
                    const time_t    maxWaitTime,
                    const string&   condition)     const ;
    bool  quit     (const time_t    scheduleTime)  const ;
    bool  shutdown (const time_t    scheduleTime)  const ;
    bool  snapshot (const time_t    scheduleTime,
                    const string&   destination)   const ;
    bool  recover  (const time_t    scheduleTime,
                    const string&   source)        const ;
    bool  reinit   (const time_t    scheduleTime,
                    const string&   configID)      const ;
    bool  replace  (const time_t    scheduleTime,
                    const string&   processList,
                    const string&   nodeList,
                    const string&   configID)      const ;
    string  askInfo (const string&  keylist)       const;    
     
  private: // implemenation of abstract GCFHandler methods
    friend class GCF::TM::GCFHandler;
    void workProc();    
    void stop();
  
  private: // implemenation of abstract ACClientFunctions methods
    friend class ACC::ACClientFunctions;
    void  handleAckMsg      (ACC::ACCmd    cmd, 
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
    ACC::ACAsyncClient              _acClient;
    bool                            _continuePoll;
    
    ALLOC_TRACER_CONTEXT  
};

inline CEPApplicationManager::CEPApplicationManager(
                                    CEPApplicationManagerInterface& interface, 
                                    const string& appName) :
      _interface(interface),
      _acClient(this, appName, 10, 100, 1, 0),
      _continuePoll(false)
      
{ 
  use(); // to avoid that this object will be deleted in GCFTask::stop;
}

inline CEPApplicationManager::~CEPApplicationManager()
{
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
 
inline bool  CEPApplicationManager::replace  (const time_t    scheduleTime,
                                              const string&   processList,
                                              const string&   nodeList,
                                              const string&   configID)      const
{
  return _acClient.replace(scheduleTime, processList, nodeList, configID);
}
 
inline string  CEPApplicationManager::askInfo (const string&  keylist)       const
{
  return _acClient.askInfo(keylist);
}
 
inline void CEPApplicationManager::stop()
{
}

  } // namespace AVB
} // namespace LOFAR
#endif
