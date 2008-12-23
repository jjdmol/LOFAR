//#  ConfigurationMgr.h: The ACC ConfMgr for all parameter manipulations
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_CONFIGURATIONMGR_H
#define LOFAR_ACCBIN_CONFIGURATIONMGR_H

// \file
// The ACC configuration manager handles all database actions for parameters.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_string.h>
#include <PL/PersistenceBroker.h>
#include <Common/ParameterSet.h>


namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

//# Description of class.
// The ConfigurationMgr is a class that performs a lot of tasks on more than
// one ParameterCollection. It is capable of converting some flavors of 
// ParameterCollections in to others and can construct Unions from other
// Collections.
class ConfigurationMgr
{
public:
	// Default constructable, uses parameterfile settings.
	ConfigurationMgr();

	// Construction with overriding parameterfile settings.
	ConfigurationMgr(const string&	hostname,
					 const string&	databasename,
					 const string&  password);

	~ConfigurationMgr();

	// Creation of several flavors of parameter collections
	TemplateUnion*	createTemplateUnion	(const string&	componentName,
										 const string&	versionNr); // const;
	ParameterUnion*	createParameterUnion(const TemplateUnion& aTU) const;
	ParameterSet*	createParameterSet  (const TemplateUnion&  aTU,
										 const ParameterUnion& aPU) const;

	// Delete collections from the database
	bool	deleteTU(const string& componentName, const string& versionNr) const;
	bool	deletePT(const string& componentName, const string& versionNr) const;
	bool	deletePS(const string& componentName, const string& versionNr) const;

	// Get collections from the database
	TemplateUnion*		getTU   (const string& componentName, 
								 const string& versionNr) const;
	ParameterTemplate*	getPT   (const string& componentName, 
								 const string& versionNr) const;
	ParameterSet*		getPS   (const string& componentName, 
								 const string& versionNr) const;

	// Add collections to the database
	bool	addTU (TemplateUnion&		aTU) const;		// private?
	bool	addPT (ParameterTemplate&	aPT) const;
	bool	addPS (ParameterSet&		aPS) const;		// private?

private:
	void	appendTU(TemplateUnion*		aTU,
					 const string&		treebase,
					 const string&		componentName,
					 uint32				sequencenr,
					 const string&		versionNr);
	string resolveVersionnr(const string&		aKey, 
							const string&		aValue) const;

	LOFAR::PL::PersistenceBroker		itsBroker;
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
