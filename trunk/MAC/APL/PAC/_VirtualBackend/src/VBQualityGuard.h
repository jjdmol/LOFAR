//#  VBQualityGuard.h: factory class for Virtual Backends.
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

#ifndef VBQUALITYGUARD_H
#define VBQUALITYGUARD_H

//# Includes
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <APL/NodeManager.h>
#include <GCF/PAL/GCF_Answer.h>

//# local includes
//# Common Includes

// forward declaration

namespace LOFAR
{  
  namespace GCF
  {
    namespace TM
    {
class GCFEvent;      
    }
  }
  namespace AVB // A)pplication layer V)irtual B)ackend
  {

#define NR_OF_NODES ((double) (_functStateProps.size() + _nrOfNotMonitoredNodes))

class VBQualityGuard;
class VirtualBackendLD;
class VBFuncStateProperty;

class VBQAnswer : public GCF::PAL::GCFAnswer
{
  public:
    VBQAnswer(VBQualityGuard& vbqg);
    void handleAnswer (GCF::TM::GCFEvent& answer);
    
  private:
    VBQualityGuard& _vbqg;  
};

class VBQualityGuard : public ANM::NodeManagerInterface
{
  public:
    VBQualityGuard(VirtualBackendLD& ld);
    virtual ~VBQualityGuard();
    
  public: 
    void monitorNodes(ANM::TNodeList& nodesToMonitor);
    void stopMonitoring();
    unsigned int getQuality() const;
    bool isQualityLow() const;
    void valueChanged(const string& propName, char value);
    void answer(GCF::TM::GCFEvent& answer);    

  protected: // implemenation of abstract NodeManagerInterface methods
    void    nodesClaimed(ANM::TNodeList& newClaimedNodes, 
                         ANM::TNodeList& releasedNodes,
                         ANM::TNodeList& faultyNodes);
    void    nodesReleased();
     
  protected:
    // protected copy constructor
    VBQualityGuard(const VBQualityGuard&);
    // protected assignment operator
    VBQualityGuard& operator=(const VBQualityGuard&);

  private: // helper methods
    void checkQuality();
    VBFuncStateProperty* findFuncStatePropObject(const string& propName);
    
  private: // data members
    VirtualBackendLD&           _vbLD;
    unsigned int                _currQuality;
    GCF::PAL::GCFExtPropertySet _cepAppProperties;
    typedef map<string /*nodename*/, VBFuncStateProperty*> TFuncStateProps;
    TFuncStateProps             _functStateProps;
    VBQAnswer                   _answer;
    ANM::NodeManager            _nodeManager;

 private: // admin. members
    ANM::TNodeList              _faultyNodes;
    unsigned int                _nrOfNotMonitoredNodes;
    unsigned int                _nrOfPendingSubscriptions;
    bool                        _lowQualityReported;
    
    ALLOC_TRACER_CONTEXT  
};

inline VBQAnswer::VBQAnswer(VBQualityGuard& vbqg) :
  _vbqg(vbqg)
{
}

inline unsigned int VBQualityGuard::getQuality() const
{
  return (unsigned int) (_currQuality / NR_OF_NODES);
}

  } // namespace AVB
} // namespace LOFAR
#endif
