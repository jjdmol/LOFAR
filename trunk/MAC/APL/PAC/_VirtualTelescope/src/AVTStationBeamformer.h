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
#include <lofar_config.h>
#include <Common/lofar_string.h>
//# GCF Includes
#include <GCF/GCF_Port.h>
//# VirtualTelescope Includes
#include "AVTLogicalDevice.h"

// forward declaration

/**
 * This is the main task of the VirtualTelescope process. 
 * @todo 
 */

class AVTStationBeamformer : public AVTLogicalDevice
{
  public:

    explicit AVTStationBeamformer(const string& name, 
                                 const TPropertySet& primaryPropertySet,
                                 const string& APCName,
                                 const string& APCScope); 
    virtual ~AVTStationBeamformer();

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
     * returns true if the prepairing state has finished
     */
    virtual GCFEvent::TResult concrete_prepairing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished);
    /**
     * returns true if the releasing state has finished
     */
    virtual GCFEvent::TResult concrete_releasing_state(GCFEvent& e, GCFPortInterface& p, bool& stateFinished);
    
    virtual void concreteClaim(GCFPortInterface& port);
    virtual void concretePrepare(GCFPortInterface& port);
    virtual void concreteResume(GCFPortInterface& port);
    virtual void concreteSuspend(GCFPortInterface& port);
    virtual void concreteRelease(GCFPortInterface& port);
    virtual void concreteDisconnected(GCFPortInterface& port);
    
  private:
    /**
     * returns true if the specified port is the BeamServer SAP
     */
    bool _isBeamServerPort(GCFPortInterface& port);

    // The BeamServer SAP
    GCFPort m_beamServer;
};
#endif
