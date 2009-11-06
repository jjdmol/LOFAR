//  tGCFtypes.cc: Test program to test the majority of the GCF types
//
//  Copyright (C) 2007
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <GCF/GCF_PVTypes.h>

using namespace LOFAR;

int main(int argc, char* argv[]) {
	INIT_LOGGER(argv[0]);

	// first try to create an object of every type
	unsigned char	buffer[] = "die_is_de_inhoud_van_een_blob.";
	unsigned char*	blob = &buffer[0];

	GCFPVBool		myBool1(true);
	GCFPVBlob		myBlob1(blob, sizeof(*blob), true);
	GCFPVChar		myChar1('A');
	GCFPVDouble		myDouble1(3.14);
//	GCFPVDynArr		myDynArr("");
	GCFPVInteger	myInt1(-34567);
	GCFPVString		myString1("Some test string");
	GCFPVUnsigned	myUnsigned1(76543);


	return (0);
}
