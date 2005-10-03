//#  StationReceptorGroup.h: handles all events for a task.
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

#ifndef StationReceptorGroup_H
#define StationReceptorGroup_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_ExtPropertySet.h>

//# local includes
#include <APLCommon/LogicalDevice.h>

//# Common Includes

//# ACC Includes

// forward declaration

namespace LOFAR
{
  
namespace ASR
{

  class StationReceptorGroup : public APLCommon::LogicalDevice
  {
    public:
      // Logical Device version
      static const string SRG_VERSION;

      // property defines

      explicit StationReceptorGroup(const string& taskName, const string& parameterFile, GCF::TM::GCFTask* pStartDaemon);
      virtual ~StationReceptorGroup();

    protected:
      // protected default constructor
      StationReceptorGroup();
      // protected copy constructor
      StationReceptorGroup(const StationReceptorGroup&);
      // protected assignment operator
      StationReceptorGroup& operator=(const StationReceptorGroup&);

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
      virtual void concreteAddExtraKeys(ACC::APS::ParameterSet& psSubset);
      
    protected:    

    private:
      bool _isCALclientPort(GCF::TM::GCFPortInterface& port) const;
      void getRCURelativeNumbers(int rcuNr,int& rackRelativeNr,int& subRackRelativeNr,int& boardRelativeNr,int& apRelativeNr,int& rcuRelativeNr);
      int getRCUHardwareNr(const string& property);
      bool rcuPropsAvailable();
      bool checkQuality();
      uint8 getRcuControlValue(const string& bandselection);

      typedef map<uint16,boost::shared_ptr<GCF::PAL::GCFExtPropertySet> > TRCUMap;
      typedef map<uint16,bool> TRCUFunctionalityMap;
    
      static string     m_CALserverName;
      GCF::TM::GCFPort  m_CALclient;
      TRCUMap           m_rcuMap;
      TRCUFunctionalityMap m_rcuFunctionalityMap;

      uint16  m_n_racks;
      uint16  m_n_subracks_per_rack;
      uint16  m_n_boards_per_subrack;
      uint16  m_n_aps_per_board;
      uint16  m_n_rcus_per_ap;
      uint16  m_n_rcus;
      long    m_rcusPropertiesAvailableTimer;
      long    m_connectTimer;
      
      ALLOC_TRACER_CONTEXT  
  };
};//APLCommon
};//LOFAR
#endif
