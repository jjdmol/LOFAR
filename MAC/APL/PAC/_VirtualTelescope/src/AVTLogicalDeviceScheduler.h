//#  AVTLogicalDeviceScheduler.h: schedules logical devices
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

#ifndef AVTLogicalDeviceScheduler_H
#define AVTLogicalDeviceScheduler_H

//# Includes
//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/GCF_Port.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>
#include <GCF/GCF_Property.h>

//# local includes
#include "AVTPropertySetAnswerHandlerInterface.h"
#include "AVTPropertySetAnswer.h"
#include "AVTLogicalDevice.h"
#include "APLInterTaskPort.h"
#include "AVTResourceManager.h"

// forward declaration

namespace AVT
{
  class AVTStationReceptor;
  
  class AVTLogicalDeviceScheduler : public GCFTask,
                                          AVTPropertySetAnswerHandlerInterface
  {
    public:

      AVTLogicalDeviceScheduler(); 
      virtual ~AVTLogicalDeviceScheduler();

      bool isInitialized();

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
      * PropertySet answer handling
      */
      virtual void handlePropertySetAnswer(GCFEvent& answer);

    protected:
      // protected copy constructor
      AVTLogicalDeviceScheduler(const AVTLogicalDeviceScheduler&);
      // protected assignment operator
      AVTLogicalDeviceScheduler& operator=(const AVTLogicalDeviceScheduler&);

    private:
      struct LogicalDeviceInfoT
      {
        // constructor:
        LogicalDeviceInfoT() :                  logicalDevice(),
                                                currentSchedule(0),
                                                clientPort(),
                                                children() {};
                                                
        boost::shared_ptr<AVTLogicalDevice>     logicalDevice;
        unsigned long                           currentSchedule;
        boost::shared_ptr<APLInterTaskPort>     clientPort;
        bool                                    permanent; // prevents releasing the LD when no more schedules exist
        map<string,LogicalDeviceInfoT>          children; // recursive
      };

      struct LogicalDeviceScheduleInfoT
      {
        // constructor: 
        LogicalDeviceScheduleInfoT() :      deviceName(),
                                            parameters(),
                                            startTime(0),
                                            stopTime(0),
                                            prepareTimerId(0),
                                            startTimerId(0),
                                            stopTimerId(0) {};
        
        string                              deviceName;
        vector<string>                      parameters;
        int                                 startTime;
        int                                 stopTime;
        unsigned long                       prepareTimerId;
        unsigned long                       startTimerId;
        unsigned long                       stopTimerId;
      };
      
      struct MaintenanceScheduleInfoT
      {
        // constructor: 
        MaintenanceScheduleInfoT() :        resource(),
                                            pMaintenanceProperty(),
                                            startTime(0),
                                            stopTime(0),
                                            startTimerId(0),
                                            stopTimerId(0) {};
        
        string                              resource;
        boost::shared_ptr<GCFProperty>      pMaintenanceProperty;
        int                                 startTime;
        int                                 stopTime;
        unsigned long                       startTimerId;
        unsigned long                       stopTimerId;
      };
      
      typedef map<string,LogicalDeviceInfoT>                  LogicalDeviceMapT;
      typedef LogicalDeviceMapT::iterator                     LogicalDeviceMapIterT;
      typedef map<unsigned long,LogicalDeviceScheduleInfoT>   LogicalDeviceScheduleT;
      typedef LogicalDeviceScheduleT::iterator                LogicalDeviceScheduleIterT;
      typedef map<unsigned long,MaintenanceScheduleInfoT>     MaintenanceScheduleT;
      typedef MaintenanceScheduleT::iterator                  MaintenanceScheduleIterT;
      typedef map<string,TPropertySet>                        PropertySetMapT;
      typedef PropertySetMapT::iterator                       PropertySetMapIterT;

      boost::shared_ptr<AVTStationReceptor> addReceptor(string srName,const TPropertySet& propertySet, const list<string>& requiredResources);
      void addReceptorGroup(string srName,const TPropertySet& propertySet, vector<boost::shared_ptr<AVTStationReceptor> >& receptors);
      LogicalDeviceMapIterT findLogicalDevice(const unsigned long scheduleId);
      LogicalDeviceMapIterT findClientPort(GCFPortInterface& port);
      LogicalDeviceScheduleIterT findSchedule(const string& deviceName,LogicalDeviceScheduleIterT beginIt);
      bool submitSchedule(const unsigned long scheduleId,const string& deviceName, const vector<string> scheduleParameters, boost::shared_ptr<APLInterTaskPort> clientPort);
      /*
       * returns true if the timerId is a prepareTimer for the given logical device
       * Also sets the current schedule id for the logical device.
       */
      bool checkPrepareTimer(const string& deviceName, unsigned long timerId);
      /*
       * returns true if the timerId is a startTimer for the given logical device
       * Also sets the current schedule id for the logical device.
       */
      bool checkStartTimer(const string& deviceName, unsigned long timerId);
      /*
       * returns true if the timerId is a stopTimer for the given logical device
       * Also resets the current schedule id for the logical device.
       */
      bool checkStopTimer(const string& deviceName, unsigned long timerId);
      
      /*
       * returns true if the timerId is a startTimer for the maintenance schedule
       */
      bool checkMaintenanceStartTimer(unsigned long timerId, MaintenanceScheduleIterT& scheduleIt);
      /*
       * returns true if the timerId is a stopTimer for the maintenance schedule
       */
      bool checkMaintenanceStopTimer(unsigned long timerId, MaintenanceScheduleIterT& scheduleIt);
      
      
      void sendWGsettings();
      void getRequiredResources(list<string>& requiredResources, int rack, int subrack, int board, int ap, int rcu);

      static string m_schedulerTaskName;


      AVTPropertySetAnswer  m_propertySetAnswer;
      GCFMyPropertySet      m_properties;
      GCFApc                m_apcLDS;
      bool                  m_initialized;
  //    GCFPort               m_beamServer;
      GCFPort*              m_pBeamServer;
      double                m_WGfrequency;
      unsigned int          m_WGamplitude;
      unsigned int          m_WGsamplePeriod;

      LogicalDeviceMapT       m_logicalDeviceMap;
      LogicalDeviceScheduleT  m_logicalDeviceSchedule;
      MaintenanceScheduleT    m_maintenanceSchedule;
      GCFPort                 m_timerPort;
      
      AVTResourceManagerPtr   m_resourceManager;
      
      PropertySetMapT         m_propsetVTmap;
      PropertySetMapT         m_propsetSBFmap;
  };
};
#endif
