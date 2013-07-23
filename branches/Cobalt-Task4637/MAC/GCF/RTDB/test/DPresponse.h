//
//  DPresponse.h: Concrete PVSSresponse class definition.
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
	using PVSS::GCFPVDynArr;
	using PVSS::PVSSresponse;
	using PVSS::PVSSresult;
	namespace RTDB {

extern int		gCreateCounter;
extern int		gSetCounter;
extern int		gGetCounter;
extern int		gQryCounter;
extern int		gQueryID;

class DPresponse : public PVSSresponse
{
public:
    DPresponse() {};
    virtual ~DPresponse() {};

protected:
    virtual void dpCreated			(const string& propName, PVSSresult		result);
    virtual void dpDeleted			(const string& propName, PVSSresult		result);
    virtual void dpeSubscribed		(const string& propName, PVSSresult		result);
    virtual void dpeSubscriptionLost(const string& /*propNm*/, PVSSresult	result);
    virtual void dpeUnsubscribed	(const string& propName, PVSSresult		result);
    virtual void dpeValueGet		(const string& propName, PVSSresult		result, const PVSS::GCFPValue& value);
    virtual void dpeValueChanged	(const string& propName, PVSSresult		result, const PVSS::GCFPValue& value);
    virtual void dpeValueSet		(const string& propName, PVSSresult		result);
    virtual void dpQuerySubscribed	(uint32 queryID, PVSSresult		result);
    virtual void dpQueryUnsubscribed(uint32 queryID, PVSSresult		result);
	virtual void dpQueryChanged		(uint32 queryID, 		 PVSSresult result,
									  const GCFPVDynArr&	DPnames,
									  const GCFPVDynArr&	DPvalues,
									  const GCFPVDynArr&	DPtypes);
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
