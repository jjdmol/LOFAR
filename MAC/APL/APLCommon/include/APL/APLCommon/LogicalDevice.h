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
#include <GCF/PAL/GCF_PVSSPort.h>
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
      void addChildPort(boost::shared_ptr<GCFPVSSPort> childPort);
      virtual bool isPrepared(vector<string>& parameters);
      TLogicalDeviceState getLogicalDeviceState() const;
      
      /**
      * PropertySetAnswerHandlerInterface method
      */
      virtual void handlePropertySetAnswer(::GCFEvent& answer);

      /**
      * The initial state handler. This handler is passed to the GCFTask constructor
      * to indicate that the F_INIT event which starts the state machine is handled
      * by this handler.
      * @param e The event that was received and needs to be handled by the state
      * handler.
      * @param p The port interface (see @a GCFPortInterface) on which the event
      * was received.
      */
      ::GCFEvent::TResult initial_state(::GCFEvent& e, ::GCFPortInterface& p);

      /**
      * The idle state handler. 
      */
      ::GCFEvent::TResult idle_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The claiming state handler. 
      */
      ::GCFEvent::TResult claiming_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The claimed state handler. 
      */
      ::GCFEvent::TResult claimed_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The preparing state handler. 
      */
      ::GCFEvent::TResult preparing_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The suspended state handler. 
      */
      ::GCFEvent::TResult suspended_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The active state handler. 
      */
      ::GCFEvent::TResult active_state(::GCFEvent& e, ::GCFPortInterface& p);
      /**
      * The releasing state handler. 
      */
      ::GCFEvent::TResult releasing_state(::GCFEvent& e, ::GCFPortInterface& p);

    protected:
      // protected default constructor
      LogicalDevice();
      // protected copy constructor
      LogicalDevice(const LogicalDevice&);
      // protected assignment operator
      LogicalDevice& operator=(const LogicalDevice&);

      /**
      * returns true if the specified port is the logicalDevice SPP
      */
      bool _isParentPort(::GCFPortInterface& port);
      bool _isServerPort(::GCFPortInterface& port);
      bool _isChildPort(::GCFPortInterface& port);
      void _sendToAllChilds(::GCFEvent& event);
      void _disconnectedHandler(::GCFPortInterface& port);
      void _doStateTransition(const TLogicalDeviceState& newState);
      void _handleTimers(::GCFEvent& event, ::GCFPortInterface& port);
      bool _isAPCLoaded() const;
      void _apcLoaded();

      /**
      * Initial state additional behaviour must be implemented in the derived classes. 
      */
      virtual ::GCFEvent::TResult concrete_initial_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE)=0;
      /**
      * Claiming state additional behaviour must be implemented in the derived classes. 
      */
      virtual ::GCFEvent::TResult concrete_claiming_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE)=0;
      /**
      * Preparing state additional behaviour must be implemented in the derived classes. 
      */
      virtual ::GCFEvent::TResult concrete_preparing_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE)=0;
      /**
      * active state additional behaviour must be implemented in the derived classes. 
      */
      virtual ::GCFEvent::TResult concrete_active_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE)=0;
      /**
      * Releasing state additional behaviour must be implemented in the derived classes. 
      */
      virtual ::GCFEvent::TResult concrete_releasing_state(::GCFEvent& e, ::GCFPortInterface& p, TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE)=0;

      /**
      * Implementation of the Claim method is done in the derived classes. 
      */
      virtual void concreteClaim(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Prepare method is done in the derived classes. 
      */
      virtual void concretePrepare(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Resume method is done in the derived classes. 
      */
      virtual void concreteResume(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Suspend method is done in the derived classes. 
      */
      virtual void concreteSuspend(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Release method is done in the derived classes. 
      */
      virtual void concreteRelease(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteParentDisconnected(::GCFPortInterface& port)=0;
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteChildDisconnected(::GCFPortInterface& port)=0;

    protected:    
      PropertySetAnswer                     m_propertySetAnswer;
      boost::shared_ptr<GCFMyPropertySet>   m_properties;

    private:
      typedef vector<boost::shared_ptr<GCFPVSSPort> > TPVSSPortVector;
      
      string                                m_serverPortName;
      ::GCFPVSSPort                         m_parentPort; // connection with parent, if any
      ::GCFPVSSPort                         m_serverPort; // listening port
      TPVSSPortVector                       m_childPorts;    // connected childs
      bool                                  m_apcLoaded;
      TLogicalDeviceState                   m_logicalDeviceState;
      
      unsigned long                         m_prepareTimerId; // actually: claim
      unsigned long                         m_startTimerId; // actually: active
      unsigned long                         m_stopTimerId; // actually: suspend
      
      ACC::ParameterSet                     m_parameterSet;
  };
};//APLCommon
};//LOFAR
#endif
