//
//  Response.h: Concrete PVSSresponse class definition.
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

#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

class Response : public PVSSresponse
{
public:
    Response() {};
    virtual ~Response() {};

protected:
    virtual void dpCreated			(const string& propName, PVSSresult		result);
    virtual void dpDeleted			(const string& propName, PVSSresult		result);
    virtual void dpeSubscribed		(const string& propName, PVSSresult		result);
    virtual void dpeSubscriptionLost(const string& /*propNm*/, PVSSresult	result);
    virtual void dpeUnsubscribed	(const string& propName, PVSSresult		result);
    virtual void dpeValueGet		(const string& propName, PVSSresult		result, const Common::GCFPValue& value);
    virtual void dpeValueChanged	(const string& propName, PVSSresult		result, const Common::GCFPValue& value);
    virtual void dpeValueSet		(const string& propName, PVSSresult		result);
    virtual void dpQuerySubscribed	(uint32 queryId, PVSSresult		result);
};

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
