//#  LogicalDevice.h: handles all events for a task.
//#
//#  Copyright (C) 2002-2004
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

#ifndef LogicalDevice_H
#define LogicalDevice_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# local includes
#include "APLCommon/PropertySetAnswerHandlerInterface.h"
#include "APLCommon/PropertySetAnswer.h"
#include "APLCommon/APLCommonExceptions.h"

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

//# ACC Includes
#include <ACC/ParameterSet.h>

// forward declaration

#define APL_DECLARE_SHARED_POINTER(classname) \
  typedef boost::shared_ptr< classname > classname##SharedPtr;

namespace LOFAR
{
  
namespace APLCommon
{

  class LogicalDevice : public ::GCFTask,
                               PropertySetAnswerHandlerInterface
  {
    public:

      typedef enum
      {
        LOGICALDEVICE_STATE_NOSTATE=-1,
        LOGICALDEVICE_STATE_IDLE=0,
        LOGICALDEVICE_STATE_CLAIMING,
        LOGICALDEVICE_STATE_CLAIMED,
        LOGICALDEVICE_STATE_PREPARING,
        LOGICALDEVICE_STATE_SUSPENDED,
        LOGICALDEVICE_STATE_ACTIVE,
        LOGICALDEVICE_STATE_RELEASING,
        LOGICALDEVICE_STATE_RELEASED
      } TLogicalDeviceState;

      static const string LD_STATE_STRING_INITIAL;
      static const string LD_STATE_STRING_IDLE;
      static const string LD_STATE_STRING_CLAIMING;
      static const string LD_STATE_STRING_CLAIMED;
      static const string LD_STATE_STRING_PREPARING;
      static const string LD_STATE_STRING_SUSPENDED;
      static const string LD_STATE_STRING_ACTIVE;
      static const string LD_STATE_STRING_RELEASING;
      static const string LD_STATE_STRING_RELEASED;

      // property defines
      static const string LD_PROPNAME_COMMAND;
      static const string LD_PROPNAME_STATUS;
      static const string LD_PROPNAME_CHILDREFS;
      
      // command defines
      static const string LD_COMMAND_SCHEDULE;
      static const string LD_COMMAND_CANCELSCHEDULE;
      static const string LD_COMMAND_CLAIM;
      static const string LD_COMMAND_PREPARE;
      static const string LD_COMMAND_RESUME;
      static const string LD_COMMAND_SUSPEND;
      static const string LD_COMMAND_RELEASE;
      
      explicit LogicalDevice(const string& taskName, const string& parameterFile) throw (APLCommon::ParameterFileNotFoundException, 
                                                                                         APLCommon::ParameterNotFoundException);
      virtual ~LogicalDevice();

      string& getServerPortName();
      virtual bool isPrepared(vector<string>& parameters);
      TLogicalDeviceState getLogicalDeviceState() const;
      
      void handlePropertySetAnswer(::GCFEvent& answer);
      ::GCFEvent::TResult initial_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult idle_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult claiming_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult claimed_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult preparing_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult suspended_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult active_state(::GCFEvent& e, ::GCFPortInterface& p);
      ::GCFEvent::TResult releasing_state(::GCFEvent& e, ::GCFPortInterface& p);

    protected:
      // protected default constructor
      LogicalDevice();
      // protected copy constructor
      LogicalDevice(const LogicalDevice&);
      // protected assignment operator
      LogicalDevice& operator=(const LogicalDevice&);

#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
      typedef GCFTCPPort  TRemotePort;
#else      
      typedef GCFPVSSPort TRemotePort;
#endif
      
      typedef boost::shared_ptr<TRemotePort>  TPortSharedPtr;

      /**
      * returns true if the specified port is the logicalDevice SPP
      */
      bool _isParentPort(::GCFPortInterface& port);
      bool _isServerPort(::GCFPortInterface& port);
      bool _isChildPort(::GCFPortInterface& port);
      bool _isChildStartDaemonPort(::GCFPortInterface& port, string& startDaemonKey);
      void _sendToAllChilds(::GCFEvent& event);
      void _disconnectedHandler(::GCFPortInterface& port);
      bool _isAPCLoaded() const;
      void _apcLoaded();
      void _doStateTransition(const TLogicalDeviceState& newState);
      void _handleTimers(::GCFEvent& event, ::GCFPortInterface& port);
      vector<string> _getChildKeys();
      void _sendEvent(::GCFEvent& event, ::GCFPortInterface& port);
      void _addChildPort(TPortSharedPtr childPort);
      void _sendScheduleToClients();
      string _getShareLocation() const;
      time_t _decodeTimeParameter(const string& timeStr) const;

      virtual void concrete_handlePropertySetAnswer(::GCFEvent& answer)=0;
      virtual ::GCFEvent::TResult concrete_initial_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;
      virtual ::GCFEvent::TResult concrete_idle_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;
      virtual ::GCFEvent::TResult concrete_claiming_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;
      virtual ::GCFEvent::TResult concrete_preparing_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;
      virtual ::GCFEvent::TResult concrete_active_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;
      virtual ::GCFEvent::TResult concrete_releasing_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState& newState)=0;

      virtual void concreteClaim(::GCFPortInterface& port)=0;
      virtual void concretePrepare(::GCFPortInterface& port)=0;
      virtual void concreteResume(::GCFPortInterface& port)=0;
      virtual void concreteSuspend(::GCFPortInterface& port)=0;
      virtual void concreteRelease(::GCFPortInterface& port)=0;
      virtual void concreteParentDisconnected(::GCFPortInterface& port)=0;
      virtual void concreteChildDisconnected(::GCFPortInterface& port)=0;

    protected:    
      APL_DECLARE_SHARED_POINTER(GCFMyPropertySet)
      
      PropertySetAnswer                     m_propertySetAnswer;
      GCFMyPropertySetSharedPtr             m_propertySet;
      ACC::ParameterSet                     m_parameterSet;

    private:
      void _schedule();
      void _cancelSchedule();
      void _claim();
      void _prepare();
      void _resume();
      void _suspend();
      void _release();

      APL_DECLARE_SHARED_POINTER(GCFEvent)
      struct TBufferedEventInfo
      {
        TBufferedEventInfo(time_t t,GCFPortInterface* p,GCFEvent* e) : 
          entryTime(t),
          port(static_cast<TRemotePort*>(p)),
          event(e){};
        
        time_t            entryTime;
        TPortSharedPtr    port;
        GCFEventSharedPtr event;
      };
      
      typedef vector<TPortSharedPtr>        TPortVector;
      typedef map<string,TPortSharedPtr>    TPortMap;
      typedef vector<TBufferedEventInfo>    TEventBufferVector;
      
      string                                m_serverPortName;
      TRemotePort                           m_parentPort; // connection with parent, if any
      TRemotePort                           m_serverPort; // listening port
      
      // the vector and map both contain the child ports. The vector is used
      // to cache the port at the moment of the accept. However, at that moment, 
      // the parent does not yet know the ID of that child. The child sends its
      // ID in the CONNECT event and when that message is received, the port and ID
      // are stored in the TPortMap. The map is used in all communication with the
      // childs.
      TPortVector                           m_childPorts;           // connected childs
      TPortMap                              m_connectedChildPorts;  // connected childs and ID's
      
      TPortMap                              m_childStartDaemonPorts; // child startDaemons
      bool                                  m_apcLoaded;
      TLogicalDeviceState                   m_logicalDeviceState;
      
      unsigned long                         m_prepareTimerId; // actually: claim
      unsigned long                         m_startTimerId; // actually: active
      unsigned long                         m_stopTimerId; // actually: suspend

      unsigned long                         m_retrySendTimerId;
      TEventBufferVector                    m_eventBuffer;

      ALLOC_TRACER_CONTEXT  
  };
};//APLCommon
};//LOFAR
#endif
