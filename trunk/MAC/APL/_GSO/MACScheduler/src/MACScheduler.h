//#  MACScheduler.h: Interface between MAC and SAS.
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

#ifndef MACScheduler_H
#define MACScheduler_H

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
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
#include <ACC/ConfigurationMgr.h>
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
#include <ACC/ParameterSet.h>

// forward declaration

namespace LOFAR
{
  
namespace GSO
{

  class MACScheduler : public ::GCFTask,
                              APLCommon::PropertySetAnswerHandlerInterface
  {
    public:
      static const string MS_CONFIG_PREFIX;
      static const string MS_TASKNAME;

      static const string MS_STATE_STRING_INITIAL;
      static const string MS_STATE_STRING_IDLE;
      
      // property defines
      static const string MS_PROPSET_NAME;
      static const string MS_PROPSET_TYPE;
      static const string MS_PROPNAME_COMMAND;
      static const string MS_PROPNAME_STATUS;
      
      // command defines
      static const string MS_COMMAND_SCHEDULE;
      static const string MS_COMMAND_UPDATESCHEDULE;
      static const string MS_COMMAND_CANCELSCHEDULE;

               MACScheduler();
      virtual ~MACScheduler();

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

    protected:
      // protected copy constructor
      MACScheduler(const MACScheduler&);
      // protected assignment operator
      MACScheduler& operator=(const MACScheduler&);

      void _disconnectedHandler(::GCFPortInterface& port);

    protected:    
      APLCommon::PropertySetAnswer          m_propertySetAnswer;
      boost::shared_ptr<GCFMyPropertySet>   m_propertySet;

    private:
      typedef boost::shared_ptr<GCFTCPPort> TTCPPortPtr;
      typedef vector<TTCPPortPtr>           TTCPPortVector;
      typedef map<string,string>            TNodeId2PortMap;
      typedef map<string,TTCPPortPtr>       TStringTCPPortMap;
      
      bool _isServerPort(const GCFTCPPort& server, const ::GCFPortInterface& port) const;
      bool _isSASclientPort(const ::GCFPortInterface& port) const;
      bool _isVISDclientPort(const ::GCFPortInterface& port, string& visd) const;
      bool _isVIclientPort(const ::GCFPortInterface& port) const;
      
      string                                m_SASserverPortName;
      GCFTCPPort                            m_SASserverPort;      // SAS-MAC communication
      TTCPPortVector                        m_SASclientPorts;     // connected SAS clients
      TStringTCPPortMap                     m_VISDclientPorts;    // connected VI StartDaemon clients
      string                                m_VIparentPortName;
      GCFTCPPort                            m_VIparentPort;       // parent for VI's
      TTCPPortVector                        m_VIclientPorts;      // created VI's
      TNodeId2PortMap                       m_NodeId2PortMap;     // maps node ID's to port names 
      
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
      boost::shared_ptr<ACC::ConfigurationMgr> m_configurationManager;
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
   };
};//GSO
};//LOFAR
#endif
