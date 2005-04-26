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
#include <boost/weak_ptr.hpp>

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
#include "APLCommon/LogicalDevice_Protocol.ph"
#include "APLCommon/StartDaemon_Protocol.ph"

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

  class LogicalDevice : public GCF::TM::GCFTask,
                               PropertySetAnswerHandlerInterface
  {
    public:

      typedef enum
      {
        LOGICALDEVICE_STATE_NOSTATE=-2,
        LOGICALDEVICE_STATE_DISABLED=-1,
        LOGICALDEVICE_STATE_INITIAL=0,
        LOGICALDEVICE_STATE_IDLE,
        LOGICALDEVICE_STATE_CLAIMING,
        LOGICALDEVICE_STATE_CLAIMED,
        LOGICALDEVICE_STATE_PREPARING,
        LOGICALDEVICE_STATE_SUSPENDED,
        LOGICALDEVICE_STATE_ACTIVE,
        LOGICALDEVICE_STATE_RELEASING,
        LOGICALDEVICE_STATE_GOINGDOWN
      } TLogicalDeviceState;

      static const string LD_STATE_STRING_DISABLED;
      static const string LD_STATE_STRING_INITIAL;
      static const string LD_STATE_STRING_IDLE;
      static const string LD_STATE_STRING_CLAIMING;
      static const string LD_STATE_STRING_CLAIMED;
      static const string LD_STATE_STRING_PREPARING;
      static const string LD_STATE_STRING_SUSPENDED;
      static const string LD_STATE_STRING_ACTIVE;
      static const string LD_STATE_STRING_RELEASING;
      static const string LD_STATE_STRING_GOINGDOWN;

      // property defines
      static const string LD_PROPSET_TYPENAME;
      static const string LD_PROPNAME_COMMAND;
      static const string LD_PROPNAME_STATUS;
      static const string LD_PROPNAME_STATE;
      static const string LD_PROPNAME_VERSION;
      static const string LD_PROPNAME_CLAIMTIME;
      static const string LD_PROPNAME_PREPARETIME;
      static const string LD_PROPNAME_STARTTIME;
      static const string LD_PROPNAME_STOPTIME;
      static const string LD_PROPNAME_CHILDREFS;
      
      // command defines
      static const string LD_COMMAND_SCHEDULE;
      static const string LD_COMMAND_CANCELSCHEDULE;
      static const string LD_COMMAND_CLAIM;
      static const string LD_COMMAND_PREPARE;
      static const string LD_COMMAND_RESUME;
      static const string LD_COMMAND_SUSPEND;
      static const string LD_COMMAND_RELEASE;
      
      explicit LogicalDevice(const string& taskName, 
                             const string& parameterFile, 
                             GCF::TM::GCFTask* pStartDaemon,
                             const string& version) throw (APLCommon::ParameterFileNotFoundException, 
                                                           APLCommon::ParameterNotFoundException,
                                                           APLCommon::WrongVersionException);
      virtual ~LogicalDevice();

      string& getServerPortName();
      virtual bool isPrepared(vector<string>& parameters);
      TLogicalDeviceState getLogicalDeviceState() const;
      
      void handlePropertySetAnswer(GCF::TM::GCFEvent& answer);
      GCF::TM::GCFEvent::TResult initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult idle_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult claimed_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult suspended_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult goingdown_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);

    protected:
      // protected default constructor
      LogicalDevice();
      // protected copy constructor
      LogicalDevice(const LogicalDevice&);
      // protected assignment operator
      LogicalDevice& operator=(const LogicalDevice&);

#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
      typedef GCF::TM::GCFTCPPort  TRemotePort;
#else      
      typedef GCF::PAL::GCFPVSSPort TRemotePort;
#endif
      
      typedef boost::shared_ptr<TRemotePort>  TPortSharedPtr;
      typedef boost::weak_ptr<TRemotePort>    TPortWeakPtr;
      APL_DECLARE_SHARED_POINTER(GCF::TM::GCFEvent)

      /**
      * returns true if the specified port is the logicalDevice SPP
      */
      bool _isParentPort(GCF::TM::GCFPortInterface& port);
      bool _isServerPort(GCF::TM::GCFPortInterface& port);
      bool _isChildPort(GCF::TM::GCFPortInterface& port);
      bool _isChildStartDaemonPort(GCF::TM::GCFPortInterface& port, string& startDaemonKey);
      void _sendToAllChilds(GCFEventSharedPtr eventPtr);
      bool _childsInState(const double requiredPercentage, const TLogicalDeviceTypes& type, const TLogicalDeviceState& state);
      bool _childsNotInState(const double requiredPercentage, const TLogicalDeviceTypes& type, const TLogicalDeviceState& state);
      void _disconnectedHandler(GCF::TM::GCFPortInterface& port);
      bool _isAPCLoaded() const;
      void _apcLoaded();
      void _doStateTransition(const TLogicalDeviceState& newState, const TLDResult& errorCode);
      void _handleTimers(GCF::TM::GCFEvent& event, GCF::TM::GCFPortInterface& port);
      vector<string> _getChildKeys();
      void _sendEvent(GCFEventSharedPtr eventPtr, GCF::TM::GCFPortInterface& port);
      void _addChildPort(TPortSharedPtr childPort);
      void _sendScheduleToClients();
      string _getShareLocation() const;
      time_t _decodeTimeParameter(const string& timeStr) const;
      time_t getClaimTime() const;
      time_t getPrepareTime() const;
      time_t getStartTime() const;
      time_t getStopTime() const;

      virtual void concrete_handlePropertySetAnswer(GCF::TM::GCFEvent& answer)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_idle_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_claimed_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLDResult& errorCode)=0;
      virtual GCF::TM::GCFEvent::TResult concrete_releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)=0;

      virtual void concreteClaim(GCF::TM::GCFPortInterface& port)=0;
      virtual void concretePrepare(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteResume(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteSuspend(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteRelease(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteParentDisconnected(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteChildDisconnected(GCF::TM::GCFPortInterface& port)=0;
      virtual void concreteHandleTimers(GCF::TM::GCFTimerEvent& timerEvent, GCF::TM::GCFPortInterface& port)=0;
      

    protected:    
      APL_DECLARE_SHARED_POINTER(GCF::PAL::GCFMyPropertySet)
      
      GCF::TM::GCFTask*                     m_startDaemon;
      PropertySetAnswer                     m_propertySetAnswer;
      GCFMyPropertySetSharedPtr             m_basePropertySet;
      std::string                           m_basePropertySetName;
      GCFMyPropertySetSharedPtr             m_detailsPropertySet;
      std::string                           m_detailsPropertySetName;
      ACC::ParameterSet                     m_parameterSet;

      string                                m_serverPortName;
      TRemotePort                           m_serverPort; // listening port

    private:
      struct TBufferedEventInfo
      {
        TBufferedEventInfo(time_t t,GCF::TM::GCFPortInterface* p,GCFEventSharedPtr e) : 
          entryTime(t),
          port(static_cast<TRemotePort*>(p)),
          event(e){};
        
        time_t            entryTime;
        TRemotePort*      port;
        GCFEventSharedPtr event;
      };
      
      typedef vector<TPortSharedPtr>          TPortVector;
      typedef map<string,TPortSharedPtr>      TPortMap;
      typedef map<string,TPortWeakPtr>        TPortWeakPtrMap;
      typedef vector<TBufferedEventInfo>      TEventBufferVector;
      typedef map<string,TLogicalDeviceTypes> TString2LDTypeMap;
      typedef map<string,TLogicalDeviceState> TString2LDStateMap;
      
      void _schedule();
      void _cancelSchedule();
      void _claim();
      void _prepare();
      void _resume();
      void _suspend();
      void _release();
      TPortVector::iterator _getChildPort(GCF::TM::GCFPortInterface& port);
      void _setChildStates(TLogicalDeviceState ldState);
      void _setConnectedChildState(GCF::TM::GCFPortInterface& port, TLogicalDeviceState ldState);

      TRemotePort                           m_parentPort; // connection with parent, if any
      
      // the vector and map both contain the child ports. The vector is used
      // to cache the port at the moment of the accept. However, at that moment, 
      // the parent does not yet know the ID of that child. The child sends its
      // ID in the CONNECT event and when that message is received, the port and ID
      // are stored in the TPortMap. The map is used in all communication with the
      // childs.
      TPortVector                           m_childPorts;           // connected childs
      TPortWeakPtrMap                       m_connectedChildPorts;  // connected childs and ID's
      
      TPortMap                              m_childStartDaemonPorts; // child startDaemons
      bool                                  m_apcLoaded;
      TLogicalDeviceState                   m_logicalDeviceState;
      
      unsigned long                         m_claimTimerId;
      unsigned long                         m_prepareTimerId;
      unsigned long                         m_startTimerId; // LD becomes active
      unsigned long                         m_stopTimerId; // LD becomes suspended
      
      time_t                                m_claimTime; // in UTC, seconds since 1-1-1970
      time_t                                m_prepareTime; // in UTC, seconds since 1-1-1970
      time_t                                m_startTime;   // in UTC, seconds since 1-1-1970
      time_t                                m_stopTime;    // in UTC, seconds since 1-1-1970

      unsigned long                         m_retrySendTimerId;
      TEventBufferVector                    m_eventBuffer;
      
      TLDResult                             m_globalError;
      const string                          m_version;

      TString2LDTypeMap                     m_childTypes;
      TString2LDStateMap                    m_childStates;

      ALLOC_TRACER_CONTEXT  
  };
};//APLCommon
};//LOFAR
#endif
