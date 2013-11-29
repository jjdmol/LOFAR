//
//  THPVSSBridge.h: Definition of the TestHarness - PVSS Bridge task class.
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#ifndef THPVSSBridge_H
#define THPVSSBridge_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/TM/GCF_Control.h>

#include <APL/APLCommon/PropertySetAnswerHandlerInterface.h>
#include <APL/APLCommon/PropertySetAnswer.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

#include "PropertyProxy.h"

namespace LOFAR
{
  
namespace MACTest
{
  
/**
 * The THPVSSBridge task receives events from the TestHarness statemachines.
 * It accesses the PVSS database based on the requests received
 */
class THPVSSBridge : public GCF::TM::GCFTask,
                            APLCommon::PropertySetAnswerHandlerInterface
{
 public:

  /**
   * The constructor for the THPVSSBridge task.
   */
  THPVSSBridge(string name);
  virtual ~THPVSSBridge();

  /**
  * PropertySetAnswerHandlerInterface method
  */
  virtual void handlePropertySetAnswer(GCF::TM::GCFEvent& answer);

  /**
   * The initial state handler. This handler is passed to the FTask constructor
   * to indicate that the F_INIT event which starts the state machine is handled
   * by this handler.
   * @param e The event that was received and needs to be handled by the state
   * handler.
   * @param p The port interface (see @a FPortInterface) on which the event
   * was received.
   */
  GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);

  /**
   * The "connected" state is reached when a client is connected.
   */
  GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface& p);

  void proxyPropSubscribed(const string& propName);
  void proxyPropSubscriptionLost(const string& propName);
  void proxyPropUnsubscribed(const string& propName);
  void proxyPropValueGet(const string& propName,const string& value);
  void proxyPropValueChanged(const string& propName,const string& value);
  void proxyPropValueSet(const string& propName);

protected:
  // protected copy constructor
  THPVSSBridge(const THPVSSBridge&);
  // protected assignment operator
  THPVSSBridge& operator=(const THPVSSBridge&);

private:
  typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet>   TMyPropertySetPtr;
  typedef boost::shared_ptr<GCF::PAL::GCFExtPropertySet>  TExtPropertySetPtr;
  typedef std::map<string,TMyPropertySetPtr>              TMyPropertySetMap;
  typedef std::map<string,TExtPropertySetPtr>             TExtPropertySetMap;
  
  void _flushSubscriptions();
  void _removeProxySubscription(const string& propName);

  APLCommon::PropertySetAnswer  m_propertySetAnswer;
  TMyPropertySetMap             m_myPropertySets;
  TExtPropertySetMap            m_extPropertySets;
  
  string                        m_serverPortName;
  GCF::TM::GCFTCPPort           m_serverPort;      // TH communication
  
  PropertyProxy                 m_propertyProxy;
  vector<string>                m_proxySubscriptions;

  ALLOC_TRACER_CONTEXT  
};

}; // namespace MACTest
}; // namespace LOFAR

#endif
