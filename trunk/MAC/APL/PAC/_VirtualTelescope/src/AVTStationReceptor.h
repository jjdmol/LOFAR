//#  AVTStationReceptor.h: implementation of the Virtual Telescope logical device.
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

#ifndef AVTStationReceptor_H
#define AVTStationReceptor_H

//# Includes
//# Common Includes
#include <time.h>
#include <Common/lofar_string.h>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/TM/GCF_Port.h>
#include <GCF/PAL/GCF_ExtProperty.h>

#include <APLCommon/APLInterTaskPort.h>

//# VirtualTelescope Includes
#include "AVTLogicalDevice.h"

namespace LOFAR
{
  
namespace AVT
{
  class AVTStationReceptor : public AVTLogicalDevice
  {
    public:

      explicit AVTStationReceptor(string& name,
                                  const string& scope,
                                  const string& APCName,
                                  const std::list<TPropertyInfo>& requiredResources); 
      virtual ~AVTStationReceptor();

      bool checkQualityRequirements();
      void setStartTime(const time_t startTime);
      void setStopTime(const time_t stopTime);
      void setFrequency(const double frequency);

    protected:
      // protected default constructor
      AVTStationReceptor();
      // protected copy constructor
      AVTStationReceptor(const AVTStationReceptor&);
      // protected assignment operator
      AVTStationReceptor& operator=(const AVTStationReceptor&);

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
      typedef std::map<std::string,boost::shared_ptr<GCFExtProperty> > TExtPropertyMap;

      time_t            m_startTime;
      time_t            m_stopTime;
      double            m_frequency;
      TExtPropertyMap             m_requiredResources;
      std::map<std::string,bool>  m_requiredResourcesStatus;
  };
};//AVT
};//LOFAR

#endif
