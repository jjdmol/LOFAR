//#  AVTLogicalDevice.h: handles all events for a task.
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

#ifndef AVTLogicalDevice_H
#define AVTLogicalDevice_H

//# Includes

#include <Common/LofarLogger.h>
//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_Task.h>

//# local includes
#include "AVTPropertySetAnswerHandlerInterface.h"
#include "AVTPropertySetAnswer.h"
#include <APLCommon/APLInterTaskPort.h>
#include "AVTDefines.h"

//# Common Includes
#include <Common/lofar_string.h>
#include <vector>

// forward declaration
#include <GCF/TM/GCF_Event.h>

namespace LOFAR
{
  
namespace AVT
{

  class AVTLogicalDevice : public GCF::TM::GCFTask, 
                                  AVTPropertySetAnswerHandlerInterface
  {
    public:

      explicit AVTLogicalDevice(string& taskName, 
                                const string& scope,
                                const string& type,
                                const string& APCName
                                ); 
      virtual ~AVTLogicalDevice();

      string& getServerPortName();
      void addClientInterTaskPort(APLCommon::APLInterTaskPort* clientPort);
      virtual bool isPrepared(vector<string>& parameters);
      TLogicalDeviceState getLogicalDeviceState() const;

      /**
      * The initial state handler. This handler is passed to the GCFTask constructor
      * to indicate that the F_INIT event which starts the state machine is handled
      * by this handler.
      * @param e The event that was received and needs to be handled by the state
      * handler.
      * @param p The port interface (see @a GCFPortInterface) on which the event
      * was received.
      */
      GCF::TM::GCFEvent::TResult initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);

      /**
      * The idle state handler. 
      */
      GCF::TM::GCFEvent::TResult idle_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The claiming state handler. 
      */
      GCF::TM::GCFEvent::TResult claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The claimed state handler. 
      */
      GCF::TM::GCFEvent::TResult claimed_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The preparing state handler. 
      */
      GCF::TM::GCFEvent::TResult preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The suspended state handler. 
      */
      GCF::TM::GCFEvent::TResult suspended_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The active state handler. 
      */
      GCF::TM::GCFEvent::TResult active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * The releasing state handler. 
      */
      GCF::TM::GCFEvent::TResult releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);

    protected:
      // protected default constructor
      AVTLogicalDevice();
      // protected copy constructor
      AVTLogicalDevice(const AVTLogicalDevice&);
      // protected assignment operator
      AVTLogicalDevice& operator=(const AVTLogicalDevice&);

      /**
      * returns true if the specified port is the logicalDevice SPP
      */
      bool _isLogicalDeviceServerPort(GCF::TM::GCFPortInterface& port);
      void _disconnectedHandler(GCF::TM::GCFPortInterface& port);
      bool isAPCLoaded() const;
      void apcLoaded();

      /**
      * Initial state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p)=0;
      /**
      * Claiming state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished)=0;
      /**
      * Preparing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished, bool& error)=0;
      /**
      * active state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p)=0;
      /**
      * Releasing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished)=0;
      /**
      * Implementation of the Claim method is done in the derived classes. 
      */

  /*******************************************
  * 
  * Remove the following abstract methods. Combine their implementations with the
  * F_INIT handling in their concrete_...ing_state methods that are entered
  * just before calling them
  * 
  * ********************************************/

      virtual void concreteClaim(GCF::TM::GCFPortInterface& port)=0;
      /**
      * Implementation of the Prepare method is done in the derived classes. 
      */
      virtual void concretePrepare(GCF::TM::GCFPortInterface& port,string& parameters)=0;
      /**
      * Implementation of the Resume method is done in the derived classes. 
      */
      virtual void concreteResume(GCF::TM::GCFPortInterface& port)=0;
      /**
      * Implementation of the Suspend method is done in the derived classes. 
      */
      virtual void concreteSuspend(GCF::TM::GCFPortInterface& port)=0;
      /**
      * Implementation of the Release method is done in the derived classes. 
      */
      virtual void concreteRelease(GCF::TM::GCFPortInterface& port)=0;
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteDisconnected(GCF::TM::GCFPortInterface& port)=0;

    protected:    
      AVTPropertySetAnswer            m_propertySetAnswer;
      GCF::PAL::GCFMyPropertySet                m_properties;
      string                          m_APC;

    private:
      string                          m_serverPortName;
      // LogicalDevice SPP
      GCF::TM::GCFPort                         m_logicalDeviceServerPort;
      std::vector<APLCommon::APLInterTaskPort*>  m_clientInterTaskPorts;
      bool                            m_apcLoaded;
      TLogicalDeviceState             m_logicalDeviceState;

      ALLOC_TRACER_CONTEXT  
  };
};//AVT
};//LOFAR
#endif
