//#  DH_OTDBlog.cc: Implements the OTDB log command dataholder.
//#
//#  Copyright (C) 2007
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
//#	 Abstract:
//#	 This class implements the command protocol between an application manager
//#	 (MAC for instance) and an Application Controller (=ACC package).
//#	 The AM has the client role, the AC the server role.
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <ALC/DH_OTDBlog.h>


namespace LOFAR {
  namespace ACC {
    namespace ALC {

// Constructor
DH_OTDBlog::DH_OTDBlog() :
	DataHolder ("", "DH_OTDBlog"),
	itsVersionNumber(0),
{
	LOG_TRACE_OBJ("DH_applControl()");
	setExtraBlob ("Extra", 1);
}


// Destructor
DH_OTDBlog::~DH_OTDBlog() 
{
	LOG_TRACE_OBJ("~DH_applControl()");
}

// Copying is allowed.
DH_OTDBlog::DH_OTDBlog(const DH_OTDBlog& that) :
	DataHolder(that),
	itsVersionNumber(that.itsVersionNumber),
{
	setExtraBlob ("Extra", 1);
}

DH_OTDBlog*		DH_OTDBlog::clone() const
{
	return new DH_OTDBlog(*this);
}

DH_OTDBlog*		DH_OTDBlog::makeDataCopy() const
{
	DH_OTDBlog*		newDHAC = new DH_OTDBlog;
	newDHAC->init();
	newDHAC->setMessages(getMessages());
	newDHAC->pack();

	return (newDHAC);
}	

// Redefines the init function.
void 	DH_OTDBlog::init()
{
	LOG_TRACE_FLOW("DH_OTDBlog:init()");

	initDataFields();

	addField ("VersionNumber", BlobField<uint16>(1));

	createDataBlock();

}

// Redefine the pack function.
void	DH_OTDBlog::pack()
{
	LOG_TRACE_RTTI("DH_OTDBlog:pack()");

	BlobOStream&	bos = createExtraBlob();		// attached to dataholder

	bos <<	itsOptions;
	
	DataHolder::pack();
}

// Redefine the unpack function.
void	DH_OTDBlog::unpack()
{
	LOG_TRACE_RTTI("DH_OTDBlog:unpack()");

	DataHolder::unpack();

	int32			version;
	bool			found;
	BlobIStream&	bis = getExtraBlob(found, version);
	ASSERTSTR (found, "DH_OTDBlog::read has no extra blob");

	bis >>	itsOptions;
	bis.getEnd();
}


//# ---------- private ----------

// Implement the initialisation of the pointers
void	DH_OTDBlog::fillDataPointers() {
	LOG_TRACE_FLOW("DH_OTDBlog:fillDataPointers()");

	itsVersionNumber = getData<uint16>("VersionNumber");

	*itsVersionNumber = 0x0100;		// TODO define a constant WriteVersion
}


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

