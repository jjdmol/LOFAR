//#  tClone.cc
//#
//#  Copyright (C) 2009
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
#include <Common/hexdump.h>
#include "Clone_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::MACIO;

int main (int	/*argc*/, char* argv[]) 
{
	INIT_VAR_LOGGER(argv[0], argv[0]);

	LOG_INFO("First cloning a GCFEvent");
	GCFEvent	theOrgGCF;
	LOG_INFO_STR(theOrgGCF);
	GCFEvent*	theClonedGCF = theOrgGCF.clone();
	LOG_INFO_STR(*theClonedGCF);
	LOG_INFO_STR("sizeof(GCFEvent)=" << sizeof(GCFEvent));
	ASSERTSTR(theOrgGCF.signal == theClonedGCF->signal, "signal differs: " << theOrgGCF.signal << " <> " << theClonedGCF->signal);
	ASSERTSTR(theOrgGCF.length == theClonedGCF->length, "length differs: " << theOrgGCF.length << " <> " << theClonedGCF->length);
	ASSERTSTR(theOrgGCF._buffer == theClonedGCF->_buffer, "_buffer differs: " << theOrgGCF._buffer << " <> " << theClonedGCF->_buffer);
//	ASSERTSTR(memcmp(&theOrgGCF, theClonedGCF, sizeof(GCFEvent)) == 0, "Clone of GCFEvent failed");

	LOG_INFO("Cloning a simple Test event");
	CloneSimpleEvent		theOrgSimple;
	theOrgSimple.uInt	= 67483;
	theOrgSimple.Int	= -45896;
	theOrgSimple.Double	= 5447.67483;
	theOrgSimple.Float	= 67483.5447;
	theOrgSimple.Secret	= "Ssst, don't tell anyone!";		// this is a 'non-printable' field in the definition
	theOrgSimple.String	= "At least also some letters. All those digits!";
	LOG_INFO_STR(theOrgSimple);

	CloneSimpleEvent*	theClonedSimple = theOrgSimple.clone();
	LOG_INFO_STR(*theClonedSimple);

	ASSERTSTR(theOrgSimple.uInt == theClonedSimple->uInt, "uInt differs: " << theOrgSimple.uInt << " <> " << theClonedSimple->uInt);
	ASSERTSTR(theOrgSimple.Int == theClonedSimple->Int, "Int differs: " << theOrgSimple.Int << " <> " << theClonedSimple->Int);
	ASSERTSTR(theOrgSimple.Double == theClonedSimple->Double, "Double differs: " << theOrgSimple.Double << " <> " << theClonedSimple->Double);
	ASSERTSTR(theOrgSimple.Float == theClonedSimple->Float, "Float differs: " << theOrgSimple.Float << " <> " << theClonedSimple->Float);
	ASSERTSTR(theOrgSimple.String == theClonedSimple->String, "String differs: " << theOrgSimple.String << " <> " << theClonedSimple->String);

	LOG_INFO("Cloning an event with a stringVector");
	CloneVectorEvent	theOrgVector;
	theOrgVector.SV.theVector.push_back("aap");
	theOrgVector.SV.theVector.push_back("noot");
	theOrgVector.SV.theVector.push_back("mies");
	theOrgVector.SV.theVector.push_back("wim");
	theOrgVector.SV.theVector.push_back("zus");
	theOrgVector.SV.theVector.push_back("jet");
	LOG_INFO_STR(theOrgVector);

	CloneVectorEvent*	theClonedVector = theOrgVector.clone();
	LOG_INFO_STR(*theClonedVector);

	return (0);
}

