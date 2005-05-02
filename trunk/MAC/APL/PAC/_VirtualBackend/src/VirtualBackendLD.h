//#  VirtualBackendLD.h: handles all events for a task.
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

#ifndef VIRTUALBACKENDLD_H
#define VIRTUALBACKENDLD_H

//# Includes

//# GCF Includes
#include <GCF/PAL/GCF_ExtPropertySet.h>

//# local includes
#include <APLCommon/LogicalDevice.h>
#include <CEPApplicationManager.h>
#include <VBQualityGuard.h>

//# Common Includes

//# ACC Includes
#include <ACC/ParameterCollection.h>

// forward declaration

namespace LOFAR
{    
  namespace AVB
  {

class VirtualBackendLD : public APLCommon::LogicalDevice,                         
                         public CEPApplicationManagerInterface
{
  public:
    // Logical Device version
    static const string VB_VERSION;

    explicit VirtualBackendLD(const string& taskName, 
                              const string& parameterFile,
                              GCF::TM::GCFTask* pStartDaemon);
    virtual ~VirtualBackendLD();

  protected:
    // protected default constructor
    VirtualBackendLD();
    // protected copy constructor
    VirtualBackendLD(const VirtualBackendLD&);
    // protected assignment operator
    VirtualBackendLD& operator=(const VirtualBackendLD&);

    virtual void concrete_handlePropertySetAnswer(GCF::TM::GCFEvent& answer);
    /**
    * Initial state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);
    /**
    * Idle state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_idle_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);
    /**
    * Claiming state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);
    /**
    * Claimed state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_claimed_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);
    /**
    * Preparing state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);
    /**
    * active state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLDResult& errorCode);
    /**
    * Releasing state additional behaviour must be implemented in the derived classes. 
    */
    virtual GCF::TM::GCFEvent::TResult concrete_releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode);

    /**
    * Implementation of the Claim method is done in the derived classes. 
    */
    virtual void concreteClaim(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Prepare method is done in the derived classes. 
    */
    virtual void concretePrepare(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Resume method is done in the derived classes. 
    */
    virtual void concreteResume(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Suspend method is done in the derived classes. 
    */
    virtual void concreteSuspend(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Release method is done in the derived classes. 
    */
    virtual void concreteRelease(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Disconnected handler is done in the derived classes. 
    */
    virtual void concreteParentDisconnected(GCF::TM::GCFPortInterface& port);
    /**
    * Implementation of the Disconnected handler is done in the derived classes. 
    */
    virtual void concreteChildDisconnected(GCF::TM::GCFPortInterface& port);
    virtual void concreteHandleTimers(GCF::TM::GCFTimerEvent& timerEvent, GCF::TM::GCFPortInterface& port);

  private: // interface for quality guard
    friend class VBQualityGuard;
    void qualityGuardStarted();
    void qualityGuardStopped();
    void lowQuality(ANM::TNodeList& faultyNodes);
    void qualityChanged();
    
  protected: // implemenation of abstract CEPApplicationManagerInterface methods
    void    appBooted(uint16 result);
    void    appDefined(uint16 result);
    void    appInitialized(uint16 result);
    void    appRunDone(uint16 result);
    void    appPaused(uint16 result);
    void    appQuitDone(uint16 result);
    void    appSnapshotDone(uint16 result);
    void    appRecovered(uint16 result);
    void    appReinitialized(uint16 result);
    void    appReplaced(uint16 result);
    string  appSupplyInfo(const string& keyList);
    void    appSupplyInfoAnswer(const string& answer);
  
  private:
    CEPApplicationManager     _cepApplication;
    VBQualityGuard            _qualityGuard;
    ACC::ParameterCollection  _cepAppParams;
    ANM::TNodeList            _neededNodes;
    unsigned long             _rstoID;
  
    ALLOC_TRACER_CONTEXT  
};
  } // namespace APLCommon
} // namespace LOFAR
#endif
