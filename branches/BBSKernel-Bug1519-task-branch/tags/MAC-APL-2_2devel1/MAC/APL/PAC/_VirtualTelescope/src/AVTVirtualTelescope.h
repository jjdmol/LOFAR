//#  AVTVirtualTelescope.h: implementation of the Virtual Telescope logical device.
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

#ifndef AVTVirtualTelescope_H
#define AVTVirtualTelescope_H

//# Includes
//# Common Includes
#include <time.h>
#include <Common/lofar_string.h>
//# GCF Includes
#include <GCF/TM/GCF_Port.h>

#include <APLCommon/APLInterTaskPort.h>

//# VirtualTelescope Includes
#include "AVTLogicalDevice.h"

namespace AVT
{
  // forward declaration
  class AVTStationBeamformer;
  class AVTStationReceptorGroup;

  class AVTVirtualTelescope : public AVTLogicalDevice
  {
    public:

      explicit AVTVirtualTelescope(string& name,
                                  const string& scope,
                                  const string& APCName,
                                  AVTStationBeamformer& sbf, 
                                  AVTStationReceptorGroup& srg); 
      virtual ~AVTVirtualTelescope();

      bool checkQualityRequirements();
      void setStartTime(const time_t startTime);
      void setStopTime(const time_t stopTime);
      void setFrequency(const double frequency);

    protected:
      // protected default constructor
      AVTVirtualTelescope();
      // protected copy constructor
      AVTVirtualTelescope(const AVTVirtualTelescope&);
      // protected assignment operator
      AVTVirtualTelescope& operator=(const AVTVirtualTelescope&);

      /**
      * initializes the SAP and SPP ports
      */
      virtual GCFEvent::TResult concrete_initial_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * returns true if claiming has finished
      */
      virtual GCFEvent::TResult concrete_claiming_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished);
      /**
      * returns true if the preparing state has finished
      */
      virtual GCFEvent::TResult concrete_preparing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished, bool& error);
      /**
      * concrete implementation of the active state
      */
      virtual GCFEvent::TResult concrete_active_state(GCFEvent& e, GCFPortInterface& p);
      /**
      * returns true if the releasing state has finished
      */
      virtual GCFEvent::TResult concrete_releasing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished);

      virtual void handlePropertySetAnswer(GCFEvent& answer);

      virtual void concreteClaim(GCFPortInterface& port);
      virtual void concretePrepare(GCFPortInterface& port,string& parameters);
      virtual void concreteResume(GCFPortInterface& port);
      virtual void concreteSuspend(GCFPortInterface& port);
      virtual void concreteRelease(GCFPortInterface& port);
      virtual void concreteDisconnected(GCFPortInterface& port);

    private:
      /**
      * returns true if the specified port is the BeamFormer logical device SAP
      */
      bool _isBeamFormerClient(GCFPortInterface& port) const;
      bool _isStationReceptorGroupClient(GCFPortInterface& port) const;
      
      bool allInState(GCFPortInterface& port, TLogicalDeviceState state, bool requireSlaveActive) const;

      // The BeamFormer task    
      AVTStationBeamformer&     m_stationBeamformer;
      // The StationReceptorGroup task
      AVTStationReceptorGroup&  m_stationReceptorGroup;
      
      // The BeamFormer SAP
  //    GCFPort m_beamFormerClient;
      APLInterTaskPort    m_beamFormerClient;
      bool                m_beamFormerConnected;
      APLInterTaskPort    m_stationReceptorGroupClient;
      bool                m_stationReceptorGroupConnected;
      long                m_qualityCheckTimerId;
      GCFPort             m_qualityCheckTimerPort;
      
      
      time_t              m_startTime;
      time_t              m_stopTime;
      double              m_frequency;

  };
  
};
#endif
