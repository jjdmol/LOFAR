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
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/GCF_Port.h>
//# VirtualTelescope Includes
#include "AVTLogicalDevice.h"

// forward declaration
namespace AVT
{
  class AVTStationBeamformer : public AVTLogicalDevice
  {
    public:

      explicit AVTStationBeamformer(string& name, 
                                    const TPropertySet& primaryPropertySet,
                                    const string& APCName,
                                    const string& APCScope,
                                    string& beamServerPortName); 
      virtual ~AVTStationBeamformer();

      GCFPort& getBeamServerPort(); // increment 1 only!!!
      virtual bool isPrepared(vector<string>& parameters);

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
      * returns true if the releasing state has finished
      */
      virtual GCFEvent::TResult concrete_releasing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished);

      virtual void handlePropertySetAnswer(GCFEvent& answer);
      virtual void handleAPCAnswer(GCFEvent& answer);

      virtual void concreteClaim(GCFPortInterface& port);
      virtual void concretePrepare(GCFPortInterface& port,string& parameters);
      virtual void concreteResume(GCFPortInterface& port);
      virtual void concreteSuspend(GCFPortInterface& port);
      virtual void concreteRelease(GCFPortInterface& port);
      virtual void concreteDisconnected(GCFPortInterface& port);

    private:
      /**
      * returns true if the specified port is the BeamServer SAP
      */
      bool _isBeamServerPort(GCFPortInterface& port);
      int convertDirection(const string type) const;

      typedef map<int,boost::shared_ptr<GCFApc> > SubbandAPCMapT;

      // The BeamServer SAP
      GCFPort         m_beamServer;
      bool            m_beamServerConnected;
      int             m_numAPCsLoaded;
      const int       m_maxAPCs;

      time_t          m_startTime;
      time_t          m_stopTime;
      double          m_frequency;
      SubbandAPCMapT  m_subbands;
      GCFApc          m_APCBeamServerStatistics;
      int             m_directionType;
      double          m_directionAngle1;
      double          m_directionAngle2;
      int             m_beamID;
  };
};
#endif
