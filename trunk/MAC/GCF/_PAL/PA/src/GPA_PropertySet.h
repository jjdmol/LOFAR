//#  GPA_PropertySet.h: manages the properties with its use count
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

#ifndef GPA_PROPERTYSET_H
#define GPA_PROPERTYSET_H

#include <GPA_Defines.h>
#include <GSA_Service.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFPortInterface;
  }
  namespace PAL 
  {

class GPAPropSetSession;

/**
   This class manages the properties with its use count, which are created 
   (resp. deleted) by means of the base class GSAService.
*/

class GPAController;

class GPAPropertySet : public GSAService
{
  public:
  	GPAPropertySet(GPAPropSetSession& session):
      _session(session),
      _name(""),
      _type(""),
      _category(Common::PS_CAT_TEMPORARY) {}
  	virtual ~GPAPropertySet() {}
    
    TPAResult enable(string& name, string& type, Common::TPSCategory category);
    TPAResult disable(bool& mustWait);
    TPAResult load(bool& mustWait);
    TPAResult unload(bool& mustWait);
    
    string name() const;
    string type() const;
    Common::TPSCategory category() const;

  protected:
    void dpCreated(const string& dpName);
    void dpDeleted(const string& dpName);
    void dpeValueGet(const string& /*dpeName*/, const Common::GCFPValue& /*value*/) {}; 
    void dpeValueChanged(const string& /*dpeName*/, const Common::GCFPValue& /*value*/) {};
    void dpeValueSet(const string& /*dpeName*/) {};
    void dpeSubscribed(const string& /*dpeName*/) {};
    void dpeSubscriptionLost (const string& /*dpeName*/) {};
    void dpeUnsubscribed(const string& /*dpeName*/) {};

  private: // data members
    GPAPropSetSession&  _session;
    string              _name;
    string              _type;
    Common::TPSCategory _category;
    
  private: // admin. data members
    unsigned int _counter;
};

inline string GPAPropertySet::name() const
{
  return _name;
}

inline string GPAPropertySet::type() const
{
  return _type;
}

inline Common::TPSCategory GPAPropertySet::category() const
{
  return _category;
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
