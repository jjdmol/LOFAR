//#  GSA_Service.h: 
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

#ifndef  GSA_SERVICE_H
#define  GSA_SERVICE_H

#include <GSA_Defines.h>
#include <GCF/GCF_PValue.h>

// PVSS forwards
class DpMsgAnswer;
class DpHLGroup;
class Variable;
class CharString;
class DpIdentifier;

namespace LOFAR {
 namespace GCF {
  namespace PAL {

class GSASCADAHandler;
class GSAWaitForAnswer;


string	GSAerror (TSAResult		resultNr);

/**
 * This is the abstract class, which provides the possibility to invoke all 
 * typical SCADA methods and handle their responses. By means of specialisation 
 * of this abstract class and instantiation on different places in the same 
 * application it becomes possible to subscribe (for instance) on the same 
 * property, which results in a separate valueChanged response per specialised 
 * instance. 
 * @see GSAWaitForAnswer
 */
class GSAService
{
public:
    GSAService ();
    virtual ~GSAService ();

protected:
    virtual TSAResult dpCreate		 (const string& dpName, const string& typeName);
    virtual TSAResult dpDelete		 (const string& dpName);
    virtual TSAResult dpeSubscribe	 (const string& dpeName);
    virtual TSAResult dpeUnsubscribe (const string& dpeName);
    virtual TSAResult dpeGet		 (const string& dpeName);
    virtual TSAResult dpeSet		 (const string& 			dpeName, 
									  const Common::GCFPValue&	value, 
									  double					timestamp,
									  bool   					wantAnswer);
    virtual TSAResult dpQuerySubscribeSingle(const string& queryWhere, 
                                             const string& queryFrom);
    virtual TSAResult dpQueryUnsubscribe	(uint32 queryId);
    
    virtual void dpCreated 			 (const string& dpName)  = 0;
    virtual void dpDeleted	 		 (const string& dpName)  = 0;
    virtual void dpeSubscribed 		 (const string& dpeName) = 0;    
    virtual void dpeSubscriptionLost (const string& dpeName) = 0;
    virtual void dpeUnsubscribed	 (const string& dpeName) = 0;
    virtual void dpeValueGet		 (const string& dpeName, const Common::GCFPValue& value) = 0;
    virtual void dpeValueChanged	 (const string& dpeName, const Common::GCFPValue& value) = 0;        
    virtual void dpeValueSet		 (const string& dpeName) = 0;
    virtual void dpQuerySubscribed	 (uint32 queryId);        

private: 
	// methods
    // interface for GSAWaitForAnswer
    void handleHotLink (const DpMsgAnswer&	answer, const GSAWaitForAnswer& wait);
    void handleHotLink (const DpHLGroup& 	group,  const GSAWaitForAnswer& wait);
    friend class GSAWaitForAnswer;
  
    // helper methods to convert PVSS dpTypes to MAC types and visa versa
    TSAResult convertPVSSToMAC (const Variable& 	variable, 
                                Common::GCFPValue** pMacValue) const;
                          
    TSAResult convertMACToPVSS (const Common::GCFPValue& macValue, 
                                Variable** 				 pVar,
                                const DpIdentifier& 	 dpId) const;
    bool getPVSSType (Common::TMACValueType macType, 
                      CharString& 			pvssTypeName) const;

    // helper methods
    TSAResult getDpId 		(const string& dpName,  		  DpIdentifier& dpId) const;
    void convPropToDpConfig (const string& dpeName, 		  string& 	  	pvssDpName, bool willReadValue);
    void convDpConfigToProp (const string& pvssDPEConfigName, string& 		dpeName);    

    void convAndForwardValueChange(const DpIdentifier& dpId, const Variable& pvssVar);
    
	// data members    
    GSAWaitForAnswer* _pWFA;
    GSASCADAHandler*  _pSCADAHandler;
};                                 

inline void GSAService::dpQuerySubscribed(uint32 /*queryId*/)
{
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
