//#  ConfigurationMgr.cc: The ACC confMgr for all parameter manipulations
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
//#  TODO: support 'instances=' in appendTU
//#
//#  Note: TRACE_CALC level is used for database queries
//#
//#  Note:This source is read best with tabstop 4.
//#
//#  $Id$

#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Attrib.h>
#include "ConfigurationMgr.h"
#include "ParCollRecord.h"
#include "database/PO_ParCollRecord.h"

using namespace LOFAR::PL;

namespace LOFAR {
  namespace ACC {

//#
//# Default constructor
//#
ConfigurationMgr::ConfigurationMgr()
{
	//# TBW: connect based on parameterset values.
}

//#
//# ConfigurationMgr(hostname, databasename, password)
//#
ConfigurationMgr::ConfigurationMgr(const string& hostname,
								   const string& databasename,
								   const string& password)
{
	itsBroker.connect(databasename, password);
}

//#
//# Destructor
//#
ConfigurationMgr::~ConfigurationMgr()
{}

string ConfigurationMgr::resolveVersionnr(const string&		aComponent, 
										  const string&		aQualification) const
{
	// Construct a query to find the version
	Query::Expr		query = (
		(attrib<ParCollRecord>("name") == aComponent) &&
		(attrib<ParCollRecord>("qualification") == aQualification));
	ParCollRecord		 				PCR;
	TPersistentObject<ParCollRecord> 	tpoPCR(PCR);

	LOG_TRACE_CALC_STR("Query = " << query);

	// Get the PT from the database
	ASSERTSTR(tpoPCR.retrieveInPlace(query, 1) >= 1,
					"Component >" << aComponent << "<, qualification >" <<
					aQualification << "< not in the database!");

	LOG_TRACE_FLOW_STR(formatString("resolveVersionnr(%s,%s)=%s", 
									aComponent.c_str(), aQualification.c_str(),
									PCR.getVersionNr().c_str()));

	return (PCR.getVersionNr());
}

//#
//#	Internal recursive routine:
//# appendTU(TemplateUnion, componentName, versionNr)
//#
void ConfigurationMgr::appendTU(TemplateUnion*	theTU,
							    const string&	parentname,
							    const string&	componentName,
								uint32			sequencenr,
							    const string&	versionNr)
{
	LOG_TRACE_FLOW(formatString("appendTU:%s|%s|%d|%s", parentname.c_str(), 
						componentName.c_str(), sequencenr, versionNr.c_str()));

	// Construct a query to find the PT
	Query::Expr		query = (
		(attrib<ParCollRecord>("name") == componentName) &&
		(attrib<ParCollRecord>("version") == versionNr));
	ParCollRecord		 				subTemplate;
	TPersistentObject<ParCollRecord> 	tpoPCR(subTemplate);

	LOG_TRACE_CALC_STR("Query = " << query);

	// Get the PT from the database
	ASSERTSTR(tpoPCR.retrieveInPlace(query, 1) >= 1,
					"Component >" << componentName << "<, version >" <<
					versionNr << "< not in database!");

	// mainComp contains whole parential hierarchy.
	string	mainComp(parentname);
	if (mainComp != "") {								// correct parentname if
		mainComp += ".";								// not root level
	}
	LOG_TRACE_VAR(formatString("mainComp=%s", mainComp.c_str()));

	// construct sequencenumber string
	string seqNrStr;
	if (sequencenr > 0) {
		seqNrStr = formatString("%d.", sequencenr);
	}

	// Walk through PT and handle the lines.
	string	paramLine;
	uint32	offset = 0;
	while (subTemplate.getLine(paramLine, &offset)) {	// get next line
		LOG_TRACE_VAR_STR("paramLine=" << paramLine);

		// There are four forms possible for the left part:
		// componentname.parameter
		// componentname.versionnr
		// componentname.subcomponent.versionnr
		// componentname.subcomponent.seqnr.versionnr
		//
		// PTcomp . PTsubcomp . PTseqnr . PTkey
		// <--------- PTfullcomp ------>

		// split line into handy parts
		string left      (keyPart  (paramLine));		// split line
		string right     (valuePart(paramLine));
		string PTkey     (keyName   (left));
		string PTfullcomp(moduleName(left));			// full component name
		
		if (PTkey != PC_KEY_VERSIONNR) {				// parameter def?
			LOG_TRACE_LOOP("Add parameterdef to TU");	// just add.
			theTU->insert(make_pair(mainComp+PTfullcomp+"."+seqNrStr+PTkey, 
									right));
		}
		else {											// versionnr line
			if (PTfullcomp == componentName) {			// own versionnr?
				LOG_TRACE_LOOP("Add own versionnr to TU");	// just add
				theTU->insert(make_pair(mainComp+PTfullcomp+"."+seqNrStr+PTkey, 
										right));
			}
			else {										// Vnr of other comp.
				uint32 PTseqnr = seqNr(keyName(PTfullcomp));// lst part is nr?
				if (PTseqnr > 0) {
					PTfullcomp = moduleName(PTfullcomp);// strip of number
				}
				string PTcomp(moduleName(PTfullcomp));// first block is parent
				string PTsubcomp = keyName(PTfullcomp);	// real comp is last part
				string subCompVersion;
				if ((right == PC_QUAL_STABLE) || (right == PC_QUAL_TEST) || 
												 (right == PC_QUAL_DEVELOP)) {
					// user want latest version of given qualification
					subCompVersion = resolveVersionnr(PTsubcomp, right);
				}
				else {									// 'hard' versionnr
					subCompVersion = right;
				}
				LOG_TRACE_LOOP_STR("version of subcomponent=" << subCompVersion);

				// Call ourself to add this part of the tree
				appendTU(theTU, mainComp+PTcomp+seqNrStr, PTsubcomp, 
														PTseqnr, subCompVersion); 
			}
		} // versionnr line
	} // while lines in template
	LOG_TRACE_FLOW("appendTU return");
}

//#
//# createTemplateUnion (componentName, versionNr)
//#
TemplateUnion*	ConfigurationMgr::createTemplateUnion(const string&	componentName,
										 			  const string&	versionNr) 
{
	TemplateUnion*		theTU = new TemplateUnion;

	appendTU (theTU, "", componentName, 0, versionNr);		// recursive append
	
	return (theTU);
}

//#
//# createParameterUnion (templateUnion)
//#
ParameterUnion*	ConfigurationMgr::createParameterUnion(const TemplateUnion& aTU) const
{
	return(0);
}

//#
//# createParamterSet (templateUnion, parameterUnion)
//#
ParameterSet*	ConfigurationMgr::createParameterSet(const TemplateUnion&  aTU,
										 			 const ParameterUnion& aPU) const
{
	return(0);
}

//#
//# deleteTU(componentName, versionNr)
//#
bool	ConfigurationMgr::deleteTU(const string& componentName, 
								   const string& versionNr) const
{
	return(true);
}

//#
//# deletePT (componentName, versionNr)
//#
bool	ConfigurationMgr::deletePT(const string& componentName, 
								   const string& versionNr) const
{
	return(true);
}

//#
//# deletePS(componentName, versionNr)
//#
bool	ConfigurationMgr::deletePS(const string& componentName, 
								   const string& versionNr) const
{
	return(true);
}


//#
//# getTU(componentName, versionNr)
//#
TemplateUnion*		ConfigurationMgr::getTU(const string& componentName, 
								 			const string& versionNr) const
{
	return(0);
}

//#
//# getPT(componentName, versionNr)
//#
ParameterTemplate*	ConfigurationMgr::getPT(const string& componentName, 
											const string& versionNr) const
{
	Query::Expr		query = (
		(attrib<ParCollRecord>("name") == componentName) &&
		(attrib<ParCollRecord>("version") == versionNr));
	ParCollRecord		 				PCR;
	TPersistentObject<ParCollRecord> 	tpoPCR(PCR);

	LOG_TRACE_CALC_STR("Query = " << query);

	tpoPCR.retrieveInPlace(query, 1);

	ParameterTemplate*		PTptr = new ParameterTemplate;
	PTptr->adoptBuffer(PCR.getCollection());

	return (PTptr);
}

//#
//# getPS(componentName, versionNr)
//#
ParameterSet*		ConfigurationMgr::getPS(const string& componentName, 
											const string& versionNr) const
{
	return(0);
}


//#
//# addTU(componentName, versionNr)
//#
bool	ConfigurationMgr::addTU (TemplateUnion&		aTU) const
{
	return(true);
}

//#
//# addPT(componentName, versionNr)
//#
bool	ConfigurationMgr::addPT (ParameterTemplate&	aPT) const
{
	string	name 	= aPT.getName();
	string	version = aPT.getVersionNr();
	string	qual	= aPT.getQualification();

	LOG_TRACE_FLOW (formatString("Saving PT %s:%s(%s)", name.c_str(), 
											version.c_str(), qual.c_str()));
	
	string			contents;
	aPT.writeBuffer(contents);
	ParCollRecord	PCRec (name, version, qual, contents);
	TPersistentObject<ParCollRecord>	tpoPCR(PCRec);

	itsBroker.save(tpoPCR);

	return (true);
}

//#
//# addPS(componentName, versionNr)
//#
bool	ConfigurationMgr::addPS (ParameterSet&		aPS) const
{
	string	name 	= aPS.getName();
	string	version = aPS.getVersionNr();
	string	qual	= "development";

	LOG_TRACE_FLOW (formatString("Saving PS %s:%s(%s)", name.c_str(), 
											version.c_str(), qual.c_str()));
	
	string			contents;
	aPS.writeBuffer(contents);
	ParCollRecord	PCRec (name, version, qual, contents);
	TPersistentObject<ParCollRecord>	tpoPCR(PCRec);

	itsBroker.save(tpoPCR);

	return (true);
}


  } // namespace ACC
} // namespace LOFAR
