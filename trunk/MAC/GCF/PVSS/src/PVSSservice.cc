//# PVSSservice.cc: 
//#
//# Copyright (C) 2002-2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//#
//# $Id$

#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <Common/StringUtil.h>

#include <DpMsgAnswer.hxx>
#include <DpMsgHotLink.hxx>
#include <DpHLGroup.hxx>
#include <DpVCItem.hxx>
#include <ErrHdl.hxx>
#include <ErrClass.hxx>
#include <Manager.hxx>
#include <FloatVar.hxx>
#include <CharVar.hxx>
#include <TextVar.hxx>
#include <IntegerVar.hxx>
#include <UIntegerVar.hxx>
#include <DynVar.hxx>
#include <BlobVar.hxx>
#include <AnyTypeVar.hxx>
#include <DpIdentifierVar.hxx>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/PVSS/PVSSinfo.h>

#include "GSA_WaitForAnswer.h"
#include "GSA_SCADAHandler.h"

namespace LOFAR {
	namespace GCF {
		namespace PVSS {

static const char*	SALErrors[]  = {
	"No error",						//  SA_NO_ERROR
	"Propertyname is missing",		//  SA_PROPNAME_MISSING
	"Datapointtype unknown",		//  SA_DPTYPE_UNKNOWN
	"MAC variabletype unknown",		//  SA_MACTYPE_UNKNOWN
	"Creation of DP failed",		//  SA_CREATEPROP_FAILED
	"Deletion of DP failed",		//  SA_DELETEPROP_FAILED
	"Subscribtion on DP failed",	//  SA_SUBSCRIBEPROP_FAILED
	"Unsubscribtion of DP failed",	//  SA_UNSUBSCRIBEPROP_FAILED
	"Set DP value failed",			//  SA_SETPROP_FAILED
	"Get DP value failed",			//  SA_GETPROP_FAILED
	"Subscribe on query failed",	//  SA_QUERY_SUBSC_FAILED
	"Unsubscribe of query failed",	//  SA_QUERY_UNSUBSC_FAILED
	"PVSS datbase not running",		//  SA_SCADA_NOT_AVAILABLE
	"DP does not exist",			//  SA_PROP_DOES_NOT_EXIST
	"DP already exists",			//  SA_PROP_ALREADY_EXIST
	"MAC variabletype mismatch"		//  SA_MACTYPE_MISMATCH
};

//
// PVSSerrstr (resultNr)
//
string PVSSerrstr(PVSSresult	resultNr)
{
	if ((resultNr < SA_NO_ERROR) || (resultNr > SA_MACTYPE_MISMATCH)) {
		return ("???");
	}

	return (SALErrors[resultNr]);
}

	
//
// PVSSservice()
//
PVSSservice::PVSSservice(PVSSresponse*	responsePtr) : 
	itsWFA(0),
	itsSCADAHandler(0),
	itsResponse(responsePtr)
{
	itsWFA			= new GSAWaitForAnswer(*this);
	itsSCADAHandler	= GSASCADAHandler::instance();
	ASSERTSTR(itsSCADAHandler, "Unable to get SCADA handler");
	ASSERTSTR(itsResponse,	    "No response-class specified");

	if (itsSCADAHandler->isOperational() == SA_SCADA_NOT_AVAILABLE) {
		LOG_FATAL(formatString("Error on creating a SCADA service"));
		Manager::exit(-1);
	}
}

//
// ~PVSSservice()
//
PVSSservice::~PVSSservice()
{
	if (itsWFA)
		delete itsWFA;

	ASSERT(itsSCADAHandler);
	GSASCADAHandler::release();
	itsSCADAHandler = 0;
}

//
// handleHotlink(answer, wait)
//
// Receive Signals.
// We are interested in SIGINT and SIGTERM. 
void PVSSservice::handleHotLink(const DpMsgAnswer& answer, const GSAWaitForAnswer& wait)
{
	CharString	pvssDPEConfigName;
	string 		DPEConfigName;
	string 		dpName;
	Variable 	*pVar;
	bool 		handled(false);
	GCFPValue*	pPropertyValue(0);
	CharString	pvssTypeName;
	TimeVar 	ts(answer.getOriginTime());

	LOG_DEBUG_STR("handleHotLink(" << Msg::getMsgName(answer.isAnswerOn()) << ")");

	PVSSinfo::_lastTimestamp.tv_sec  = ts.getSeconds();
	PVSSinfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;

	LOG_TRACE_FLOW_STR("Answer has " << answer.getNumberOfGroups() << " groups");
	for (AnswerGroup *pGrItem = answer.getFirstGroup();
								pGrItem; pGrItem = answer.getNextGroup()) {
		LOG_TRACE_FLOW_STR("Group has " << pGrItem->getNrOfItems() << " items");
		for (AnswerItem *pAnItem = pGrItem->getFirstItem(); pAnItem;
											pAnItem = pGrItem->getNextItem()) {
			PVSSinfo::_lastSysNr = (int) pAnItem->getDpIdentifier().getSystem();
			if (pAnItem->getDpIdentifier().convertToString(pvssDPEConfigName) == PVSS_FALSE) {
				if (answer.isAnswerOn() == DP_MSG_DP_REQ) {
					LOG_TRACE_FLOW(formatString("DP %s was deleted successful",wait.getDpName().c_str()));
					itsResponse->dpDeleted(wait.getDpName(), SA_NO_ERROR);
					handled = true;
				}
				else {
					LOG_FATAL(formatString("PVSS: Could not convert dpIdentifier '%d'", 
											pAnItem->getDpIdentifier().getDp())); 
				}
			}
			else { 
				DPEConfigName = pvssDPEConfigName;
				convDpConfigToProp(DPEConfigName, dpName);
//				LOG_DEBUG_STR("dpIdentifier = " << pAnItem->getDpIdentifier());
//				LOG_DEBUG_STR("dpName = " << dpName);
				handled = true;
				switch (answer.isAnswerOn()) {
				case DP_MSG_CONNECT:
					// called after dpeSubscribe
					LOG_TRACE_FLOW(formatString("DPE %s was subscribed successful", dpName.c_str()));
					itsResponse->dpeSubscribed(dpName, SA_NO_ERROR);
					break;

				case DP_MSG_DP_REQ:
					// called after dpCreate
					LOG_TRACE_FLOW(formatString("DP %s was created successful", dpName.c_str()));
					itsResponse->dpCreated(dpName, SA_NO_ERROR);
					break;

				case DP_MSG_SIMPLE_REQUEST:
					// called after dpGet
					pVar = pAnItem->getValuePtr();
					if (pVar) {		// could be NULL !!
						if (convertPVSSToMAC(*pVar, &pPropertyValue) != SA_NO_ERROR) {
							LOG_ERROR(formatString (
								"Could not convert PVSS DP (type %s) to MAC property (%s)", 
								(const char*)pvssTypeName, dpName.c_str()));
						}
						else {
							LOG_TRACE_FLOW(formatString("Value of '%s' was get", dpName.c_str()));
							itsResponse->dpeValueGet(dpName, SA_NO_ERROR, *pPropertyValue);
						}
						if (pPropertyValue)
							delete pPropertyValue; // constructed by convertPVSSToMAC method
					}
					break;

				case DP_MSG_FILTER_REQUEST:
				case DP_MSG_FILTER_CONNECT: {
					// hotlink as result of query subscriptions. First item contains the query id 
					// received in the answer of the query request. The second item is a dyndynanytype
					// containingh all the changes.
					LOG_DEBUG("handleHotLinkAnswer: query subscriptions");
					if (pGrItem->getNrOfItems() != 2) {
						unsigned int	queryID(((UIntegerVar*)pAnItem->getValuePtr())->getValue());
						itsResponse->dpQuerySubscribed (queryID, 
										(pGrItem->getError()) ? SA_SUBSCRIBEPROP_FAILED : SA_NO_ERROR);
						break;
					}

					// process the answer. A query result is returned as an array
					// a result of a queryConnect as seperate values.
					Variable*	firstVar  = pAnItem->getValuePtr();
					pAnItem 			  = pGrItem->getNextItem();
					Variable*	secondVar = pAnItem->getValuePtr();
					_processQueryResult(firstVar, secondVar, 
										(answer.isAnswerOn() != DP_MSG_FILTER_CONNECT));
				}
				break;

				case DP_MSG_FILTER_DISCONNECT: {
					// called after dpQueryDisconnect
					unsigned int	queryID(((UIntegerVar*)pAnItem->getValuePtr())->getValue());
					itsResponse->dpQueryUnsubscribed (queryID, 
									(pGrItem->getError()) ? SA_UNSUBSCRIBEPROP_FAILED : SA_NO_ERROR);
				}
				break;

				default:
					LOG_TRACE_FLOW_STR("Event " << answer.isAnswerOn() << " unhandled");
					handled = false;
					break;
				}
			}
			PVSSinfo::_lastSysNr = 0;
		} // for all AnserItems in group

		if (!handled) {
			if (pGrItem->wasOk() == PVSS_TRUE) {
				if (answer.isAnswerOn() == DP_MSG_COMPLEX_VC) {
					// this must be the answer on a dpSet(Wait) 
					LOG_TRACE_FLOW_STR("dpe " << wait.getDpName() << " was set");
					itsResponse->dpeValueSet(wait.getDpName(), SA_NO_ERROR);
					handled = true;
					break;		// and for-loop, all groups are empty.
				}
			}
			else {
				LOG_DEBUG(formatString("Error (%s) in answer on: %d", 
									(const char*) pGrItem->getError()->getErrorText(), 
									answer.isAnswerOn()));
			}
		}
	} // for AnswerGroup

	if (!handled) { 
		MsgType mt = answer.isAnswerOn();
		LOG_DEBUG(formatString ("Answer on: %d is not handled", mt));
	}
	PVSSinfo::_lastTimestamp.tv_sec = 0;
	PVSSinfo::_lastTimestamp.tv_usec = 0;
}

void _showDynDynVar(Variable*	pVar) 
{
	ASSERT(pVar->isDynDynVar());
	LOG_DEBUG_STR("DynDynVars type =  " << ((DynVar*)pVar)->getType());
	LOG_DEBUG_STR("DynDynVars first dimension has " << ((DynVar*)pVar)->getArrayLength() << " elements");
	LOG_DEBUG_STR(pVar->formatValue(""));
}


//
// handleHotLink(group, wait)
//
// Handle incoming hotlinks.
// This function is called from our hotlink object
void PVSSservice::handleHotLink(const DpHLGroup& group, const GSAWaitForAnswer& wait)
{
	ErrClass* 	pErr(0);
	TimeVar 	ts(group.getOriginTime());

	LOG_DEBUG("handleHotLinkGroup");
	
	if ((pErr = group.getErrorPtr()) != 0) {
		// The only error, which can occur here means always that the subscriptions 
		// to the DPE is lost
		LOG_DEBUG_STR("getDpName = " << wait.getDpName());
		uint32	queryID	= atoi(wait.getDpName().c_str());
		itsResponse->dpQueryUnsubscribed(queryID, SA_NO_ERROR);
		return;
	}

	// When the group.getIdentifier is set the group contains the result of a query or a
	// query subscription. The identifier is than the ID of the query that was returned
	// when the query was send. When the identifier is 0 the group contains a couple of changed DPs.
	if (group.getNumberOfItems() != 2) {
//	if (group.getIdentifier() == 0) { 
		// A group consists of pairs of DpIdentifier and values called items.
		// There is exactly one item for all configs we are connected.
		PVSSinfo::_lastTimestamp.tv_sec  = ts.getSeconds();
		PVSSinfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;
		PVSSinfo::_lastManNum 			 = group.getOriginManager().getManNum();
		PVSSinfo::_lastManType 			 = group.getOriginManager().getManType();
		PVSSinfo::_lastSysNr 			 = (int) group.getOriginManager().getSystem();
		// normal subscription (with dpeSubscribe)
		for (DpVCItem *pItem = group.getFirstItem(); pItem; pItem = group.getNextItem()) {
			Variable *pVar = pItem->getValuePtr();
			if (pVar) {	// could be NULL !!
//				LOG_DEBUG_STR("formatValue=" << pVar->formatValue(""));
				convAndForwardValueChange(pItem->getDpIdentifier(), *pVar);
			}
		}
	}
	else {
		// hotlink as result of query subscriptions. First item contains the query id 
		// received in the answer of the query request. The second item is a dyndynanytype
		// containingh all the changes.
		DpVCItem*	pItem     = group.getFirstItem();
		Variable*	firstVar  = pItem->getValuePtr();
		pItem 				  = group.getNextItem();
		Variable*	secondVar = pItem->getValuePtr();
		_processQueryResult(firstVar, secondVar, false);	// return all at once
	}

	PVSSinfo::_lastTimestamp.tv_sec = 0;
	PVSSinfo::_lastTimestamp.tv_usec = 0;
	PVSSinfo::_lastManNum = 0;
	PVSSinfo::_lastManType = 0;
	PVSSinfo::_lastSysNr = 0;
}

//
// extractArrayValue(Variable, row, column)
//
const Variable* extractArrayValue(Variable& var, int	row, int	column)
{
	Variable* pRetVar(0);
	Variable* pVar = ((DynVar&)*((DynVar&)var)[row])[column];
	if (pVar) {
		if (pVar->isA() == ANYTYPE_VAR) {
			pRetVar = ((AnyTypeVar*) pVar)->getVar();
		}
	}
	return pRetVar;
}

//
// convAndForwardValueChange(dpID, pvssVar)
//
void PVSSservice::convAndForwardValueChange(const DpIdentifier& dpId, const Variable& pvssVar)
{
	static CharString pvssDPEConfigName = "";
	static string 	  DPEConfigName     = "";
	static string 	  dpName            = "";
	static GCFPValue* pPropertyValue    = 0;

	if (dpId.convertToString(pvssDPEConfigName) == PVSS_FALSE) {
		LOG_FATAL(formatString("PVSS: Could not convert dpIdentifier '%d'", 
					dpId.getDp()));
		return;
	}

	DPEConfigName = pvssDPEConfigName;
	convDpConfigToProp(DPEConfigName, dpName);
	if (convertPVSSToMAC(pvssVar, &pPropertyValue) != SA_NO_ERROR) {
		LOG_ERROR(formatString("Could not convert PVSS DP(%s) to MAC property", 
				dpName.c_str()));
	}
	else {
		LOG_TRACE_FLOW(formatString("Value of '%s' has been changed", dpName.c_str()));
		itsResponse->dpeValueChanged(dpName, SA_NO_ERROR, *pPropertyValue);
	}
	if (pPropertyValue) {
		delete pPropertyValue; // constructed by convertPVSSToMAC method
	}
}

//
// _processQueryResult(firstVarPtr, secondVarPtr, passSeperate)
//
void PVSSservice::_processQueryResult(Variable*		firstVar, 
									  Variable*		secondVar,
									  bool			passSeperate)
{
	ASSERT(firstVar->isA()  == UINTEGER_VAR);
	ASSERT(secondVar->isDynDynVar());

	unsigned int	queryID(((UIntegerVar*)firstVar)->getValue());
	LOG_DEBUG_STR("processQueryResult of query " << queryID << (passSeperate ? " separate" : " compacted"));

	// second item contains the changed property and its new value
	// it is received in a dyndynanytype variable with the following construction:
	// Note that indexing starts in this case at 1!!!
	// [ 
	//   1: [ (this first item describes always the structure of the remaining items,
	//          its like a header of a table with columns)
	//        1: this header of the first column is empty because it points 
	//           always to the DP name
	//        2: header of the second column, it points to the result value for
	//           the first "SELECT" field in the query
	//        3: header of the third column for the second field
	//        4: ....
	//      ]
	//   2: [ 
	//        1: DP ID of the changed value (used to get the DP name),
	//        2: value for the first "SELECT" field in the query 
	//        3: value for the second "SELECT" field in the query 
	//        4: ...
	//      ]
	// ]
	//
	// Since we constructed the queries hard-coded the columns returned are:
	// 1: DP identifier
	// 2: online.._value
	// 3: online.._stime
	// 4: online.._managerID	[ REO: whare do we need this for???? ]

//	_showDynDynVar(secondVar);

	const Variable* pTempVar;
	int			 	nrOfRows = ((DynVar*)secondVar)->getArrayLength();
	LOG_DEBUG_STR("Queryresult contains " << nrOfRows-1 << " DPs");
	GCFPValueArray	DPnames;
	GCFPValueArray	DPvalues;
	GCFPValueArray	DPtimes;
	PVSSresult		result(SA_NO_ERROR);

	// when query is empty as least return an empty answer.
	if (nrOfRows == 1) {	// query is empty
		itsResponse->dpQueryChanged(queryID, result, 
									GCFPVDynArr(LPT_DYNSTRING, DPnames),
									GCFPVDynArr(LPT_DYNSTRING, DPvalues),
									GCFPVDynArr(LPT_DYNDATETIME, DPtimes));
		return;
	}

	// process the query result
	for (int row = 2; row <= nrOfRows; row++) {
		// --- extract the DPID of the changed DP (column 1)
		if ((pTempVar = extractArrayValue(*secondVar, row, 1)) == 0) {
			LOG_ERROR_STR("Extracting DpID from row " << row << " failed");
			return;
		}
		if (pTempVar->isA() != DPIDENTIFIER_VAR) {
			LOG_ERROR_STR("Column 1 on row " << row << 
						  " is not an DpID type but has VarType " <<
						  Variable::getTypeName(pTempVar->isA()));
			return;
		}
		DpIdentifierVar* pDpId = (DpIdentifierVar*) pTempVar;
		CharString		 DPname;
		if (pDpId->getValue().convertToString(DPname) == PVSS_FALSE) {
			DPname = "";
		}
		LOG_TRACE_VAR_STR("DpIdentifier = " << DPname);

		// --- extract the value of the changed DP (column 2)
		if ((pTempVar = extractArrayValue(*secondVar, row, 2)) == 0)  {
			LOG_ERROR_STR("Extracting Value from row " << row << " failed");
			return;
		}

		// Pass each rresult seperate of as a bunch of vectors?
		if (passSeperate) {
			convAndForwardValueChange(pDpId->getValue(), *pTempVar);
			continue;
		}

		// Add DPname, value and time to the vectors we are going to return.
		GCFPValue*	valuePtr;
		GCFPValue*	timePtr;
		if (convertPVSSToMAC(*pTempVar, &valuePtr) != SA_NO_ERROR) {
			result = SA_ELEMENTS_MISSING;
			continue;	// ERROR was logged in converPVSSToMac, skip in answer.
		}

		// --- extract the timestamp of the changed DP (column 3) also
		if ((pTempVar = extractArrayValue(*secondVar, row, 3)) == 0) {
			LOG_ERROR_STR("Extracting timestamp from row " << row << " failed");
			continue;	// skip this DP in the answer.
		}
		if (pTempVar->isA() != TIME_VAR) {
			LOG_ERROR_STR("Column 3 on row " << row << 
						  " is not an TimeVar type but has VarType " <<
						  Variable::getTypeName(pTempVar->isA()));
			return;
		}
		if (convertPVSSToMAC(*pTempVar, &timePtr) != SA_NO_ERROR) {
			result = SA_ELEMENTS_MISSING;
			continue;	// ERROR was logged in converPVSSToMac, skip in answer.
		}

		DPnames.push_back (new GCFPVString(string(DPname)));
		DPvalues.push_back(valuePtr);
		DPtimes.push_back (timePtr);
		valuePtr = 0;
		timePtr = 0;

#if 0
		// ---------- REO: Not the faintest ideas where we need to next code for ----------
		TimeVar	ts = *(TimeVar*) pTempVar;
		LOG_TRACE_VAR_STR("TimeStamp = " << pTempVar->formatValue(""));

		PVSSinfo::_lastTimestamp.tv_sec = ts.getSeconds();
		PVSSinfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;

		// extract the originator mananger ID of the changed DP (column 4)
		if ((pTempVar = extractArrayValue(*secondVar, row, 4)) == 0)  {
			LOG_ERROR_STR("Extracting Manager from row " << row << " failed");
			return;
		}
		if (pTempVar->isA() != UINTEGER_VAR)  {
			LOG_ERROR_STR("Column 4 on row " << row << 
						  " is not an IntegerVar type but has VarType " << 
						  Variable::getTypeName(pTempVar->isA()));
			return;
		}
		UIntegerVar* pManIdInt = (UIntegerVar*) pTempVar;
		LOG_TRACE_VAR_STR("ManagerID = " << pTempVar->formatValue(""));

		ManagerIdentifier manId;
		manId.convFromInt(pManIdInt->getValue());
		PVSSinfo::_lastManNum  = manId.getManNum();
		PVSSinfo::_lastManType = manId.getManType();
		PVSSinfo::_lastSysNr   = manId.getSystem();
		// ---------- REO: end of obscure code ----------
#endif
	} // for row

	if (!passSeperate) {
		itsResponse->dpQueryChanged(queryID, result, 
									GCFPVDynArr(LPT_DYNSTRING, DPnames),
									GCFPVDynArr(DPvalues[0]->getType(), DPvalues),
									GCFPVDynArr(LPT_DYNDATETIME, DPtimes));
		// free used memory
		int		nrElems = DPnames.size();
		for (int i = 0; i < nrElems; ++i) {
			delete DPnames[i];
			delete DPvalues[i];
			delete DPtimes[i];
		}
	}
}

//
// doWork()
//
void PVSSservice::doWork()
{
	ASSERT(itsSCADAHandler);
	itsSCADAHandler->workProc();
}

//
// dpCreate(dpname, typename)
//
PVSSresult PVSSservice::dpCreate(const string& dpName, 
							   const string& typeName)
{
	PVSSresult result(SA_NO_ERROR);

	DpTypeId 			dpTypeId;
	LangText 			dpNameLang(PVSSinfo::getDPbasename(dpName).c_str());
	GSAWaitForAnswer	*pWFA = new GSAWaitForAnswer(*this);
	CharString 			pvssTypeName(typeName.c_str());
	uint 				sysNr = PVSSinfo::getSysId(dpName);

	LOG_TRACE_FLOW(formatString("Create DP '%s'", dpName.c_str()));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) == SA_SCADA_NOT_AVAILABLE) {
		LOG_FATAL(formatString ("Unable to create DP: '%s'", dpName.c_str()));
	}
	else if (PVSSinfo::propExists(dpName)) {
		LOG_WARN(formatString ("DP '%s' already exists", dpName.c_str()));
  		result = SA_PROP_ALREADY_EXIST;
	}
	else if (Manager::getTypeId(pvssTypeName, dpTypeId, sysNr) == PVSS_FALSE) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "createProp",					// our function name
					  CharString("DatapointType ") + 
					  pvssTypeName + 
					  CharString(" missing"));

		LOG_FATAL(formatString("PVSS: DatapointType '%s' unknown", 
												(const char*) pvssTypeName));

		result = SA_DPTYPE_UNKNOWN;

	}
	else if (Manager::dpCreate(dpNameLang, dpTypeId, pWFA, sysNr) == PVSS_FALSE) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpCreate",					// our function name
					  CharString("Datapoint ") + 
					  dpName.c_str() + 
					  CharString(" could not be created"));

		LOG_ERROR(formatString("PVSS: Unable to create DP: '%s'", dpName.c_str()));

		result = SA_CREATEPROP_FAILED;
	}
	else {
		LOG_DEBUG(formatString("Creation of DP '%s' at system %d was requested successful", 
												PVSSinfo::getDPbasename(dpName).c_str(), sysNr));
	}

	// some error occured?
	if (result != SA_NO_ERROR) {
		itsResponse->dpCreated(dpName, result);
		// default the PVSS API is configured to delete this object
		// but if there is an error occured PVSS API will not do this
		delete pWFA;
	}

	return result;
}

//
// dpDelete(dpname)
//
PVSSresult PVSSservice::dpDelete(const string& dpName)
{
	PVSSresult 			result(SA_NO_ERROR);
	DpIdentifier 		dpId;
	GSAWaitForAnswer	*pWFA = new GSAWaitForAnswer(*this);
	pWFA->setDpName(dpName);

	LOG_TRACE_FLOW(formatString("Delete DP '%s'", dpName.c_str()));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString ("Unable to delete DP: '%s'", dpName.c_str()));
	}
	else if (!PVSSinfo::propExists(dpName)) {
		LOG_WARN(formatString ("DP '%s' does not exists", dpName.c_str()));
		result = SA_PROP_DOES_NOT_EXIST;
	}
	else if ((result = getDpId(dpName, dpId)) != SA_NO_ERROR) {
		LOG_ERROR(formatString ("Unable to delete DP: '%s'", dpName.c_str()));
	}
	else if (Manager::dpDelete(dpId, pWFA) == PVSS_FALSE) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpDelete",					// our function name
					  CharString("Datapoint ") + dpName.c_str() + 
					  CharString(" could not be deleted"));

		LOG_ERROR(formatString ("PVSS: Unable to delete DP: '%s'", dpName.c_str()));

		result = SA_DELETEPROP_FAILED;
	}
	else {
		LOG_TRACE_FLOW(formatString ("Deletion of DP '%s' was requested successful", 
																	dpName.c_str()));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpDeleted(dpName, result);
		// default the PVSS API is configured to delete this object
		// but if there is an error occured PVSS API will not do this
		delete pWFA;
	}

	return result;
}

//
// dpeSubscribe(propName)
//
PVSSresult PVSSservice::dpeSubscribe(const string& propName)
{
	PVSSresult 		result(SA_NO_ERROR);
	DpIdentifier 	dpId;
	string 			pvssDpName;
	itsWFA->setDpName(propName);

	convPropToDpConfig(propName, pvssDpName, true);		//add :_online.._value

	LOG_TRACE_FLOW(formatString ("Subscribe on property '%s'", propName.c_str()));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString("Unable to subscribe on property: '%s'",propName.c_str()));
	}
	else if (!PVSSinfo::propExists(propName)) {
		LOG_WARN(formatString ("Property: '%s' does not exists", propName.c_str()));
		result = SA_PROP_DOES_NOT_EXIST;
	}
	else if ((result = getDpId(pvssDpName, dpId)) == SA_NO_ERROR) {
		DpIdentList dpIdList;

		dpIdList.append(dpId);
		if (Manager::dpConnect(dpIdList, itsWFA, PVSS_FALSE) == PVSS_FALSE) {
			ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
						  ErrClass::ERR_PARAM,			// wrong name: blame others
						  ErrClass::UNEXPECTEDSTATE,	// fits all
						  "PVSSservice",					// our file name
						  "dpeSubscribe",				// our function name
						  CharString("Datapoint ") + propName.c_str() + 
						  CharString(" could not be connected"));

			LOG_ERROR(formatString("PVSS: Unable to subscribe on property: '%s'", 
																	propName.c_str()));
			result = SA_SUBSCRIBEPROP_FAILED;
		}
		else {
			LOG_TRACE_FLOW(formatString(
						"Subscription on property '%s' was requested successful", 
						propName.c_str()));
		}
	}
	else {
		LOG_ERROR(formatString("Unable to subscribe on property: '%s'",propName.c_str()));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpeSubscribed(propName, result);
	}

	return result;
}

//
// dpeUnsubscribe(propName)
//
PVSSresult PVSSservice::dpeUnsubscribe(const string& propName)
{
	PVSSresult 		result(SA_NO_ERROR);
	DpIdentifier 	dpId;
	string 			pvssDpName;

	convPropToDpConfig(propName, pvssDpName, true);		// add :_online.._value

	LOG_TRACE_FLOW(formatString("Unsubscribe from property '%s'", propName.c_str()));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString ("Unable to unsubscribe from property: '%s'", 
																propName.c_str()));
	}
	else if (!PVSSinfo::propExists(propName)) {
		LOG_WARN(formatString ("Property: '%s' does not exists", propName.c_str()));
		result = SA_PROP_DOES_NOT_EXIST;
	}
	else if ((result = getDpId(pvssDpName, dpId)) == SA_NO_ERROR) {
		DpIdentList dpIdList;

		dpIdList.append(dpId);

		if (Manager::dpDisconnect(dpIdList, itsWFA) == PVSS_FALSE) {
			ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
						  ErrClass::ERR_PARAM,			// wrong name: blame others
						  ErrClass::UNEXPECTEDSTATE,	// fits all
						  "PVSSservice",					// our file name
						  "dpeUnsubscribe",				// our function name
						  CharString("Datapoint ") + propName.c_str() + 
						  CharString(" could not be disconnected"));

			LOG_ERROR(formatString ("PVSS: Unable to unsubscribe from property: '%s'", 
																propName.c_str()));
			result = SA_UNSUBSCRIBEPROP_FAILED;
		}
		else {
			LOG_TRACE_FLOW(formatString (
				"Unsubscription from property '%s' was requested successful", 
				propName.c_str()));
		}
	}
	else {
		LOG_ERROR(formatString ("Unable to unsubscribe from property: '%s'", 
								propName.c_str()));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpeUnsubscribed(propName, result);
	}
	return result;
}

//
// dpeGet(dpename)
//
PVSSresult PVSSservice::dpeGet(const string& dpeName)
{
	PVSSresult 			result(SA_NO_ERROR);
	DpIdentifier 		dpId;
	string 				pvssDpName;
	GSAWaitForAnswer	*pWFA = new GSAWaitForAnswer(*this);
	pWFA->setDpName(dpeName);

	convPropToDpConfig(dpeName, pvssDpName, true);	// add :_online.._value

	LOG_TRACE_FLOW (formatString("Request value of property '%s'", dpeName.c_str()));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString ( "Unable to request of property: '%s'", dpeName.c_str()));
	}
	else if (!PVSSinfo::propExists(dpeName)) {
		LOG_WARN(formatString ( "Property: '%s' does not exists", dpeName.c_str()));
		result = SA_PROP_DOES_NOT_EXIST;
	}
	else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR) {
		LOG_ERROR(formatString("Unable to request value of property: '%s'", 
								dpeName.c_str()));
	}
	else if (Manager::dpGet(dpId, pWFA) == PVSS_FALSE) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpeGet",						// our function name
					  CharString("Value of datapoint ") + dpeName.c_str() + 
					  CharString(" could not be requested"));

		LOG_ERROR(formatString("PVSS: Unable to request value of property: '%s'", 
					dpeName.c_str()));

		result = SA_GETPROP_FAILED;
	}
	else {
		LOG_TRACE_FLOW(formatString("Value of property '%s' was requested successful", 
					dpeName.c_str()));
	}

	if (result != SA_NO_ERROR) {
		// default the PVSS API is configured to delete this object
		// but if there is an error occured PVSS API will not do this
		GCFPValue*		dummyValue = new GCFPVBool(false);
		itsResponse->dpeValueGet(dpeName, result, *dummyValue);
		delete dummyValue;
		delete pWFA;
	}

	return result;
}

//
// dpeSet (name, GCFPValue, timestamp, wantanswer)
//
PVSSresult PVSSservice::dpeSet(const string& 	dpeName, 
							   const GCFPValue& value,
							   double 			timestamp, 
							   bool 			wantAnswer)
{
	PVSSresult 		result(SA_NO_ERROR);
	DpIdentifier 	dpId;
	Variable* 		pVar(0);
	string 			pvssDpName;

	convPropToDpConfig(dpeName, pvssDpName, false);	// add .:_original.._value

	LOG_TRACE_FLOW_STR ("Set value of property '" << dpeName << "' to " << value.getValueAsString());

	ASSERT(itsSCADAHandler);
	// DB must be active
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString("Unable to set value of property: '%s'", dpeName.c_str()));
	}
	// Property must exist
	else if (!PVSSinfo::propExists(dpeName)) {
		LOG_WARN(formatString("Property: '%s' does not exists", dpeName.c_str()));
		result = SA_PROP_DOES_NOT_EXIST;
	}
	else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR) {
		LOG_ERROR(formatString("Unable to set value of property: '%s'", dpeName.c_str()));
	}
	else if ((result = convertMACToPVSS(value, &pVar, dpId)) != SA_NO_ERROR) {
		LOG_ERROR(formatString("Unable to set value of property: '%s'", dpeName.c_str()));
	}
	else {
		GSAWaitForAnswer* pWFA(0);
		if (wantAnswer) {
			// Note: pWFA will be deleted by PVSS API
			pWFA = new GSAWaitForAnswer(*this); 
			pWFA->setDpName(dpeName);
		}

		// Finally do the real SET of the parameter
		PVSSboolean retVal;
		if (timestamp > 0.0) {
			TimeVar ts;
			ts.setSeconds((long)trunc(timestamp));
			ts.setMilli((time_t)((timestamp - trunc(timestamp)) * 1000));
			retVal = Manager::dpSetTimed(ts, dpId, *pVar, pWFA);
		}
		else {
			retVal = Manager::dpSet(dpId, *pVar, pWFA);
		}

		if (retVal == PVSS_FALSE) {
			ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
						  ErrClass::ERR_PARAM,			// wrong name: blame others
						  ErrClass::UNEXPECTEDSTATE,	// fits all
						  "PVSSservice",					// our file name
						  "dpeSet()",					// our function name
						  CharString("Value of datapoint ") + dpeName.c_str() + 
						  CharString(" could not be set"));

			LOG_ERROR(formatString (
					"PVSS: Unable to set value of property: '%s' (with 'answer')",
					dpeName.c_str()));

			result = SA_SETPROP_FAILED;
			// default the PVSS API is configured to delete this object
			// but if there is an error occured PVSS API will not do this
			delete pWFA;
		}
		else {
			if (wantAnswer) {
			LOG_TRACE_FLOW(formatString (
					"Setting the value of property '%s' is requested successful", 
					dpeName.c_str()));
			}
			else {
				LOG_TRACE_FLOW(formatString ("Property value '%s' is set successful", 
						dpeName.c_str()));
			}
		}
	}

	if (pVar) {
		delete pVar; // constructed by convertMACToPVSS method
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpeValueSet(dpeName, result);
	}

	return (result);
}

//
// dpeSetMultiple(dpeNames, values, timestamp, wantAnswer)
//
PVSSresult PVSSservice::dpeSetMultiple(const string&				dpName,
									   vector<string>				dpeNames, 
									   vector<GCFPValue*>			values, 
									   double						timestamp,
									   bool   						wantAnswer)
{
	PVSSresult 		result(SA_NO_ERROR);
	DpIdValueList	dpIdList;
	DpIdentifier 	dpId;
	Variable* 		pVar(0);
	string 			pvssDpName;

	LOG_TRACE_FLOW_STR ("Set value of properties " << dpName);

	ASSERT(itsSCADAHandler);
	// DB must be active
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL_STR("Database Down, unable to set value of property: " << dpName);
		if (wantAnswer) {
			itsResponse->dpeValueSet(dpName, result);
		}
		return (result);
	}

	// Property must exist
	if (!PVSSinfo::propExists(dpName)) {
		LOG_WARN(formatString("Property: '%s' does not exists", dpName.c_str()));
		if (wantAnswer) {
			itsResponse->dpeValueSet(dpName, (result = SA_PROP_DOES_NOT_EXIST));
		}
		return (result);
	}

	// construct the DpIdValueList
	vector<string>::const_iterator	nameIter = dpeNames.begin();
	vector<string>::const_iterator	nameEnd  = dpeNames.end();
	vector<GCFPValue*>::const_iterator	valIter = values.begin();
	vector<GCFPValue*>::const_iterator	valEnd  = values.end();
	while (nameIter != nameEnd && valIter != valEnd) {
		convPropToDpConfig(dpName+"."+*nameIter, pvssDpName, false);	// add .:_original.._value
		if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR) {
			LOG_ERROR(formatString("Property: '%s' is unknown", pvssDpName.c_str()));
			if (wantAnswer) {
				itsResponse->dpeValueSet(dpName, result);
			}
			return (result);
		}
	
		if ((result = convertMACToPVSS(**valIter, &pVar, dpId)) != SA_NO_ERROR) {
			LOG_ERROR(formatString("Property: '%s' can not be converted to PVSS type", pvssDpName.c_str()));
			if (wantAnswer) {
				itsResponse->dpeValueSet(dpName, result);
			}
			return (result);
		}
		if (!dpIdList.appendItem(dpId, *pVar)) {
			LOG_ERROR_STR("Adding " << *nameIter << " to the argument list failed");
			if (wantAnswer) {
				itsResponse->dpeValueSet(dpName, (result = SA_SETPROP_FAILED));
			}
			return (result);
		}
		if (pVar) {
			delete pVar;
			pVar = 0;
		}
		nameIter++;
		valIter++;
	}

	GSAWaitForAnswer* pWFA(0);
	if (wantAnswer) {
		// Note: pWFA will be deleted by PVSS API
		pWFA = new GSAWaitForAnswer(*this); 
		pWFA->setDpName(dpName);
	}

	// Finally do the real SET of the parameter
	PVSSboolean retVal;
	if (timestamp > 0.0) {
		TimeVar ts;
		ts.setSeconds((long)trunc(timestamp));
		ts.setMilli((time_t)((timestamp - trunc(timestamp)) * 1000));
		retVal = Manager::dpSetTimed(ts, dpIdList, pWFA);
	}
	else {
		retVal = Manager::dpSet(dpIdList, pWFA);
	}

	// check result of request
	if (retVal == PVSS_FALSE) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpeSet()",					// our function name
					  CharString("Value of datapoint ") + dpName.c_str() + 
					  CharString(" could not be set"));

		LOG_ERROR(formatString (
				"PVSS: Unable to set value of property: '%s' (with 'answer')",
				dpName.c_str()));

		result = SA_SETPROP_FAILED;
		// default the PVSS API is configured to delete this object
		// but if there is an error occured PVSS API will not do this
		delete pWFA;
	}
	else {
		if (wantAnswer) {
			LOG_TRACE_FLOW(formatString (
				"Setting the value of property '%s' is requested successful", 
				dpName.c_str()));
		}
		else {
			LOG_TRACE_FLOW(formatString ("Value of property '%s' is set successful", 
					dpName.c_str()));
		}
	}

	if (pVar) {
		delete pVar; // constructed by convertMACToPVSS method
	}

	if (result != SA_NO_ERROR) {
		if (wantAnswer) {
			itsResponse->dpeValueSet(dpName, result);
		}
	}

	return (result);
}


//
// dpQuerySubscribeSingle(queryFrom, queryWhere)
//
PVSSresult PVSSservice::dpQuerySubscribeSingle(const string& queryFrom, const string& queryWhere)
{
	PVSSresult result(SA_NO_ERROR);
	const char queryFormat[] = "SELECT '_online.._value', '_online.._stime' FROM %s WHERE '_online.._online_bad' = \"FALSE\" ";

	CharString query;
	query.format(queryFormat, queryFrom.c_str());
	if (!queryWhere.empty()) {
		query += "AND ";
		query += queryWhere.c_str();
	}

	LOG_TRACE_FLOW(formatString("dpQuery(%s)", (const char*)query));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString("Unable to query subscriptions:'%s'",(const char*) query));
	}
	else if (!Manager::dpQueryConnectSingle(query, PVSS_TRUE, itsWFA, PVSS_FALSE)) { // complete
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpQuerySubscribeSingle()",	// our function name
					  CharString("Query subscription (") + query + 
					  CharString(") could not be requested!"));

		LOG_ERROR(formatString("PVSS: Unable to perform dpQueryConnectSingle: '%s'",
					(const char*) query));

		result = SA_QUERY_SUBSC_FAILED;
	}
	else {
		LOG_TRACE_FLOW(formatString ( "Query subscription (%s) was requested succesful", 
					(const char*) query));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpQuerySubscribed(0, result);
	}

	return result;
}

//
// dpQuerySubscribeAll(queryFrom, queryWhere)
//
PVSSresult PVSSservice::dpQuerySubscribeAll(const string& queryFrom, const string& queryWhere)
{
	PVSSresult result(SA_NO_ERROR);
	const char queryFormat[] = "SELECT '_online.._value', '_online.._stime' FROM %s WHERE '_online.._online_bad' = \"FALSE\" ";

	CharString query;
	query.format(queryFormat, queryFrom.c_str());
	if (!queryWhere.empty()) {
		query += "AND ";
		query += queryWhere.c_str();
	}

	LOG_TRACE_FLOW(formatString("dpQuery(%s)", (const char*)query));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL(formatString("Unable to query subscriptions:'%s'",(const char*) query));
	}
	else if (!Manager::dpQueryConnectAll(query, PVSS_TRUE, itsWFA, PVSS_FALSE)) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpQuerySubscribeAll()",	// our function name
					  CharString("Query subscription (") + query + 
					  CharString(") could not be requested!"));

		LOG_ERROR(formatString("PVSS: Unable to perform dpQueryConnectAll: '%s'",
					(const char*) query));

		result = SA_QUERY_SUBSC_FAILED;
	}
	else {
		LOG_TRACE_FLOW(formatString ( "Query subscription (%s) was requested succesful", 
					(const char*) query));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpQuerySubscribed(0, result);
	}

	return result;
}

//
// dpQueryUnscubscribe(queryID)
//
PVSSresult PVSSservice::dpQueryUnsubscribe(uint32 queryId)
{
	PVSSresult result(SA_NO_ERROR);
	LOG_TRACE_FLOW (formatString("Unsubscription from query '%d'", queryId));

	ASSERT(itsSCADAHandler);
	if ((result = itsSCADAHandler->isOperational()) != SA_NO_ERROR) {
		LOG_FATAL (formatString("Unable to unsubscribe: '%d'", queryId));
	}
	else if (!Manager::dpQueryDisconnect(queryId, itsWFA, PVSS_FALSE)) {
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "dpQueryUnsubScribe()",	// our function name
					  CharString("Query unsubscription (") + queryId + 
					  CharString(") could not be requested!"));

		LOG_ERROR(formatString("PVSS: Unable to perform dpQueryDisconnect: '%d'", 
					queryId)); 
		result = SA_QUERY_UNSUBSC_FAILED;
	}
	else {
		LOG_TRACE_FLOW(formatString (
				"Unsubscription of query (%d) was requested succesful", queryId));
	}

	if (result != SA_NO_ERROR) {
		itsResponse->dpQueryUnsubscribed(queryId, result);
	}

	return result;
}

//
// convertPVSSToMAC(variable, GCFPValue)
//
PVSSresult PVSSservice::convertPVSSToMAC(const Variable& variable, 
									   GCFPValue** pMacValue) const
{
	PVSSresult result(SA_NO_ERROR);
	*pMacValue = 0;

	switch (variable.isA()) {
	case BIT_VAR: 
		*pMacValue = new GCFPVBool(((BitVar *)&variable)->getValue()); 	
		break;
	case CHAR_VAR: 
		*pMacValue = new GCFPVChar(((CharVar *)&variable)->getValue()); 	
		break;
	case UINTEGER_VAR: 
		*pMacValue = new GCFPVUnsigned(((UIntegerVar *)&variable)->getValue()); 	
		break;
	case INTEGER_VAR: 
		*pMacValue = new GCFPVInteger(((IntegerVar *)&variable)->getValue()); 	
		break;
	case FLOAT_VAR: 
		*pMacValue = new GCFPVDouble(((FloatVar *)&variable)->getValue()); 	
		break;
	case TEXT_VAR: 
		*pMacValue = new GCFPVString(((TextVar*)&variable)->getValue()); 
		break;
	case BLOB_VAR: {
		const Blob* pBlob = &((BlobVar *)&variable)->getValue();
		*pMacValue = new GCFPVBlob((unsigned char*) pBlob->getData(), pBlob->getLen()); // no memcpy !!!
		}
		break;
//	case BIT32_VAR: 
//		*pMacValue = new GCFPVBit32(((Bit32Var *)&variable)->getValue()); 
//		break;
	case TIME_VAR: 
		*pMacValue = new GCFPVDateTime(((TimeVar *)&variable)->getSeconds(), 
									   ((TimeVar *)&variable)->getMilli()); 
		break;
	case DYNBIT_VAR: 
	case DYNCHAR_VAR: 
	case DYNUINTEGER_VAR: 
	case DYNINTEGER_VAR: 
	case DYNFLOAT_VAR:
	case DYNTEXT_VAR: {
		TMACValueType type(NO_LPT);
		switch (variable.isA()) {
		case DYNBIT_VAR: 		type = LPT_DYNBOOL; break;
		case DYNCHAR_VAR: 		type = LPT_DYNCHAR; break;
		case DYNUINTEGER_VAR: 	type = LPT_DYNUNSIGNED; break;
		case DYNINTEGER_VAR: 	type = LPT_DYNINTEGER; break;
		case DYNFLOAT_VAR: 		type = LPT_DYNDOUBLE; break;
		case DYNTEXT_VAR: 		type = LPT_DYNSTRING; break;
		case DYNBLOB_VAR: 		type = LPT_DYNBLOB; break;
		default: break;
		}

		if (type != NO_LPT) {
			const DynVar* pDynVar = (const DynVar*) (&variable);
			GCFPValueArray arrayTo;
			GCFPValue* pItemValue(0);
			for (Variable* pVar = pDynVar->getFirst(); pVar; pVar = pDynVar->getNext()) {
				switch (pVar->isA()) {
				case BIT_VAR:
					pItemValue = new GCFPVBool(((BitVar*)pVar)->getValue());
					break;
				case CHAR_VAR:
					pItemValue = new GCFPVChar(((CharVar*)pVar)->getValue());
					break;
				case INTEGER_VAR:
					pItemValue = new GCFPVInteger(((IntegerVar*)pVar)->getValue());
					break;
				case UINTEGER_VAR:
					pItemValue = new GCFPVUnsigned(((UIntegerVar*)pVar)->getValue());
					break;
				case FLOAT_VAR:
					pItemValue = new GCFPVDouble(((FloatVar*)pVar)->getValue());
					break;
				case TEXT_VAR:
					pItemValue = new GCFPVString(((TextVar*)pVar)->getValue()); 
					break;
				case BLOB_VAR: {
					const Blob* pBlob = &((BlobVar *)&variable)->getValue();
					pItemValue = new GCFPVBlob((unsigned char*)pBlob->getData(), pBlob->getLen()); // no memcpy !!!
					}
					break;
				default:
					break;
				}
				arrayTo.push_back(pItemValue);
			} // for

			*pMacValue = new GCFPVDynArr(type, arrayTo);
			for (GCFPValueArray::iterator iter = arrayTo.begin(); iter != arrayTo.end(); ++iter) {
				delete *iter;
			}
		} // if
		} // DYN_XXXvar
		break;
	default:
		result = SA_DPTYPE_UNKNOWN;
		LOG_ERROR(formatString("DPE type %s not supported (yet)!", 
				Variable::getTypeName(variable.isA())));
		break;
	}

	return result;
}

//
// convertMACToPVSS(GCFPValue, Variable, dpID)
//
PVSSresult PVSSservice::convertMACToPVSS(const GCFPValue& macValue, 
									   Variable** 		pVar, 
									   const DpIdentifier& dpId) const
{
	PVSSresult 		result(SA_NO_ERROR);
	DpElementType 	elTypeId(DPELEMENT_NOELEMENT);
	PVSSboolean		res = Manager::getElementType(dpId, elTypeId);
	*pVar = 0;

	if (res == PVSS_FALSE) {
		LOG_FATAL("PVSS: Could not get dpeType");
		return SA_DPTYPE_UNKNOWN;
	}

	switch (macValue.getType()) {
	case LPT_BOOL:
		if (elTypeId == DPELEMENT_BIT)
			*pVar = new BitVar(((GCFPVBool*)&macValue)->getValue());
		break;
	case LPT_CHAR:
		if (elTypeId == DPELEMENT_CHAR) 
			*pVar = new CharVar(((GCFPVChar*)&macValue)->getValue());
		break;
	case LPT_UNSIGNED:
		if (elTypeId == DPELEMENT_UINT) 
			*pVar = new UIntegerVar(((GCFPVUnsigned*)&macValue)->getValue());
		break;
	case LPT_INTEGER:
		if (elTypeId == DPELEMENT_INT) 
			*pVar = new IntegerVar(((GCFPVInteger*)&macValue)->getValue());
		break;
	case LPT_DOUBLE:
		if (elTypeId == DPELEMENT_FLOAT) 
			*pVar = new FloatVar(((GCFPVDouble*)&macValue)->getValue());
		break;
	case LPT_STRING:
		if (elTypeId == DPELEMENT_TEXT) 
			*pVar = new TextVar(((GCFPVString*)&macValue)->getValue().c_str());
		break;
	case LPT_BLOB:
		if (elTypeId == DPELEMENT_BLOB) {
			*pVar = new BlobVar(((GCFPVBlob*)&macValue)->getValue(), 
								((GCFPVBlob*)&macValue)->getLen(), PVSS_TRUE);
		}
		break;
	case LPT_DATETIME:
		if (elTypeId == DPELEMENT_TIME) 
			*pVar = new TimeVar(((GCFPVDateTime*)&macValue)->getSeconds(),
								((GCFPVDateTime*)&macValue)->getMilli());
		break;
//	case LPT_BIT32:
//		if (elTypeId == DPELEMENT_BIT32) 
//			*pVar = new Bit32Var(((GCFPVBit32 *)&macValue)->getValue());
//		break;
	default:
		if (macValue.getType() > LPT_DYNARR && macValue.getType() < END_DYNLPT) {
			Variable* pItemValue(0);
			VariableType type(NOTYPE_VAR);
			// the type for the new FPValue must be determined 
			// separat, because the array could be empty
			switch (macValue.getType()) {
			case LPT_DYNBOOL:
				if (elTypeId == DPELEMENT_DYNBIT) type = BIT_VAR;
				break;
			case LPT_DYNCHAR:
				if (elTypeId == DPELEMENT_DYNCHAR) type = CHAR_VAR;
				break;
			case LPT_DYNINTEGER:
				if (elTypeId == DPELEMENT_DYNINT) type = INTEGER_VAR;
				break;
			case LPT_DYNUNSIGNED:
				if (elTypeId == DPELEMENT_DYNUINT) type = UINTEGER_VAR;
				break;
			case LPT_DYNDOUBLE:
				if (elTypeId == DPELEMENT_DYNFLOAT) type = FLOAT_VAR;
				break;
			case LPT_DYNSTRING:
				if (elTypeId == DPELEMENT_DYNTEXT) type = TEXT_VAR;
				break;
			case LPT_DYNBLOB:
				if (elTypeId == DPELEMENT_DYNBLOB) type = BLOB_VAR;
				break;
			case LPT_DYNREF:
				if (elTypeId == DPELEMENT_DYNBLOB) type = TEXT_VAR;
				break;
			default:
				break;
			}
			if (type == NOTYPE_VAR) {
				// type mismatch so stop with converting data
				break;
			}
			*pVar = new DynVar(type);
			GCFPValue* pValue;
			const GCFPValueArray& arrayFrom = ((GCFPVDynArr*)&macValue)->getValue();
			for (GCFPValueArray::const_iterator iter = arrayFrom.begin(); iter != arrayFrom.end(); ++iter) {
				pValue = (*iter);
				switch (pValue->getType()) {
				case LPT_BOOL:
					pItemValue = new BitVar(((GCFPVBool*)pValue)->getValue());
					break;
				case LPT_CHAR:
					pItemValue = new CharVar(((GCFPVChar*)pValue)->getValue());
					break;
				case LPT_INTEGER:
					pItemValue = new IntegerVar(((GCFPVInteger*)pValue)->getValue());
					break;
				case LPT_UNSIGNED:
					pItemValue = new UIntegerVar(((GCFPVUnsigned*)pValue)->getValue());
					break;
				case LPT_DOUBLE:
					pItemValue = new FloatVar(((GCFPVDouble*)pValue)->getValue());
					break;
				case LPT_STRING:
					pItemValue = new TextVar(((GCFPVString*)pValue)->getValue().c_str());
					break;
				case LPT_BLOB:
					pItemValue = new BlobVar(((GCFPVBlob*)pValue)->getValue(), ((GCFPVBlob*)pValue)->getLen(), PVSS_TRUE);
					break;
				default:
					break;
				}
				if (pItemValue) {
					((DynVar *)(*pVar))->append(*pItemValue);
					delete pItemValue;
					pItemValue = 0;
				}
			}
		}
		else
			result = SA_MACTYPE_UNKNOWN;
		break;
	}
	if (result == SA_NO_ERROR && *pVar == 0) {
		CharString valueTypeName;
		getPVSSType(macValue.getType(), valueTypeName);
		LOG_ERROR(formatString (
		"Type mismatch! Property type: %s != Value type: %d (see in DpElementType.hxx)",
				(const char*) valueTypeName, elTypeId));

		result = SA_MACTYPE_MISMATCH;
	}

	return result;
}

//
// getPVSSType(macType, pvssTypeName)
//
bool PVSSservice::getPVSSType(TMACValueType macType, 
							 CharString& pvssTypeName) const
{
	switch (macType) {
	case LPT_BOOL:
		pvssTypeName = "LPT_BOOL";
		break;
	case LPT_CHAR:
		pvssTypeName = "LPT_CHAR";
		break;
	case LPT_REF:
		pvssTypeName = "LPT_REF";
		break;
	case LPT_UNSIGNED:
		pvssTypeName = "LPT_UNSIGNED";
		break;
	case LPT_INTEGER:
		pvssTypeName = "LPT_INTEGER";
		break;
	case LPT_DOUBLE:
		pvssTypeName = "LPT_DOUBLE";
		break;
	case LPT_STRING:
		pvssTypeName = "LPT_STRING";
		break;
	case LPT_BLOB:
		pvssTypeName = "LPT_BLOB";
		break;
	case LPT_DYNBOOL:
		pvssTypeName = "LPT_DYNBOOL";
		break;
	case LPT_DYNCHAR:
		pvssTypeName = "LPT_DYNCHAR";
		break;
	case LPT_DYNUNSIGNED:
		pvssTypeName = "LPT_DYNUNSIGNED";
		break;
	case LPT_DYNINTEGER:
		pvssTypeName = "LPT_DYNINTEGER";
		break;
	case LPT_DYNDOUBLE:
		pvssTypeName = "LPT_DYNDOUBLE";
		break;
	case LPT_DYNSTRING:
		pvssTypeName = "LPT_DYNSTRING";
		break;
	case LPT_DYNBLOB:
		pvssTypeName = "LPT_DYNBLOB";
		break;
	case LPT_DYNREF:
		pvssTypeName = "LPT_DYNREF";
		break;
	default:
		return (false);
	}

	return (true);
}

//
// getDpId(dpname, &dpID)
//
PVSSresult PVSSservice::getDpId(const string& dpName, DpIdentifier& dpId) const
{
	PVSSresult result(SA_NO_ERROR);

	CharString pvssDpName(dpName.c_str());

	// Ask the Identification for the DpId of our Datapoints
	if (Manager::getId(pvssDpName, dpId) == PVSS_FALSE) {
		// This name was unknown.
		// The parameters are in Bascis/ErrClass.hxx
		ErrHdl::error(ErrClass::PRIO_SEVERE,		// It is a severe error
					  ErrClass::ERR_PARAM,			// wrong name: blame others
					  ErrClass::UNEXPECTEDSTATE,	// fits all
					  "PVSSservice",					// our file name
					  "getDpId",					// our function name
					  CharString("Datapoint ") + pvssDpName + CharString(" missing"));

		LOG_DEBUG(formatString ( "PVSS: Datapoint '%s' missing", dpName.c_str()));

		result = SA_PROPNAME_MISSING;
	}

	return result;
}

//
// convPropToDpConfig(propName, pvssDpName, willReadValue)
//
void PVSSservice::convPropToDpConfig(const string& propName, string& pvssDpName, bool willReadValue)
{
	pvssDpName = propName.c_str();
	if (propName.find('.') >= propName.size()) {
		pvssDpName += ".";
	}
	if (willReadValue) {
		pvssDpName += ":_online.._value";
	}
	else {
		pvssDpName += ":_original.._value";
	}
}

//
// convDpConfigToProp(pvssDPEConfigName, propName)
//
// DPname has the form: SYS:DP.EL:_xxx or likewise.
// Stripoff the last part so that the DP name remains.
//
void PVSSservice::convDpConfigToProp(const string& pvssDPEConfigName, string& propName)
{
	string::size_type doublePointPos = pvssDPEConfigName.find(':');
	string::size_type dotPos 		 = pvssDPEConfigName.find('.');
	string::size_type nrOfCharsToCopy(pvssDPEConfigName.length());

	if (doublePointPos < dotPos) {
		doublePointPos = pvssDPEConfigName.find(':', doublePointPos + 1);
	}

	if (doublePointPos < pvssDPEConfigName.length()) {
		nrOfCharsToCopy = doublePointPos;
	}
	else {
		doublePointPos = pvssDPEConfigName.length();
	}

	if ((doublePointPos - 1) == dotPos) {
		nrOfCharsToCopy--;
	}

	propName.assign(pvssDPEConfigName, 0, nrOfCharsToCopy); 
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
