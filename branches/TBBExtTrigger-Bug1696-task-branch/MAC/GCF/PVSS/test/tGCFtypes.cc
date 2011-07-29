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
#include <Common/hexdump.h>
#include <GCF/PVSS/GCF_PVTypes.h>

using namespace LOFAR;
using namespace GCF::PVSS;

int main(int argc, char* argv[]) {
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

	GCFPVDynArr		originalArr(testArr);
	GCFPVDynArr		indenticalArr(testArr);
	ASSERTSTR (originalArr == indenticalArr, "originalArr and indenticalArr are NOT identical");
	cout << "originalArr and indenticalArr are identical" << endl;
	cout << originalArr << endl;

	GCFPVDynArr		copiedArr(LPT_STRING);
	copiedArr.copy(originalArr);
	ASSERTSTR (originalArr == copiedArr, "originalArr and copiedArr are NOT identical");
	cout << "originalArr and copiedArr are identical" << endl;

	testArr.push_back(new GCFPVString("teun"));
	GCFPVDynArr		differentArr(testArr);
	ASSERTSTR (originalArr != differentArr, "originalArr and differentArr ARE identical");
	cout << "originalArr and differentArr are not identical" << endl;

	int	size = originalArr.getSize();
	char*	packbuffer = new char[size];
	originalArr.pack(packbuffer);
	cout << "packed originalArr is " << size << " bytes" << endl;
	hexdump(packbuffer, size);
	unsigned int		offset(0);
	GCFPValue*	unpackedObject = GCFPValue::unpackValue(packbuffer, &offset);
	cout << "type of unpackedObject = " << unpackedObject->getTypeName() << endl;
	cout << "unpacked originalArr is:" << *unpackedObject << endl;
	delete	unpackedObject;
	delete [] packbuffer;
	
	GCFPValueArray	emptyVector;
	GCFPVDynArr		emptyArr(emptyVector);
	GCFPVDynArr		emptyStringArr(LPT_STRING);
	cout << "Type of an empty array        = " << emptyArr.getType() << "(" << emptyArr.getTypeName() << ")" << endl;
	cout << "Type of an empty string array = " << emptyStringArr.getType() << "(" << emptyStringArr.getTypeName() << ")" << endl;
	cout << "Type of a filled string array = " << originalArr.getType() << "(" << originalArr.getTypeName() << ")" << endl;

	GCFPValueArray		twoDVector;
	GCFPValueArray		intVector;
	intVector.push_back(new GCFPVInteger(25));
	intVector.push_back(new GCFPVInteger(3125));

	twoDVector.push_back(&originalArr);
	twoDVector.push_back(new GCFPVDynArr(intVector));
	GCFPVDynArr			dyndynArr(twoDVector);
	cout << "Type of a 2D array            = " << dyndynArr.getType() << "(" << dyndynArr.getTypeName() << ")" << endl;
	cout << dyndynArr << endl;

	size = dyndynArr.getSize();
	packbuffer = new char[size];
	dyndynArr.pack(packbuffer);
	cout << "packed dyndynArr is " << size << " bytes" << endl;
	hexdump(packbuffer, size);
	offset = 0;
	unpackedObject = GCFPValue::unpackValue(packbuffer, &offset);
	cout << "type of unpackedObject = " << unpackedObject->getTypeName() << endl;
	cout << "unpacked dyndynArr is:" << *unpackedObject << endl;
	delete	unpackedObject;
	delete [] packbuffer;

	return (0);
}
