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
#include <GCF/PVSS/GCF_PVTypes.h>

using namespace LOFAR;
using namespace GCF::PVSS;

int main(int, char* argv[]) {
	INIT_LOGGER(argv[0]);

	// first try to create an object of every type
	unsigned char	buffer[] = "die_is_de_inhoud_van_een_blob.";
	unsigned char*	blob = &buffer[0];

	GCFPVBool		myBool1(true);
	GCFPVBlob		myBlob1(blob, sizeof(*blob), true);
	GCFPVChar		myChar1('A');
	GCFPVDouble		myDouble1(3.14);
	GCFPVInteger	myInt1(-34567);
	GCFPVString		myString1("Some test string");
	GCFPVUnsigned	myUnsigned1(76543);

	GCFPValueArray	testArr;
	testArr.push_back(new GCFPVString("aap"));
	testArr.push_back(new GCFPVString("noot"));
	testArr.push_back(new GCFPVString("mies"));
	testArr.push_back(new GCFPVString("wim"));
	testArr.push_back(new GCFPVString("zus"));

	GCFPVDynArr		originalArr(LPT_STRING, testArr);
	GCFPVDynArr		indenticalArr(LPT_STRING, testArr);
	ASSERTSTR (originalArr == indenticalArr, "originalArr and indenticalArr are NOT identical");
	cout << "originalArr and indenticalArr are identical" << endl;

	GCFPVDynArr		copiedArr(LPT_STRING);
	copiedArr.copy(originalArr);
	ASSERTSTR (originalArr == copiedArr, "originalArr and copiedArr are NOT identical");
	cout << "originalArr and copiedArr are identical" << endl;

	testArr.push_back(new GCFPVString("teun"));
	GCFPVDynArr		differentArr(LPT_STRING, testArr);
	ASSERTSTR (originalArr != differentArr, "originalArr and differentArr ARE identical");
	cout << "originalArr and differentArr are not identical" << endl;
	
	return (0);
}
