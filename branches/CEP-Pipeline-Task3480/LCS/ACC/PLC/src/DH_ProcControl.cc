//# DH_ProcControl.cc: Implements the Application Controller command protocol.
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

//# Includes
#include <PLC/DH_ProcControl.h>


namespace LOFAR {
  namespace ACC {
    namespace PLC {

// Constructor
DH_ProcControl::DH_ProcControl() :
	DataHolder      ("", "DH_ProcControl"),
	itsVersionNumber(0),
	itsCommand      (0),
	itsResult       (0)
{
	setExtraBlob ("Extra", 1);
}


DH_ProcControl::DH_ProcControl(const PCCmd	aCommand) :
	DataHolder      ("", "DH_ProcControl"),
	itsVersionNumber(0),
	itsCommand      (0),
	itsResult       (0)
{
	setExtraBlob ("Extra", 1);
	this->init();
	setCommand(aCommand);
}


// Destructor
DH_ProcControl::~DH_ProcControl() 
{ }

// Copying is allowed.
DH_ProcControl::DH_ProcControl(const DH_ProcControl& that) :
	DataHolder      (that),
	itsVersionNumber(that.itsVersionNumber),
	itsCommand      (that.itsCommand),
	itsResult       (that.itsResult)
{ }

DH_ProcControl*		DH_ProcControl::clone() const
{
	return new DH_ProcControl(*this);
}


// Redefines the init function.
void 	DH_ProcControl::init()
{
	LOG_TRACE_RTTI ("DH_ProcControl:init");

	// Initialize the fieldset
	initDataFields();

	// Add the fields to the definition
	addField ("VersionNumber", BlobField<uint16>(1));
	addField ("Command", 	   BlobField<int16>(1));	// PCCmd
	addField ("Result", 	   BlobField<uint16>(1));

	// create the data blob (calls fillpointers).
	createDataBlock();

}


//# ---------- private ----------

// Implement the initialisation of the pointers
void	DH_ProcControl::fillDataPointers() {
	LOG_TRACE_RTTI ("DH_ProcControl:fillDataPointers");

	itsVersionNumber = getData<uint16>("VersionNumber");
	itsCommand 		 = getData<int16> ("Command");
	itsResult 		 = getData<uint16>("Result");

	*itsVersionNumber = 0x0100;		// TODO define a constant WriteVersion
}


    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

