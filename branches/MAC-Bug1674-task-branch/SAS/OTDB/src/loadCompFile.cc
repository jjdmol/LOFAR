//#  loadCompFile.cc: Routine from TreeMaint for loading the node definitions.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/wSpaceSplit.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/ParamTypeConv.h>
#include <OTDB/UnitConv.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/misc.h>

namespace LOFAR {
  namespace OTDB {

//
// saveParam(OTDBparam) : bool
//
bool TreeMaintenance::saveParam(OTDBparam&	aParam)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "saveVICparam");
	try {
		// remove ' chars from description
		string	cleanDesc(aParam.description);
		size_t	pos = 0;
		while((pos = cleanDesc.find_first_of("'",pos)) != string::npos) {
			cleanDesc.erase(pos, 1);
		}
		// remove ' chars from limits
		string	cleanLimits(aParam.limits);
		pos = 0;
		while((pos = cleanLimits.find_first_of("'",pos)) != string::npos) {
			cleanLimits.erase(pos, 1);
		}

		LOG_TRACE_FLOW(formatString("saveVCparam(%d,'%s',%d,%d,'%s','%s')",
							aParam.nodeID(),
							aParam.name.c_str(),
							aParam.type,
							aParam.unit,
							cleanLimits.c_str(),
							cleanDesc.c_str()));

		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT saveVICparamDef(%d,%d,'%s',%d::int2,%d::int2,%d::int2,%d::int2,'%s'::boolean,'%s'::text,'%s'::text)",
							itsConn->getAuthToken(),
							aParam.nodeID(),
							aParam.name.c_str(),
							aParam.type,
							aParam.unit,
							aParam.pruning,
							aParam.valMoment,
							(aParam.runtimeMod ? "TRUE" : "FALSE"),
							cleanLimits.c_str(),
							cleanDesc.c_str()));

		// Analyse result
		nodeIDType		vParamID;
		res[0]["savevicparamDef"].to(vParamID);
		if (!vParamID) {
			itsError = "Unable to save the node";
			return (false);
		}

		xAction.commit();
		aParam.itsNodeID = vParamID;
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during saving VICparam:") + ex.what();
		LOG_FATAL(ex.what());
		return (false);
	}

	return (false);
}

//
// getComponentNode (nodeID) : VICnodeDef
//
VICnodeDef TreeMaintenance::getComponentNode (nodeIDType		aNodeID)
{
	VICnodeDef		empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR ("getVCnode(" << aNodeID << ")" );

	work	xAction(*(itsConn->getConn()), "getVCnode");
	try {
		result res = xAction.exec("SELECT * from getVICnodeDef(" +
								  toString(aNodeID) + ")");
		if (res.empty()) {
			return (empty);
		}

		return (VICnodeDef(res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getVCnode:") + ex.what();
		LOG_FATAL(ex.what());
	}
	return (empty);
}

//
// getComponentParams (nodeID) : vector<OTDBparam>
//
vector<OTDBparam> TreeMaintenance::getComponentParams (nodeIDType		aNodeID)
{
	vector<OTDBparam>		resultVec;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (resultVec);
	}

	LOG_TRACE_FLOW_STR ("getVCparams(" << aNodeID << ")" );

	work	xAction(*(itsConn->getConn()), "getVCparams");
	try {
		result res = xAction.exec("SELECT * from getVCparams(" +
								  toString(aNodeID) + ")");
		if (res.empty()) {
			return (resultVec);
		}

		for (result::size_type i = 0; i < res.size(); ++i) {
			resultVec.push_back(OTDBparam(0, res[i]));
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getVCparams:") + ex.what();
		LOG_FATAL(ex.what());
	}
	return (resultVec);
}

//
// getComponentList(namefragment, toponly): vector<VICnodedef>
//
vector<VICnodeDef>	TreeMaintenance::getComponentList (
										const string& 	namefragment,
										bool			topOnly)
{
	vector<VICnodeDef>		resultVec;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (resultVec);
	}

	// note: namefragement has the syntax: [#]namefragment[{versionnr}][%]
	//		 always strip off #
	//		 always remove and convert versionnr
	uint32		theVersion = getVersionNrFromName(namefragment);
	string		theName    = cleanNodeName(namefragment);
	if ((*(namefragment.rbegin()) == '%') && (*(theName.rbegin()) != '%')) {
		theName.append("%");
	}

	LOG_TRACE_FLOW_STR ("getCompList(" << theName <<","<< theVersion <<","<< 
											toString(topOnly) << ")" );

	work	xAction(*(itsConn->getConn()), "getCompList");
	try {
		result res = xAction.exec("SELECT * from getVCNodeList('" +
												theName + "'," + 
												toString(theVersion) + ",'" +
												toString(topOnly) + "')");
		if (res.empty()) {
			return (resultVec);
		}

		for (result::size_type i = 0; i < res.size(); ++i) {
			resultVec.push_back(VICnodeDef(res[i]));
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getComponentList:") 
						+ ex.what();
		LOG_FATAL(ex.what());
	}
	return (resultVec);
}

// 
// saveComponentNode (VICnodeDef) : bool
//
bool TreeMaintenance::saveComponentNode	(VICnodeDef&	aNode)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "saveVCnode");
	try {
		// remove ' chars from description
		string	cleanDesc(aNode.description);
		size_t	pos = 0;
		while((pos = cleanDesc.find_first_of("'",pos)) != string::npos) {
			cleanDesc.erase(pos, 1);
		}
		// remove ' chars from limits
		string	cleanConstraints(aNode.constraints);
		pos = 0;
		while((pos = cleanConstraints.find_first_of("'",pos)) != string::npos) {
			cleanConstraints.erase(pos, 1);
		}

		LOG_DEBUG (formatString("saveComponentNode(%d,'%s',%d,%d,'%s','%s')", 
							aNode.nodeID(), aNode.name.c_str(), aNode.version,
							aNode.classif, cleanConstraints.c_str(), cleanDesc.c_str()));
					
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT saveVCnode(%d,%d,'%s',%d,%d::int2,'%s'::text,'%s'::text)",
							itsConn->getAuthToken(),
							aNode.nodeID(),
							aNode.name.c_str(),
							aNode.version,
							aNode.classif,
							cleanConstraints.c_str(),
							cleanDesc.c_str()));

		// Analyse result
		nodeIDType		vNodeID;
		res[0]["savevcnode"].to(vNodeID);
		if (!vNodeID) {
			itsError = "Unable to save the node";
			return (false);
		}

		xAction.commit();
		aNode.itsNodeID = vNodeID;
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during save of VICnode:") + ex.what();
		LOG_FATAL(ex.what());
		return (false);
	}

	return (false);
}

//
// getfullComponentName(node&)
//
// Returns the full componentname including the versionnumber
string	TreeMaintenance::getFullComponentName(VICnodeDef&	aNode)
{
	return(formatString("#%s{%s}", aNode.name.c_str(), VersionNr(aNode.version).c_str()));
}

// [private]
// getNodeDef (name, version, classif) : VICnodedef
//
VICnodeDef	TreeMaintenance::getNodeDef (const string&	name,
							   uint32		version,
							   classifType	classif)
{
	VICnodeDef		empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR("getVCnode(" << name <<","<< version <<","<< classif <<")");

	work	xAction(*(itsConn->getConn()), "getVCnode");
	try {
		result res = xAction.exec("SELECT * from getVICnodeDef('" +
								  name + "'," + 
								  toString(version) + "," + 
								  toString(classif) + "::int2)");
		if (res.empty()) {
			return (empty);
		}

		return (VICnodeDef(res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getVCnode:") + ex.what();
		LOG_FATAL(ex.what());
	}
	return (empty);
}

//
// isTopComponent (nodeID)
//
bool	TreeMaintenance::isTopComponent	(nodeIDType		aNodeID)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR ("isTopComponent(" << aNodeID <<")");

	work	xAction(*(itsConn->getConn()), "isTopComponent");
	try {
		result res = xAction.exec("SELECT * from isTopComponent('" +
								  toString(aNodeID) + "')");
		bool		isTop;
		res[0]["isTopComponent"].to(isTop);
		return (isTop);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during isTopComponent:") + ex.what();
		LOG_FATAL(ex.what());
	}
	return (false);

}


//
// deleteComponentNode(nodeID)
//
// Delete a component node (when there are no references to it anymore
//
bool	TreeMaintenance::deleteComponentNode(nodeIDType		aNodeID)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("deleteVCnode(" << aNodeID << ")");

	work	xAction(*(itsConn->getConn()), "removeVCnode");
	try {
		result res = xAction.exec("SELECT * from removeVCnode(" +
								  toString(itsConn->getAuthToken()) + "," +
								  toString(aNodeID) + ")");

		// when something goes wrong an exception is thrown
		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during deleteComponent:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}
	return (false);

}

//
// loadComponentFile(treeID, filename): nodeID
//
// a VIC tree is build up from single components. The definition of a
// component can loaded from a file with this call
nodeIDType	TreeMaintenance::loadComponentFile (const string&	filename,
												const string&	forcedVersionNr,
												const string&	forcedQualifier)
{
	bool	versionNrIsForced(!forcedVersionNr.empty());
	bool	qualifierIsForced(!forcedQualifier.empty());

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	// Try opening the input file
	ifstream	inFile;
	LOG_DEBUG_STR("Opening componentfile " << filename);
	inFile.open(filename.c_str());
	if (!inFile) {
		itsError = string("Cannot open input file ") + filename;
		LOG_ERROR_STR(itsError);
	 	return (0);
	}

	// get convertors (from database)
	ParamTypeConv	PTconv(itsConn);
	UnitConv		UTconv(itsConn);
	ClassifConv		CTconv(itsConn);

	// scan the inputfile
	const uint16	maxLineLen = 1024;
	char			line [maxLineLen];
	uint16			lineNr     = 0;
	bool			inError    = false;
	nodeIDType		topNodeID  = -1;
	while (!inError && inFile.getline(line, maxLineLen)) {
		lineNr++;

		// skip empty and comment dos lines
		if (!line[0] || line[0] == '#' || line[0] == 0x0d) {
			continue;
		}
		
		// split line into fields
		vector<string>	args = wSpaceSplit(line,2);
		if (args.empty()) {
			continue;
		}
		LOG_TRACE_CALC_STR (lineNr << ":" << line);

		// -- NODE --
		if (!args[0].compare("node")) {
			vector<string>	args = wSpaceSplit(line,6);
			// syntax: node name version qual node-constraint description
			// all elements are required
			if (args.size() != 6) {
				itsError = toString(lineNr) + 
							": 'node'-line needs 6 columns not " + toString(args.size());
				LOG_ERROR_STR(itsError);
				inError = true;
				break;
			}
			// construct object (name, version, classif, constr. descr.)
		 	VICnodeDef	topNode(args[1], VersionNr(versionNrIsForced ? forcedVersionNr : args[2]), 
								CTconv.get(qualifierIsForced ? forcedQualifier : args[3]), 
								args[4], args[5]);
			saveComponentNode (topNode);			// private call
			topNodeID = topNode.itsNodeID;
		}
		// -- USES --
		else if (!args[0].compare("uses")) {
			vector<string>	args = wSpaceSplit(line,6);
			// syntax: uses name min_version classif instances
			if (args.size() != 6) {
				itsError = toString(lineNr) + 
							": 'uses'-line needs 6 columns not " + toString(args.size());
				LOG_FATAL(itsError);
				inError = true;
				break;
			}

			// Check that module that is referenced exists in the database.
			VICnodeDef	ParentNode = getNodeDef(args[1], 
											VersionNr(versionNrIsForced ? forcedVersionNr  : args[2]), 
											CTconv.get(qualifierIsForced ? forcedQualifier : args[3]));		// private call
			if (!ParentNode.nodeID()) {
				itsError = toString(lineNr) + ": Node " + args[1] +"," +
									(versionNrIsForced ? forcedVersionNr : args[2]) + "," + 
									(qualifierIsForced ? forcedQualifier : args[3]) + " not found";
				LOG_FATAL(itsError);
				inError = true;
				break;
			}

			// NOTE: parent-child relation is stored as a parameter
			//		 at the parent, telling the number of childs.
			OTDBparam		AttachedChild;
			AttachedChild.itsNodeID   = topNodeID;
			AttachedChild.name 		  = getFullComponentName(ParentNode);
			AttachedChild.index 	  = 0;		// not used in VICparamdef
			AttachedChild.type 		  = PTconv.get("int");
			AttachedChild.unit 		  =	UTconv.get("-");
			AttachedChild.pruning 	  = 0;
			AttachedChild.valMoment   = 0;
			AttachedChild.runtimeMod  = false;
			AttachedChild.limits	  = args[4];
			AttachedChild.description = args[5];
			saveParam (AttachedChild);
		}
		// -- PAR -- 
		else if (!args[0].compare("par")) {
			vector<string>	args = wSpaceSplit(line,10);
			// syntax: par name type unit valMoment RTmod pruning 
			//									value constraint description
			if (args.size() != 10) {
				itsError = toString(lineNr) + 
							": 'par'-line needs 10 columns not " + toString(args.size());
				LOG_FATAL(itsError);
				inError = true;
				break;
			}

			// construct lacking optional arguments
			OTDBparam		AttachedChild;
			AttachedChild.itsNodeID   = topNodeID;
			AttachedChild.name 		  = args[1];
			AttachedChild.index 	  = 0;
			AttachedChild.type 		  = PTconv.get(args[3]);	
			AttachedChild.unit 		  =	UTconv.get(args[4]);
			AttachedChild.pruning 	  = StringToInt16(args[5]);
			AttachedChild.valMoment   = StringToInt16(args[6]);
			AttachedChild.runtimeMod  = (args[2].find("O",0)) ? true : false;
			AttachedChild.limits	  = args[7];
			AttachedChild.description = args[9];
			saveParam (AttachedChild);
			// TODO: args[8] constraint, args[6] valmoment
		}
		// -- UNKNOWN --
		else {
			itsError = formatString("line %d does not start with a keyword",
									lineNr);
			LOG_FATAL(itsError);
			inError = true;
			break;
		}
	}

	inFile.close();

	return (inError ? 0 : topNodeID);
}

  } // namespace OTDB
} // namespace LOFAR

