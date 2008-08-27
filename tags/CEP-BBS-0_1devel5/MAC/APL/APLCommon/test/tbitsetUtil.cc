//#  tbitsetUtil.cc
//#
//#  Copyright (C) 2008
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
#include <APL/APLCommon/bitsetUtil.tcc>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	bitset<256>		bs1;
	bs1.reset();
	cout << bitset2CompactedArrayString(bs1) << endl;

	for (int i = 5; i < 20; i++) {
		bs1.set(i);
	}
	cout << bitset2CompactedArrayString(bs1) << endl;

	for (int i =25; i < 29; i++) {
		bs1.set(i);
	}
	cout << bitset2CompactedArrayString(bs1) << endl;

	bs1.reset(7);
	bs1.reset(8);
	bs1.reset(10);
	bs1.reset(12);
	bs1.reset(14);
	cout << bitset2CompactedArrayString(bs1) << endl;

	bs1.set();
	cout << bitset2CompactedArrayString(bs1) << endl;

	// does it also work with other size bitsets?
	bitset<32>	bs2;
	bs2.reset();
	bs2.set(5);
	cout << bitset2CompactedArrayString(bs2) << endl;
	
	return (0);
}

