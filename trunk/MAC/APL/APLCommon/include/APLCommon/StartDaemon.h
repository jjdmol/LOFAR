//#  StartDaemon.h: Server class that creates Logical Devices upon request.
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

#ifndef StartDaemon_H
#define StartDaemon_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_PVSSPort.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# Common Includes
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//# local includes
#include "APLCommon/PropertySetAnswerHandlerInterface.h"
#include "APLCommon/PropertySetAnswer.h"

#include "APLCommon/StartDaemon_Protocol.ph"

// forward declaration

namespace LOFAR
{
  
namespace APLCommon
{
  class LogicalDevice;
  class LogicalDeviceFactory;
  
  class StartDaemon : public ::GCFTask,
                             PropertySetAnswerHandlerInterface
  {
    public:
    
      static const string PSTYPE_STARTDAEMON;
      static const string SD_PROPNAME_COMMAND;
      static const string SD_PROPNAME_STATUS;
      static const string SD_COMMAND_SCHEDULE;
      static const string SD_COMMAND_STOP;

      StartDaemon(const string& name); 
      virtual ~StartDaemon();
      
      void registerFactory(TLogicalDeviceTypes ldType, boost::shared_ptr<LogicalDeviceFactory> factory);
      TSDResult createLogicalDevice(const TLogicalDeviceTypes ldType, const string& taskName, const string& fileName);

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

      virtual void handlePropertySetAnswer(GCFEvent& answer);

    protected:
      // protected copy constructor
      StartDaemon(const StartDaemon&);
      // protected assignment operator
      StartDaemon& operator=(const StartDaemon&);

    private:
      bool _isServerPort(::GCFPortInterface& port);
      bool _isChildPort(::GCFPortInterface& port);
      void _disconnectedHandler(::GCFPortInterface& port);

#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
      typedef GCFTCPPort  TThePortTypeInUse;
#else      
      typedef GCFPVSSPort TThePortTypeInUse;
#endif
      typedef map<TLogicalDeviceTypes,boost::shared_ptr<LogicalDeviceFactory> > TFactoryMap;
      typedef vector<boost::shared_ptr<LogicalDevice> > TLogicalDeviceVector;
      typedef vector<boost::shared_ptr<TThePortTypeInUse> > TPortVector;

      PropertySetAnswer               m_propertySetAnswer;
      ::GCFMyPropertySet              m_properties;
      string                          m_serverPortName;
      TThePortTypeInUse               m_serverPort; // listening port
      TPortVector                     m_childPorts;    // connected childs

      TFactoryMap                     m_factories;
      TLogicalDeviceVector            m_logicalDevices;
    
      ALLOC_TRACER_CONTEXT  
  };
};//APLCommon
};//LOFAR
#endif
