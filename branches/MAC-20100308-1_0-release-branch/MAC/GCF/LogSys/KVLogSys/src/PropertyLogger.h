//#  PropertyLogger.h: 
//#
//#  Copyright (C) 2002-2003
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

#ifndef PROPERTYLOGGER_H
#define PROPERTYLOGGER_H

#include <GCF/PAL/GCF_PropertyProxy.h>
#include <GCF/TM/GCF_Fsm.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace TM
  {
class GCFPortInterface;
  }
  namespace LogSys
  {
class KeyValueLoggerDaemon;

class PropertyLogger : public PAL::GCFPropertyProxy
{
  public:
    PropertyLogger(KeyValueLoggerDaemon& daemon);
    virtual ~PropertyLogger() {};
    
    void skipUpdatesFrom(uint8 manNum, const TM::GCFPortInterface& p);
    void clientGone(const TM::GCFPortInterface& p);

  protected:    
    void propSubscribed(const string& /*propName*/) {}
    void propSubscriptionLost(const string& /*propName*/) {}
    void propUnsubscribed(const string& /*propName*/) {}
    void propValueGet(const string& /*propName*/, const Common::GCFPValue& /*value*/) {} 
    void propValueChanged(const string& propName, const Common::GCFPValue& value);
    void propValueSet(const string& /*propName*/) {}
  
  private:
    KeyValueLoggerDaemon& _daemon;
    typedef map<const TM::GCFPortInterface*, uint8> TManNumToSkip;
    TManNumToSkip _manNumToSkip;
    static TM::GCFDummyPort _dummyPort;
};
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
#endif
