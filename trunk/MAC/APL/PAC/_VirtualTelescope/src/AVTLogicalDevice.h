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
//# Common Includes
#include <Common/lofar_string.h>
#include <vector>

//# GCF Includes
#include <GCF/GCF_Port.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>

//# local includes
#include "AVTPropertySetAnswerHandlerInterface.h"
#include "AVTAPCAnswerHandlerInterface.h"
#include "AVTPropertySetAnswer.h"
#include "AVTAPCAnswer.h"
#include "../../../APLCommon/src/APLInterTaskPort.h"

// forward declaration

namespace AVT
{

  class AVTLogicalDevice : public GCFTask, 
                                  AVTPropertySetAnswerHandlerInterface,
                                  AVTAPCAnswerHandlerInterface
  {
    public:

      explicit AVTLogicalDevice(string& taskName, 
                                const TPropertySet& primaryPropertySet,
                                const string& APCName,
                                const string& APCScope); 
      virtual ~AVTLogicalDevice();

      string& getServerPortName();
      void addClientInterTaskPort(APLInterTaskPort* clientPort);
      virtual bool isPrepared(vector<string>& parameters);

      /**
      * The initial state handler. This handler is passed to the GCFTask constructor
      * to indicate that the F_INIT event which starts the state machine is handled
      * by this handler.
      * @param e The event that was received and needs to be handled by the state
      * handler.
      * @param p The port interface (see @a GCFPortInterface) on which the event
      * was received.
      */
      GCFEvent::TResult initial_state(GCFEvent& e, GCFPortInterface& p);

      /**
      * The idle state handler. 
      */
      GCFEvent::TResult idle_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The claiming state handler. 
      */
      GCFEvent::TResult claiming_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The claimed state handler. 
      */
      GCFEvent::TResult claimed_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The preparing state handler. 
      */
      GCFEvent::TResult preparing_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The suspended state handler. 
      */
      GCFEvent::TResult suspended_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The active state handler. 
      */
      GCFEvent::TResult active_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * The releasing state handler. 
      */
      GCFEvent::TResult releasing_state(GCFEvent& e, GCFPortInterface& p);

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
      bool _isLogicalDeviceServerPort(GCFPortInterface& port);
      void _disconnectedHandler(GCFPortInterface& port);
      bool isAPCLoaded() const;
      void apcLoaded();

      /**
      * Initial state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCFEvent::TResult concrete_initial_state(GCFEvent& e, GCFPortInterface& p)=0;
      /**
      * Claiming state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCFEvent::TResult concrete_claiming_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished)=0;
      /**
      * Preparing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCFEvent::TResult concrete_preparing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished, bool& error)=0;
      /**
      * Releasing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCFEvent::TResult concrete_releasing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished)=0;
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

      virtual void concreteClaim(GCFPortInterface& port)=0;
      /**
      * Implementation of the Prepare method is done in the derived classes. 
      */
      virtual void concretePrepare(GCFPortInterface& port,string& parameters)=0;
      /**
      * Implementation of the Resume method is done in the derived classes. 
      */
      virtual void concreteResume(GCFPortInterface& port)=0;
      /**
      * Implementation of the Suspend method is done in the derived classes. 
      */
      virtual void concreteSuspend(GCFPortInterface& port)=0;
      /**
      * Implementation of the Release method is done in the derived classes. 
      */
      virtual void concreteRelease(GCFPortInterface& port)=0;
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteDisconnected(GCFPortInterface& port)=0;

    protected:    
      AVTPropertySetAnswer            m_propertySetAnswer;
      AVTAPCAnswer                    m_APCAnswer;
      GCFMyPropertySet                m_properties;
      GCFApc                          m_APC;

    private:
      string                          m_serverPortName;
      // LogicalDevice SPP
      GCFPort                         m_logicalDeviceServerPort;
      std::vector<APLInterTaskPort*>  m_clientInterTaskPorts;
      bool                            m_apcLoaded;
  };
};
#endif
