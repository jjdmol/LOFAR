//
//  Service.h: Concrete GSAService class definition.
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

#include <GSA_Service.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {
class Service : public GSAService
{
  public:
    Service() {};
    virtual ~Service() {};

    virtual TSAResult dpCreate(const string& propName, 
                               const string& macType);
    virtual TSAResult dpDelete(const string& propName);
    virtual TSAResult dpeSubscribe(const string& propName);
    virtual TSAResult dpeUnsubscribe(const string& propName);
    virtual TSAResult dpeGet(const string& propName);
    virtual TSAResult dpeSet(const string& propName, const Common::GCFPValue& value, double timestamp, bool wantAnswer = false);
    virtual bool dpeExists(const string& propName);
    
  protected:
    virtual void dpCreated(const string& propName);
    virtual void dpDeleted(const string& propName);
    virtual void dpeSubscribed(const string& propName);
    virtual void dpeSubscriptionLost(const string& /*propName*/) {;}
    virtual void dpeUnsubscribed(const string& propName);
    virtual void dpeValueGet(const string& propName, const Common::GCFPValue& value);
    virtual void dpeValueChanged(const string& propName, const Common::GCFPValue& value);
    virtual void dpeValueSet(const string& propName);
  
  private:
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
