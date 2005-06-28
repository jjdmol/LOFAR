//#  AVTStationBeamformer.h: implementation of the Beamformer logical device.
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

#ifndef AVTStationBeamformer_H
#define AVTStationBeamformer_H

//# Includes
//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <set>
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/TM/GCF_Port.h>
//# VirtualTelescope Includes
#include "AVTLogicalDevice.h"

// forward declaration
namespace LOFAR
{
  
namespace AVT
{
  class AVTStationBeamformer : public AVTLogicalDevice
  {
    public:

      explicit AVTStationBeamformer(string& name,
                                    const string& scope,
                                    const string& APCName,
                                    string& beamServerPortName); 
      virtual ~AVTStationBeamformer();

      GCF::TM::GCFPort& getBeamServerPort(); // increment 1 only!!! 
      virtual bool isPrepared(vector<string>& parameters);
      bool checkQualityRequirements();

    protected:
      // protected default constructor
      AVTStationBeamformer();
      // protected copy constructor
      AVTStationBeamformer(const AVTStationBeamformer&);
      // protected assignment operator
      AVTStationBeamformer& operator=(const AVTStationBeamformer&);

      /**
      * initializes the SAP and SPP ports
      */
      virtual GCF::TM::GCFEvent::TResult concrete_initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * returns true if claiming has finished
      */
      virtual GCF::TM::GCFEvent::TResult concrete_claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished);
      /**
      * returns true if the preparing state has finished
      */
      virtual GCF::TM::GCFEvent::TResult concrete_preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished, bool& error);
      /**
      * concrete implementation of the active state
      */
      virtual GCF::TM::GCFEvent::TResult concrete_active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      /**
      * returns true if the releasing state has finished
      */
      virtual GCF::TM::GCFEvent::TResult concrete_releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, bool& stateFinished);

      virtual void handlePropertySetAnswer(GCF::TM::GCFEvent& answer);

      virtual void concreteClaim(GCF::TM::GCFPortInterface& port);
      virtual void concretePrepare(GCF::TM::GCFPortInterface& port,string& parameters);
      virtual void concreteResume(GCF::TM::GCFPortInterface& port);
      virtual void concreteSuspend(GCF::TM::GCFPortInterface& port);
      virtual void concreteRelease(GCF::TM::GCFPortInterface& port);
      virtual void concreteDisconnected(GCF::TM::GCFPortInterface& port);

    private:
      /**
      * returns true if the specified port is the BeamServer SAP
      */
      bool _isBeamServerPort(GCF::TM::GCFPortInterface& port);
      int convertDirection(const string type) const;

      // The BeamServer SAP
      GCF::TM::GCFPort         m_beamServer;
      bool            m_beamServerConnected;
      int             m_numAPCsLoaded;
      const int       m_maxAPCs;

      time_t          m_startTime;
      time_t          m_stopTime;
      double          m_frequency;
      std::set<int>   m_subbands;

      int             m_directionType;
      double          m_directionAngle1;
      double          m_directionAngle2;
      int             m_beamID;

      ALLOC_TRACER_CONTEXT  
  };
};//AVT
};//LOFAR
#endif
