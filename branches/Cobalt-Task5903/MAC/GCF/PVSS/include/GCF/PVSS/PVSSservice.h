//#  PVSSservice.h: 
//#
//#  Copyright (C) 2002-2008
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

#ifndef  PVSS_SERVICE_H
#define  PVSS_SERVICE_H

#include <GCF/PVSS/PVSSresult.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <Common/LofarTypes.h>

// PVSS forwards
class DpMsgAnswer;
class DpHLGroup;
class Variable;
class CharString;
class DpIdentifier;

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

class GSASCADAHandler;
class GSAWaitForAnswer;
class PVSSresponse;


string	PVSSerrstr (PVSSresult		resultNr);

// This class is the interface to the PVSS database. It contains the
// 'initiating' call only. The result of the (async) action is 
// routed to the PVSSresponse class.
class PVSSservice
{
public:
    PVSSservice (PVSSresponse*	response);
    ~PVSSservice ();

    PVSSresult dpCreate		 (const string& dpName, const string& typeName);
    PVSSresult dpDelete		 (const string& dpName);

    PVSSresult dpeSubscribe	 (const string& dpeName);
    PVSSresult dpeUnsubscribe(const string& dpeName);
    PVSSresult dpeGet		 (const string& dpeName);
    PVSSresult dpeSet		 (const string& 				dpeName, 
							  const GCFPValue&				value, 
							  double						timestamp = 0.0,
							  bool   						wantAnswer = true);
    PVSSresult dpeSetMultiple(const string&					dpName,
							  vector<string>				dpeNames, 
							  vector<GCFPValue*>			values, 
							  double						timestamp = 0.0,
							  bool   						wantAnswer = true);
    PVSSresult dpQuerySubscribeSingle(const string& queryFrom, 
                                      const string& queryWhere);
    PVSSresult dpQuerySubscribeAll   (const string& queryFrom, 
                                      const string& queryWhere);
    PVSSresult dpQueryUnsubscribe	(uint32 queryId);

	void doWork();	// Needed by RTDB_Propertyset.
    
private: 
	// methods
    // interface for GSAWaitForAnswer
    void handleHotLink (const DpMsgAnswer&	answer, const GSAWaitForAnswer& wait);
    void handleHotLink (const DpHLGroup& 	group,  const GSAWaitForAnswer& wait);
	void _processQueryResult(Variable*	firstVar,	
							 Variable*	secondVar,
							 bool		passSeperate);
    friend class GSAWaitForAnswer;
  
    // helper methods to convert PVSS dpTypes to MAC types and visa versa
    PVSSresult convertPVSSToMAC (const Variable&		variable, 
                                 GCFPValue**			pMacValue) const;
                          
    PVSSresult convertMACToPVSS (const GCFPValue&			macValue, 
                                 Variable** 				pVar,
                                 const DpIdentifier&		dpId) const;
    bool getPVSSType (TMACValueType macType, 
                      CharString& 			pvssTypeName) const;

    // helper methods
    PVSSresult getDpId 		(const string& dpName,  		  DpIdentifier& dpId) const;
    void convPropToDpConfig (const string& dpeName, 		  string& 	  	pvssDpName, bool willReadValue);
    void convDpConfigToProp (const string& pvssDPEConfigName, string& 		dpeName);    

    void convAndForwardValueChange(const DpIdentifier& dpId, const Variable& pvssVar);
    
	// data members    
    GSAWaitForAnswer*	itsWFA;
    GSASCADAHandler*  	itsSCADAHandler;
	PVSSresponse*		itsResponse;
};                                 


  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
