//#  AVTStationReceptorGroup.h: implementation of the Virtual Telescope logical device.
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

#ifndef AVTStationReceptorGroup_H
#define AVTStationReceptorGroup_H

//# Includes
#include <vector>
#include <boost/shared_ptr.hpp>

//# Common Includes
#include <time.h>
#include <Common/lofar_string.h>
//# GCF Includes
#include <GCF/TM/GCF_Port.h>

#include <APLCommon/APLInterTaskPort.h>

//# VirtualTelescope Includes
#include "AVTDefines.h"
#include "AVTLogicalDevice.h"

namespace AVT
{
  // forward declaration
  class AVTStationReceptor;

  class AVTStationReceptorGroup : public AVTLogicalDevice
  {
    public:

      explicit AVTStationReceptorGroup(string& name,
                                       const string& scope,
                                       const string& APCName,
                                       std::vector<boost::shared_ptr<AVTStationReceptor> >& rcus); 
      virtual ~AVTStationReceptorGroup();

      virtual bool isPrepared(vector<string>& parameters);
      bool checkQualityRequirements(int maxFailedResources);
      void setStartTime(const time_t startTime);
      void setStopTime(const time_t stopTime);
      void setFrequency(const double frequency);

    protected:
      // protected default constructor
      AVTStationReceptorGroup();
      // protected copy constructor
      AVTStationReceptorGroup(const AVTStationReceptorGroup&);
      // protected assignment operator
      AVTStationReceptorGroup& operator=(const AVTStationReceptorGroup&);

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
      struct TStationReceptorConnection
      {
        // constructor
        TStationReceptorConnection( boost::shared_ptr<AVTStationReceptor> _rcu,
                                    GCFTask&                              _containerTask, 
                                    string&                               _name, 
                                    GCFPort::TPortType                    _type, 
                                    int                                   _protocol,
                                    bool                                  _connected);

        boost::shared_ptr<AVTStationReceptor>   rcu;
        boost::shared_ptr<APLInterTaskPort>     clientPort;
        bool                                    connected;
      };
      typedef std::vector<TStationReceptorConnection>  TStationReceptorVector;
      typedef TStationReceptorVector::iterator         TStationReceptorVectorIter;
      
      bool isStationReceptorClient(GCFPortInterface& port);
      bool setReceptorConnected(GCFPortInterface& port, bool connected);
      bool allReceptorsConnected();
      bool setReceptorState(GCFPortInterface& port, TLogicalDeviceState state);
      bool allReceptorsInState(TLogicalDeviceState state);
      void sendToAllReceptors(GCFEvent& event);

      // The StationReceptor tasks    
      TStationReceptorVector  m_stationReceptors;
      time_t                  m_startTime;
      time_t                  m_stopTime;
      double                  m_frequency;
  };
};

#endif
