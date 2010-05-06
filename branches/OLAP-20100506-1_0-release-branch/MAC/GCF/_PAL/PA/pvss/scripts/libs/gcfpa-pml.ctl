//# gcfpa-pml.ctl
//#
//#  Copyright (C) 2002-2004
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

//#
//# Functions to handle propertysets from PVSS scripts and panels
//# 

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcfpa-com.ctl"

global dyn_dyn_string gCallBackList; // 1 == callBackFuncName; 2 == ID; 3 == myManNum
global dyn_dyn_string gSeqList;	// 1 == seqNr ; 2 == ID; 3 == scope
global dyn_dyn_string gPSList;	// 1 == scope; 2 == ID

// GCF INTERFACE METHODS - START

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfInit(callbackFunction) : ID
//
//
//
////////////////////////////////////////////////////////////////////////////////
unsigned gcfInit(string callBackFuncName) {
/*	LOG_DEBUG("gcfInit: ", callBackFuncName);

	dyn_string callBackFuncNames, IDlist;
	callBackFuncNames = getDynString(gCallBackList, 1);
	IDlist = getDynString(gCallBackList, 2);
	unsigned ID = 0;
	do {
		ID++;
	} while (dynContains(IDlist, ID) > 0);
	
	LOG_INFO("GCF: ID " + ID + " is claimed for unique communication with PA of GCF.");
	dyn_string newItem;
	string callBackDP = "__gcfportUIM" + myManNum() +"_" + ID + "_DPAclient";
	if (!dpExists(callBackDP)) {
		dpCreate(callBackDP, "GCFDistPort");
	}
	LOG_DEBUG("GCF: " + callBackDP + " will be used for communication with PA.");
	dpConnect("gcfMainCallBack", FALSE, callBackDP + ".");			
	newItem = makeDynString(callBackFuncName, ID, myManNum());
	gCallBackList[dynlen(gCallBackList) + 1] = newItem;
 	dpConnect("gcfWatchDog", FALSE, "__gcf_wd.sys");
	return ID;
*/
}

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfLeave(ID, inTerminate)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfLeave(unsigned ID, bool inTerminate = false) {
/*	LOG_DEBUG("GCF: ID " + ID + " will be freed.");
	
	for (int i = 1; i <= dynlen(gCallBackList); i++) {
		if (gCallBackList[i][2] == ID) {
			string callBackDP = buildCallBackDP(ID) + ".";
			LOG_DEBUG("GCF: " + callBackDP + " will not be used anymore.");
			//if (!inTerminate) dpDisconnect("gcfMainCallBack", callBackDP);
			dynRemove(gCallBackList, i);
			break;
		}
	}
	dyn_string systemsToInform;
	for (int i = 1; i <= dynlen(gSeqList); i++) {
		if (gSeqList[i][2] == ID) {
			dynAppend(systemsToInform, dpSubStr(gSeqList[i][3], DPSUB_SYS));
			dynRemove(gSeqList, i);
		}
	}
	for (int i = 1; i <= dynlen(gPSList); i++) {
		if (gPSList[i][2] == ID) {
			dynAppend(systemsToInform, dpSubStr(gPSList[i][1], DPSUB_SYS));
			dynRemove(gPSList, i);
		}
	}
	
	dynUnique(systemsToInform);
	string portID = buildPortId(ID);
	string disconnectMsg = "d" + portID;	
	for (int i = 1; i <= dynlen(systemsToInform); i++) {
		sendEventToPA(disconnectMsg, systemsToInform[i]);
	}
*/
}

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfLoadPS(ID, PSname)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfLoadPS(unsigned ID, string psScope) {
/*
  	LOG_DEBUG("gcfLoadPS: ", ID, psScope);
	correctScope(psScope);
	LOG_TRACE("GCF: Request to load property set " + psScope);
	if (idExists(ID) && isPAOnline(dpSubStr(psScope, DPSUB_SYS))) {
		string portID = buildPortId(ID);
		string callBackDP = buildCallBackDP(ID); 
		unsigned seqNr = registerAction(ID, psScope);
		string msg = "u" + portID + "|" + callBackDP + "|l|" + seqNr + "|" + getDPNameOnly(psScope);
		sendEventToPA(msg, dpSubStr(psScope, DPSUB_SYS));
	}
*/
}

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfUnloadPS(ID, PSname)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfUnloadPS(unsigned ID, string psScope) {
/*
  	LOG_DEBUG("gcfUnloadPS: ", ID, psScope);
	correctScope(psScope);
	LOG_TRACE("GCF: Request to unload property set " + psScope);
	if (idExists(ID) && isPAOnline(dpSubStr(psScope, DPSUB_SYS))) {
		string portID = buildPortId(ID);
		string callBackDP = buildCallBackDP(ID); 
		unsigned seqNr = registerAction(ID, psScope);
		string msg = "u" + portID + "|" + callBackDP + "|ul|" + seqNr + "|" +  + getDPNameOnly(psScope);
		sendEventToPA(msg, dpSubStr(psScope, DPSUB_SYS));
		deletePropSet(psScope, ID);
	}
*/
}

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfConfigurePS(ID, PSname, APCname)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfConfigurePS(unsigned ID, string psScope, string psApcName) {
/*  	LOG_DEBUG("gcfConfigurePS: ", ID, psScope, psApcName);
	correctScope(psScope);
	LOG_TRACE("GCF: Request to configure property set " + psScope + " with APC " + psApcName);
	if (idExists(ID) && isPAOnline(dpSubStr(psScope, DPSUB_SYS))) {
		string portID = buildPortId(ID);
		string callBackDP = buildCallBackDP(ID); 
		unsigned seqNr = registerAction(ID, psScope);
		string msg = "u" + portID + "|" + callBackDP + "|conf|" + seqNr + "|" +  + getDPNameOnly(psScope) + "|" + psApcName;
		sendEventToPA(msg, dpSubStr(psScope, DPSUB_SYS));
	}
*/
}

// GCF INTERFACE METHODS - END

// GCF INTERNAL METHODS - START

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler gcfMainCallBack(DP, value)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfMainCallBack(string callBackDP, blob value) {
/*
	LOG_DEBUG("gcfMainCallBack: ", callBackDP, value);
	dyn_string splittedDP = strsplit(dpSubStr(callBackDP, DPSUB_DP), "_");
	unsigned lastElement = dynlen(splittedDP);
	unsigned ID = splittedDP[lastElement - 1];
	string callBackFunc = findCallBackFunc(ID);
	dyn_string msg;
	string msgValue;
	blobGetValue(value, 0, msgValue, bloblen(value));
	LOG_TRACE("GCF: Incomming message: " + msgValue);
	msg = strsplit(msgValue, "|");
	dyn_string response;
	if (msg[1] == "m") {
		dynAppend(response, msg[4]); 				// 4 == response signal
		if (msg[4] == "loaded") {		
			string psScope = getPropSet(msg[5]); 	// 5 == seqNr
			if (msg[6] == "OK") {					// 6 == result
				dyn_string newItem = makeDynString(psScope, ID);
				gPSList[dynlen(gPSList) + 1] = newItem;
			}
			unregisterAction(msg[5]);
			dynAppend(response, psScope);
			dynAppend(response, msg[6]); 
			callUserDefinedFunction(callBackFunc, response);
		}
		else if (msg[4] == "unloaded") {
			dynAppend(response, getPropSet(msg[5])); 
			dynAppend(response, msg[6]); 			// 6 == result
			unregisterAction(msg[5]);
			callUserDefinedFunction(callBackFunc, response);
		}
		else if (msg[4] == "configured") {
			dynAppend(response, getPropSet(msg[5]));
			dynAppend(response, msg[7]); // 7 == apcName
			dynAppend(response, msg[6]); // 6 == result
			unregisterAction(msg[5]);
			callUserDefinedFunction(callBackFunc, response);
		}
		else if (msg[4] == "gone") {
			deletePropSet(msg[5], ID); // 5 == property set scope
			for (int i = 1; i <= dynlen(gSeqList); i++) {
				if (gSeqList[i][2] == ID && gSeqList[i][3] == msg[5]) {
					dynRemove(gSeqList, i);
				}
			}
			dynAppend(response, msg[5]);
			callUserDefinedFunction(callBackFunc, response);
		}		
	}
*/
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function registerAction(ID, PSname)
//
//
//
////////////////////////////////////////////////////////////////////////////////
unsigned registerAction(unsigned ID, string& psScope) {
//  	LOG_DEBUG("registerAction: ", ID, psScope);
//	dyn_string seqNrlist = getDynString(gSeqList, 1);
	unsigned seqNr = 0;
/*	do {
		seqNr++;
	} while (dynContains(seqNrlist, seqNr) > 0);
	
	dyn_string newItem = makeDynString(seqNr, ID, psScope);	
	gSeqList[dynlen(gSeqList) + 1] = newItem;
*/
	return seqNr;
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function unregisterAction(seqNr)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void unregisterAction(unsigned seqNr) {

  	LOG_DEBUG("unregisterAction: ", seqNr);
	for (int i = 1; i <= dynlen(gSeqList); i++) {
		if (gSeqList[i][1] == seqNr) {
			dynRemove(gSeqList, i);
			break;
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function getPropSet(seqNr) : PSname
//
//
//
////////////////////////////////////////////////////////////////////////////////
string getPropSet(unsigned seqNr) {
	LOG_DEBUG("getPropSet: ", seqNr);
	for (int i = 1; i <= dynlen(gSeqList); i++) {
		if (gSeqList[i][1] == seqNr) {
			return gSeqList[i][3];
		}
	}	
	return "";
}


////////////////////////////////////////////////////////////////////////////////
//
// Private Function findCallBackFunc(ID) : name
//
//
//
////////////////////////////////////////////////////////////////////////////////
string findCallBackFunc(unsigned ID) {
	LOG_DEBUG("findCallBackFunc: ", ID);
	for (int i = 1; i <= dynlen(gCallBackList); i++) {
		if (gCallBackList[i][2] == ID) {
			return gCallBackList[i][1];
		}
	}
	return "";
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function idExists(ID) : bool
//
//
//
////////////////////////////////////////////////////////////////////////////////
bool idExists(unsigned ID) {
	LOG_DEBUG("idExists: ", ID);
	dyn_string IDlist;
	IDlist = getDynString(gCallBackList, 2);
	return (dynContains(IDlist, ID) > 0); 
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function buildPortID(ID) : name
//
//
//
////////////////////////////////////////////////////////////////////////////////
string buildPortId(unsigned ID) {
	LOG_DEBUG("buildPortId: ", ID);
	string systemId = (string) getSystemId();
	return systemId + ":Ui:" + myManNum() + ":" + ID + ":";
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function buildCallBackDP(ID) : name
//
//
//
////////////////////////////////////////////////////////////////////////////////
string buildCallBackDP(unsigned ID) {
	LOG_DEBUG("buildCallBackDP: ", ID);
	return getSystemName() + "__gcfportUIM" + myManNum() +"_" + ID + "_DPAclient";		
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function callUserDefinedFunction(function, &response)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void callUserDefinedFunction(string& callBackFunc, dyn_string& response) {
	LOG_DEBUG("callUserDefinedFunction: ", callBackFunc, response);
	if (isFunctionDefined(callBackFunc)) {
		startThread(callBackFunc, response);
	}
	else {
		LOG_TRACE("GCF ERROR: CallBackFunc '" + callBackFunc + "' not defined by user!"); 
	}	
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function deletePropSeT(PSname, ID)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void deletePropSet(string& psScope, unsigned ID) {
	LOG_DEBUG("deletePropSet: ", psScope, ID);
	for (int i = 1; i <= dynlen(gPSList); i++) {
		if (gPSList[i][2] == ID && gPSList[i][1] == psScope) {
			dynRemove(gPSList, i);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function correctScope(PSname)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void correctScope(string& psScope) {
	LOG_DEBUG("correctScope: ", psScope);
	if (dpExists(psScope)) {
		psScope = dpSubStr(psScope, DPSUB_SYS) + dpSubStr(psScope, DPSUB_DP);
	}
	else {
		if (dpSubStr(psScope, DPSUB_SYS) == "") {
			psScope = getSystemName() + psScope;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function getDPNameOnly(PSname) : name
//
//
//
////////////////////////////////////////////////////////////////////////////////
string getDPNameOnly(string& psScope) {
	LOG_DEBUG("getDPNameOnly: ", psScope);
	if (dpExists(psScope)) {
		return dpSubStr(psScope, DPSUB_DP);
	}
	else {
		if (dpSubStr(psScope, DPSUB_SYS) == "") {
			return psScope;
		}
		else {
			return substr(psScope, strlen(dpSubStr(psScope, DPSUB_SYS)));
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function isPAOnline (sysname) : bool,
//
//
//
////////////////////////////////////////////////////////////////////////////////
bool isPAOnline(string sysName) {
//	LOG_DEBUG("isPAOnline: ", sysName);
	bool paOnline = dpExists(sysName + "__gcfportAPI_DPAserver");
	
	if (!paOnline) {
		LOG_ERROR("GCF ERROR: PA on system " + sysName + " not reachable!");
	}

	return paOnline;
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler gcfWatchDog (dp, message)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void gcfWatchDog(string dp, string wdMsg) {
/*	LOG_DEBUG("gcfWatchDog: ", dp, wdMsg);

	unsigned sysNr = substr(wdMsg, 1, strlen(wdMsg) - 2);
	if (wdMsg[0] != 'd') {
		LOG_TRACE("GCF: System " + getSystemName(sysNr) + " came.");
		return;
	}

	LOG_TRACE("GCF: System " + getSystemName(sysNr) + " is gone.");

	string 		sysNrOfLoadedPS;
	string 		callBackFunc;
	dyn_string 	indication;
	for (int i = 1; i <= dynlen(gPSList); i++) {
		sysNrOfLoadedPS = getSystemId(dpSubStr(gPSList[i][1], DPSUB_SYS)); // 1 == loaded property set scope
		if (sysNrOfLoadedPS == sysNr) {
			dynAppend(indication, "gone"); 
			dynAppend(indication, gPSList[i][1]); 			
			callBackFunc = findCallBackFunc(gPSList[i][2]); // 2 == ID
			callUserDefinedFunction(callBackFunc, indication);
			dynRemove(gPSList, i);
		}
	}
	for (int i = 1; i <= dynlen(gSeqList); i++) {
		sysNrOfLoadedPS = getSystemId(dpSubStr(gSeqList[i][3], DPSUB_SYS)); // 3 == loaded property set scope
		if (sysNrOfLoadedPS == sysNr) {
			dynAppend(indication, "gone"); 
			dynAppend(indication, gSeqList[i][3]); 			
			callBackFunc = findCallBackFunc(gSeqList[i][2]); // 2 == ID
			callUserDefinedFunction(callBackFunc, indication);
			dynRemove(gSeqList, i);
		}
	}	
*/
}
