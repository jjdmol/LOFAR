//#  tKVpair.cc: test KVpair class
//#
//#  Copyright (C) 2011
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
//#  $Id: tKVpair.cc 11044 2008-03-21 08:38:09Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Common/KVpair.h>

using namespace LOFAR;

int main (int, char*	argv[])
{
	INIT_LOGGER(argv[0]);

	cout << "\n--- Testing constructors ---" << endl;
	KVpair	KV0("stringValue", string("aap noot mies"));
	cout << "KV0: " << KV0 << endl;

	KVpair	KV1("charPtrValue", "wim zus jet");
	cout << "KV1: " << KV1 << endl;

	KVpair	KV2("booleanValue", true);
	cout << "KV2: " << KV2 << endl;

	KVpair	KV3("IntegerValue", 125000);
	cout << "KV3: " << KV3 << endl;

	double d = 57566757.000125;
	KVpair	KV4("DoubleValue", d);
	cout << "KV4: " << KV4 << endl;

	float	f = 57566757.000125;
	KVpair	KV5("FloatValue", f);
	cout << "KV5: " << KV5 << endl;

	time_t	t = time(0);
	KVpair	KV6("TimeTValue", t);
	cout << "KV6: " << KV6 << endl;

	vector<int>		vi;
	vi.push_back( 5);
	vi.push_back(78);
	vi.push_back(32);
	vi.push_back(39);
	vi.push_back(5003);
	KVpair	KV7("IntVectorValue", vi);
	cout << "KV7: " << KV7 << endl;

	cout << "\n--- Testing constructors with timestamp ---" << endl;
	KVpair	KVT0("stringValue", string("aap noot mies"), true);
	cout << ">>>KVT0: " << KVT0 << "<<<" << endl;

	KVpair	KVT1("charPtrValue", "wim zus jet", true);
	cout << ">>>KVT1: " << KVT1 << "<<<" << endl;

	KVpair	KVT2("booleanValue", true, true);
	cout << ">>>KVT2: " << KVT2 << "<<<" << endl;

	KVpair	KVT3("IntegerValue", 125000, true);
	cout << ">>>KVT3: " << KVT3 << "<<<" << endl;

	KVpair	KVT4("DoubleValue", d, true);
	cout << ">>>KVT4: " << KVT4 << "<<<" << endl;

	KVpair	KVT5("FloatValue", f, true);
	cout << ">>>KVT5: " << KVT5 << "<<<" << endl;

	KVpair	KVT6("TimeTValue", t, true);
	cout << ">>>KVT6: " << KVT6 << "<<<" << endl;

	KVpair	KVT7("IntVectorValue", vi, true);
	cout << ">>>KVT7: " << KVT7 << "<<<" << endl;

	cout << "\n--- Testing copy and assignment operators ---" << endl;
	KVpair	KVT1D(KVT1);
	ASSERTSTR(KVT1 == KVT1D, "Assignment constructor failed");

	KVpair	KVT2D = KVT2;
	ASSERTSTR(KVT2 == KVT2D, "Copy operator failed");

	cout << "\nPassed all tests!" << endl;

	return (0);
}
