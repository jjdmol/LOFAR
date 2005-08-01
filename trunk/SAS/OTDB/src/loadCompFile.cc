//#  loadCompFile.cc: Routine from VICadmin for loading the node definitions.
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
#include <Common/lofar_fstream.h>
#include <Common/lofar_vector.h>
#include <OTDB/VICadmin.h>
#include <OTDB/wSpaceSplit.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/ParamType.h>
#include <OTDB/UnitType.h>
#include <OTDB/ClassifType.h>
#include <OTDB/misc.h>

namespace LOFAR {
  namespace OTDB {

bool VICadmin::saveNode	(VICnodeDef&	aNode)
{
	if (!itsConn->connect()) {
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "saveVCnode");

	try {
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT saveVCnode(%d,%d,'%s',%d,%d::int2,'%s'::text,'%s'::text)",
							itsConn->getAuthToken(),
							aNode.nodeID(),
							aNode.name.c_str(),
							aNode.version,
							aNode.classif,
							aNode.constraints.c_str(),
							aNode.description.c_str()));

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
	catch (Exception& ex) {
		LOG_FATAL_STR(ex.what());
		return (false);
	}

	return (false);
}

bool VICadmin::saveParam(OTDBparam&	aParam)
{
	if (!itsConn->connect()) {
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "saveVICparam");

	try {
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
							aParam.limits.c_str(),
							aParam.description.c_str()));

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
	catch (Exception& ex) {
		LOG_FATAL_STR(ex.what());
		return (false);
	}

	return (false);
}
bool VICadmin::deleteNode (VICnodeDef&	aNode)
{
	if (!itsConn->connect()) {
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "removeVCnode");

	try {
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT removeVCnode(%d,%d,%d)",
							itsConn->getAuthToken(),
							aNode.nodeID()));

		// Analyse result
		bool		removeOK;
		res[0]["removevcnode"].to(removeOK);
		if (!removeOK) {
			itsError = "Unable to remove the node.";
			return (false);
		}
		// invalidate the nodeID of the object.
		xAction.commit();
		aNode.itsNodeID = 0;
		return (true);
	}
	catch (Exception& ex) {
		LOG_FATAL_STR(ex.what());
		return (false);
	}

	return (false);

}

VICnodeDef VICadmin::getNode (nodeIDType		aNodeID)
{
	VICnodeDef		empty;

	if (!itsConn->connect()) {
		return (empty);
	}

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
		itsError = string("Exception during VICadmin:getVCnode:") + ex.what();
	}
	return (empty);
}

VICnodeDef	VICadmin::getNode (const string&	name,
							   uint32			version,
							   treeClassifType	classif)
{
	VICnodeDef		empty;

	if (!itsConn->connect()) {
		return (empty);
	}

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
		itsError = string("Exception during VICadmin:getVCnode:") + ex.what();
	}
	return (empty);
}

vector<VICnodeDef>	VICadmin::getTopComponentList (const string& namefragment)
{
	vector<VICnodeDef>		resultVec;

	if (!itsConn->connect()) {
		return (resultVec);
	}

	work	xAction(*(itsConn->getConn()), "getTopCompList");
	try {
		result res = xAction.exec("SELECT * from getVCtopNodeList('" +
								  namefragment + "')");
		if (res.empty()) {
			return (resultVec);
		}

		for (result::size_type i = 0; i < res.size(); ++i) {
			resultVec.push_back(VICnodeDef(res[i]));
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during VICadmin:getTopComponentList:") 
						+ ex.what();
	}
	return (resultVec);
}

//
// loadComponentFile(treeID, filename): nodeID
//
// a VIC tree is build up from single components. The definition of a
// component can loaded from a file with this call
nodeIDType	VICadmin::loadComponentFile (const string&	filename)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	// Try opening the input file
	ifstream	inFile;
	LOG_DEBUG_STR("Opening componentfile " << filename);
	inFile.open(filename.c_str());
	if (!inFile) {
		LOG_ERROR_STR("Cannot open input file " << filename);
	 	return (0);
	}

	// get convertors (from database)
	ParamType	PTconv(itsConn);
	UnitType	UTconv(itsConn);
	ClassifType	CTconv(itsConn);

	// scan the inputfile
	const uint16	maxLineLen = 1024;
	char			line [maxLineLen];
	uint16			lineNr     = 0;
	bool			inError    = false;
	nodeIDType		topNodeID  = -1;
	while (!inError && inFile.getline(line, maxLineLen)) {
		lineNr++;

		// skip empty and comment lines
		if (!line[0] || line[0] == '#') {
			continue;
		}
		
		// split line into fields
		vector<string>	args = wSpaceSplit(line,2);
		if (args.empty()) {
			continue;
		}
		LOG_TRACE_FLOW_STR (lineNr << ":" << line);

		// -- NODE --
		if (!args[0].compare("node")) {
			vector<string>	args = wSpaceSplit(line,6);
			// syntax: node name version qual node-constraint description
			// all elements are required
			if (args.size() < 6) {
				LOG_FATAL_STR(lineNr << ": Too less arguments for node-line");
				inError = true;
				break;
			}
			// construct object (name, version, classif, constr. descr.)
		 	VICnodeDef	topNode(args[1], VersionNr(args[2]), 
										CTconv.get(args[3]), args[4], args[5]);
			saveNode (topNode);
			topNodeID = topNode.itsNodeID;
		}
		// -- USES --
		else if (!args[0].compare("uses")) {
			vector<string>	args = wSpaceSplit(line,5);
			// syntax: uses name min_version classif instances
			if (args.size() < 5) {
				LOG_FATAL_STR(lineNr << ": Too less arguments for uses-line");
				inError = true;
				break;
			}

			// Check that module that is referenced exists in the database.
			VICnodeDef	ChildNode = getNode(args[1], 
											VersionNr(args[2]), 
											CTconv.get(args[3]));
			if (!ChildNode.nodeID()) {
				LOG_FATAL_STR (lineNr << ": Node "<< args[1] << "," << args[2] 
									  << "," << args[3] << " not found");
				inError = true;
				break;
			}

			// NOTE: parent-child relation is stored as a parameter
			//		 at the parent, telling the number of childs.
			OTDBparam		AttachedChild;
			AttachedChild.itsNodeID   = topNodeID;
			AttachedChild.name 		  = "#" + args[1];
			AttachedChild.index 	  = 0;		// not used in VICparamdef
			AttachedChild.type 		  = PTconv.get("int");
			AttachedChild.unit 		  =	UTconv.get("-");
			AttachedChild.pruning 	  = 0;
			AttachedChild.valMoment   = 0;
			AttachedChild.runtimeMod  = false;
			AttachedChild.limits	  = args[4];
			AttachedChild.description = "";
			saveParam (AttachedChild);
		}
		// -- PAR -- 
		else if (!args[0].compare("par")) {
			vector<string>	args = wSpaceSplit(line,10);
			// syntax: par name type unit valMoment RTmod pruning 
			//									value constraint description
			if (args.size() < 9) {
				LOG_FATAL_STR(lineNr << ": Too less arguments for par-line");
				inError = true;
				break;
			}

			// construct lacking optional arguments
			while (args.size() < 10) {
				args.push_back("");
			}
			OTDBparam		AttachedChild;
			AttachedChild.itsNodeID   = topNodeID;
			AttachedChild.name 		  = args[1];
			AttachedChild.index 	  = 0;
			AttachedChild.type 		  = PTconv.get(args[3]);	
			AttachedChild.unit 		  =	UTconv.get(args[4]);
			AttachedChild.pruning 	  = StringToInt16(args[5]);
			AttachedChild.valMoment   = 0;	// toValMoment(args[6]);
			AttachedChild.runtimeMod  = (args[2].find("O",0)) ? true : false;
			AttachedChild.limits	  = args[7];
			AttachedChild.description = args[9];
			saveParam (AttachedChild);
			// TODO: args[8] constraint, args[6] valmoment
		}
		// -- UNKNOWN --
		else {
			LOG_FATAL_STR("line " << lineNr << " does not start with keyword");
			inError = true;
			break;
		}
	}

	inFile.close();

	return (inError ? 0 : topNodeID);
}

  } // namespace OTDB
} // namespace LOFAR

