//#  PVSSinfo.cc: 
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

#include <lofar_config.h>

#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresult.h>
#include <Manager.hxx>
#include <Datapoint.hxx>
#include <DpContainer.hxx>
#include <DpIdentification.hxx>
#include <Common/StringUtil.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

string 	PVSSinfo::_sysName 		= "";
string 	PVSSinfo::_projName 	= "";
uint 	PVSSinfo::_lastSysNr 	= 0;
timeval PVSSinfo::_lastTimestamp= {0, 0};
uint 	PVSSinfo::_lastManNum 	= 0;
uint 	PVSSinfo::_lastManType 	= 0;

TMACValueType macValueTypes[] = 
{
  NO_LPT,           // DPELEMENT_NOELEMENT,
  NO_LPT,           // DPELEMENT_RECORD,
  NO_LPT,           // DPELEMENT_ARRAY,
  LPT_DYNCHAR,      // DPELEMENT_DYNCHAR,
  LPT_DYNUNSIGNED,  // DPELEMENT_DYNUINT,
  LPT_DYNINTEGER,   // DPELEMENT_DYNINT,
  LPT_DYNDOUBLE,    // DPELEMENT_DYNFLOAT,
  LPT_DYNBOOL,      // DPELEMENT_DYNBIT,
  NO_LPT,           // DPELEMENT_DYN32BIT, not yet
  LPT_DYNSTRING,    // DPELEMENT_DYNTEXT,
  NO_LPT,           // DPELEMENT_DYNTIME, not yet
  NO_LPT,           // DPELEMENT_CHARARRAY,
  NO_LPT,           // DPELEMENT_UINTARRAY,
  NO_LPT,           // DPELEMENT_INTARRAY,
  NO_LPT,           // DPELEMENT_FLOATARRAY,
  NO_LPT,           // DPELEMENT_BITARRAY,
  NO_LPT,           // DPELEMENT_32BITARRAY,
  NO_LPT,           // DPELEMENT_TEXTARRAY,
  NO_LPT,           // DPELEMENT_TIMEARRAY,
  LPT_CHAR,         // DPELEMENT_CHAR,
  LPT_UNSIGNED,     // DPELEMENT_UINT,
  LPT_INTEGER,      // DPELEMENT_INT,
  LPT_DOUBLE,       // DPELEMENT_FLOAT,
  LPT_BOOL,         // DPELEMENT_BIT,
  NO_LPT,           // DPELEMENT_32BIT, not yet
  LPT_STRING,       // DPELEMENT_TEXT,
  NO_LPT,           // DPELEMENT_TIME, not yet
  NO_LPT,           // DPELEMENT_DPID,
  NO_LPT,           // DPELEMENT_NOVALUE,
  NO_LPT,           // DPELEMENT_DYNDPID,
  NO_LPT,           // DPELEMENT_DYNCHARARRAY,
  NO_LPT,           // DPELEMENT_DYNUINTARRAY,
  NO_LPT,           // DPELEMENT_DYNINTARRAY,
  NO_LPT,           // DPELEMENT_DYNFLOATARRAY,
  NO_LPT,           // DPELEMENT_DYNBITARRAY,
  NO_LPT,           // DPELEMENT_DYN32BITARRAY,
  NO_LPT,           // DPELEMENT_DYNTEXTARRAY,
  NO_LPT,           // DPELEMENT_DYNTIMEARRAY,
  NO_LPT,           // DPELEMENT_DYNDPIDARRAY,
  NO_LPT,           // DPELEMENT_DPIDARRAY,
  NO_LPT,           // DPELEMENT_NOVALUEARRAY,
  NO_LPT,           // DPELEMENT_TYPEREFERENCE,
  NO_LPT,           // DPELEMENT_LANGTEXT,
  NO_LPT,           // DPELEMENT_LANGTEXTARRAY,
  NO_LPT,           // DPELEMENT_DYNLANGTEXT,
  NO_LPT,           // DPELEMENT_DYNLANGTEXTARRAY,
  LPT_BLOB,         // DPELEMENT_BLOB,
  NO_LPT,           // DPELEMENT_BLOBARRAY,
  LPT_DYNBLOB,      // DPELEMENT_DYNBLOB,
  NO_LPT,           // DPELEMENT_DYNBLOBARRAY,
};

//
// propExists(dpeName)
//
bool PVSSinfo::propExists(const string& dpeName)
{
	DpIdentifier 	dpId;
	CharString 		dpePvssName(dpeName.c_str());
	LOG_TRACE_VAR_STR("propExists(" << dpeName << "):" << dpePvssName);

	if (Manager::getId(dpePvssName, dpId) == PVSS_FALSE) {
		return false;
	}

	return true;
}

//
// typeExists (typeName)
//
bool PVSSinfo::typeExists (const string& dpTypeName)
{
	CharString 	pvssTypeName(dpTypeName.c_str());
	DpTypeId 	dpTypeId; 
	return (Manager::getTypeId(pvssTypeName, dpTypeId) == PVSS_TRUE);
}

//
// getMACTypeId (dpeName)
//
TMACValueType PVSSinfo::getMACTypeId (const string& dpeName)
{
	// first find out whether there is a system name specified or not
	vector<string> splittedDpeName = StringUtil::split(dpeName, ':');
	uint 		sysNr = getSysId(dpeName);
	if (sysNr == 0) {
		sysNr = PVSSinfo::getLocalSystemId();
	}

	DpElementType 	dpElType; 
	DpIdentifier 	dpId;
	CharString 		pvssDpeName(dpeName.c_str());
	if ((Manager::getId(pvssDpeName, dpId) == PVSS_TRUE) &&
		(Manager::getTypeContainerPtr(sysNr)->getElementType(dpId, dpElType) == DpTypeContOK)) {
			return(macValueTypes[dpElType]);
	}
	return (NO_LPT);
}

//
// getLocalSystemName()
//
const string& PVSSinfo::getLocalSystemName()
{
	if (_sysName.length() == 0) {
		CharString sysName;
		if (Manager::getSystemName(Resources::getSystem(), sysName) == PVSS_TRUE) {      
			_sysName += (const char*) sysName;
		}
	}
	return (_sysName);
}

//
// getLocalSystemId()
//
uint PVSSinfo::getLocalSystemId()
{
	return((int)Resources::getSystem());
}

//
// getSystemName(sysnr)
//
const string PVSSinfo::getSystemName(uint sysnr)
{
	CharString sysName;
	if (Manager::getSystemName(sysnr, sysName) == PVSS_TRUE) {      
		return ((const char*) sysName);
	}
	return ("");
}

//
// getProjectName()
//
const string& PVSSinfo::getProjectName()
{
	if (_projName.length() == 0) {
		CharString projName = Resources::getProjectName();
		_projName += (const char*) projName;
	}
	return (_projName);
}

//
// getSysId(name)
//
uint PVSSinfo::getSysId(const string& name)
{
	string::size_type index = name.find(':');
	if (index == string::npos) {
		index = name.length();
	}
	CharString sysName(name.c_str(), index);
	uint	 sysNr;
	// workaround - start
	DpIdentifier dpId;
	Manager::getId(sysName + ":", dpId);
	sysNr = (int)dpId.getSystem();
	// workaround - end
	/*
	DpIdentificationResult res;
	if ((res = Manager::getDpIdentificationPtr()->getSystemId((const char*)sysName, sysNr)) != DpIdentOK) {
		LOG_ERROR(formatString("PVSS could not find system nr for '%s', error %d",
								(const char*) sysName, res));    
		sysNr = 0;
	}*/
	return (sysNr);
}

//
// getOwnManNum()
//
uint PVSSinfo::getOwnManNum()
{
	return (Resources::getManNum());
}

//
// getAllProperties(typeFilter, dpFilter, &foundProps)
//
void PVSSinfo::getAllProperties(const string& typeFilter, 
                                const string& dpFilter, 
                                vector<string>& foundProperties)
{
	foundProperties.clear();
	DpIdentification::TypeIdNamePair* 	typeArray(0);
	DpIdentification::DpIdNamePair* 	dpIdArray(0);
	PVSSlong 							howManyTypes, howManyDpIds;

	// get all DP types which meet with a "typeFilter"
	Manager::getDpIdentificationPtr()->
			getSortedTypeNames(typeFilter.c_str(), typeArray, howManyTypes, PVSS_FALSE);

	list<TPropertyInfo> propInfos;
	string 				dpeName;
	for (PVSSlong i = 0; i < howManyTypes; i++) {
		// Get all DP's for each type
		Manager::getDpIdentificationPtr()->
				getSortedDpIdNames(dpFilter.c_str(), dpIdArray, howManyDpIds, 
									PVSS_FALSE, typeArray[i].id);    

		for (PVSSlong j = 0; j < howManyDpIds; j++) {
			if (j == 0) {
				// get type structure (type elements) of the current DPT only once for
				// all found DP's
				getTypeStruct((const char*) typeArray[i].text.getText(), propInfos);
			}
			for (list<TPropertyInfo>::iterator iter = propInfos.begin();
												iter != propInfos.end(); iter++) {
				// concatenate DP name and element name to a DPE.
				dpeName = formatString ("%s.%s", (const char *)dpIdArray[j].id.toString(),
										iter->propName.c_str());

				LOG_TRACE_FLOW(formatString("Found DPE: %s", dpeName.c_str()));
				foundProperties.push_back(dpeName);
			}
		}
		if (dpIdArray) {
			delete [] dpIdArray;
		}
		dpIdArray = 0;
	}
	if (typeArray) {
		delete [] typeArray;
	}
}

//
// getAllTypes(typeFilter, &foundTypes)
//
void PVSSinfo::getAllTypes(const string& typeFilter, 
                           vector<string>& foundTypes)
{
	foundTypes.clear();
	DpIdentification::TypeIdNamePair* 	typeArray(0);
	PVSSlong 							howManyTypes;

	// get all DP types which meats with a "typeFilter"
	Manager::getDpIdentificationPtr()->
			getSortedTypeNames(typeFilter.c_str(), typeArray, howManyTypes, PVSS_FALSE);

	for (PVSSlong i = 0; i < howManyTypes; i++) {
		LOG_TRACE_FLOW(formatString("Found type: %s", (const char *)typeArray[i].text.getText()));
		foundTypes.push_back((const char *)typeArray[i].text.getText());
	}
	if (typeArray) {
		delete [] typeArray;  
	}
}                             

// forward declaration
void buildTypeStructTree(const string path, const DpType* pType, const DpElementId elId, list<TPropertyInfo>& propInfos);

//
// getTypeStruct(typename, &propInfo, sysnr)
//
TGCFResult PVSSinfo::getTypeStruct(const string& 		typeName, 
								  list<TPropertyInfo>& 	propInfos, 
								  uint	 				sysNr)
{
	propInfos.clear();

	CharString 	pvssTypeName = typeName.c_str();
	DpTypeId 	typeId;
	DpType* 	pType;
	if (Manager::getTypeId(pvssTypeName, typeId, sysNr) == PVSS_FALSE) {
		string sysName = getSystemName(sysNr);
		LOG_ERROR(formatString("PVSS could not find type %s on system %s(%d)", 
								typeName.c_str(), sysName.c_str(), sysNr));
		return(GCF_PVSS_ERROR);
	}

	if ((pType = Manager::getTypeContainerPtr(sysNr)->getTypePtr(typeId)) == 0) {
		LOG_ERROR(formatString("PVSS internal error on type information (%s:%s)",
								getSystemName(sysNr).c_str(), typeName.c_str()));
		return (GCF_PVSS_ERROR);
	}

	DpElementId elId = pType->getRootElement();
	string 		path;
	buildTypeStructTree(path, pType, elId, propInfos);
	return (GCF_NO_ERROR);  
}

//
// buildTypeStructTree(path, dpType, dpElement, &propInfo)
//
void buildTypeStructTree(const string 			path, 
                         const DpType* 			pType, 
                         const DpElementId 		elId, 
                         list<TPropertyInfo>& 	propInfos)
{
	string 			propName = path;
	DpElementType 	elType = pType->getTypeNodePtr(elId)->getElementType();
	if (elId != pType->getRootElement()) {
		char* elName;
		Manager::getDpIdentificationPtr()->getElementName(pType->getName(), elId, elName);
		if (elType != DPELEMENT_TYPEREFERENCE) {
			if (propName.length() > 0) {
				propName += '.';
			}
			propName += elName;
		}
		delete [] elName; 
	}

	if (elType != DPELEMENT_RECORD && elType != DPELEMENT_TYPEREFERENCE) {
		if (macValueTypes[elType] != NO_LPT) {      
			if (PVSSinfo::isValidPropName(propName.c_str())) {
				TPropertyInfo propInfo;
				propInfo.propName = propName;
				propInfo.type 	  = macValueTypes[elType];
				propInfos.push_back(propInfo);
			}
			else {
				LOG_WARN(formatString ( 
					"Property name %s meets not the name convention! Not add!!!",
					propName.c_str()));
			}
		}
		else {
			LOG_ERROR(formatString(
				"TypeElement type %d (see DpElementType.hxx) is unknown to GCF (%s). Not add!!!",
				elType, propName.c_str()));      
		}
	} 
	else {
		DynPtrArrayIndex nrOfChilds = pType->getSonNumber(elId);
		DpElementId childElId;
		for (DynPtrArrayIndex i = 0; i < nrOfChilds; i++) {
			childElId = pType->getSon(elId, i);
			buildTypeStructTree(propName, pType, childElId, propInfos);
		}
	}
}

//
// isValidPropName(name)
//
bool PVSSinfo::isValidPropName(const char* propName)
{
	ASSERT(propName);

	char			doubleSep[] = {GCF_PROP_NAME_SEP, GCF_PROP_NAME_SEP, 0};
	unsigned int	length = strlen(propName);

	// Invalid: .***   ***.  and  ***..***
	if (propName[0] == GCF_PROP_NAME_SEP || propName[length - 1] == GCF_PROP_NAME_SEP ) {
		LOG_TRACE_COND_STR("isValidPropName(" << propName << "): dot at edge of name");
		return (false);
	}
	if (strstr(propName, doubleSep) != 0) {
		LOG_TRACE_COND_STR("isValidPropName(" << propName << "): double dot");
		return (false);
	}

	// ref indication may only found at begin or after a GCF_PROP_NAME_SEP
	char	refInd[] = "__";
	char*	refIndPos = strstr(propName, refInd);
	if (refIndPos != 0) {									// we found it
		if (refIndPos > propName) {							// not at begin
			if (*(refIndPos - 1) != GCF_PROP_NAME_SEP) {	// not at a dot
				LOG_TRACE_COND_STR("isValidPropName(" << propName << "): double underscore not after dot");
				return (false);
			}
		}
		// ref indication may not used in struct name: ***__***.*** is not valid
		if (strchr(refIndPos, GCF_PROP_NAME_SEP) > 0) {
			LOG_TRACE_COND_STR("isValidPropName(" << propName << "): double underscore in DP-part");
			return (false);
		}
	}

	// only allow dots, :  and __ in the name.
	for (unsigned short i = 0; i < length; i++) {
		if (refIndPos > 0 && ((propName + i) == refIndPos)) {
			i += 2; // skip the ref indicator
		}
		if (!isalnum(propName[i]) && (propName[i] != GCF_PROP_NAME_SEP) && 
									 (propName[i] != GCF_SYS_NAME_SEP) &&
									 (propName[i] != GCF_SCOPE_NAME_SEP)) {
			LOG_TRACE_COND_STR("isValidPropName(" << propName << "): illegal character at pos " << i << ":" << propName[i]);
			return (false);
		}
	}
	return (true);
}

//
// isValidScope(name)
//
bool PVSSinfo::isValidScope(const char* scopeName)
{
	return (isValidPropName(scopeName));
#if 0
	ASSERT(scopeName);

	char			doubleSep[] = {GCF_SCOPE_NAME_SEP, GCF_SCOPE_NAME_SEP, 0};
	unsigned int	length 		= strlen(scopeName);
	char*			sysNameSep	= strchr(scopeName, ':');
	if (sysNameSep > 0) {
		length -= (sysNameSep + 1 - scopeName);
		scopeName = sysNameSep + 1;
	}
	if (scopeName[0] == GCF_SCOPE_NAME_SEP || scopeName[length - 1] == GCF_SCOPE_NAME_SEP ) {
		return (false);
	}
	if (strstr(scopeName, doubleSep) != 0) {
		return (false);
	}
	for(unsigned short i = 0; i < length; i++) {
		if (!isalnum(scopeName[i]) && scopeName[i] != GCF_SCOPE_NAME_SEP) {
			return (false);
		}
	}
	return (true);
#endif
}


  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
