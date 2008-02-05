//#  tAPLUtilities.cc
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/APLCommon/APLUtilities.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

#define	DO_COMPACT_TEST(x) \
	cout << (x) << " ==> " << APLUtilities::compactedArrayString(x) << endl;

#define	DO_EXPAND_TEST(x) \
	cout << (x) << " ==> " << APLUtilities::expandedArrayString(x) << endl;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	DO_COMPACT_TEST("[ a,b,c ]");
	DO_COMPACT_TEST("[ aa01,aa06 ]");
	DO_COMPACT_TEST("[ aa01,aa02 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03,aa04 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03,aa05 ]");
	DO_COMPACT_TEST("[ aa01, aa02 ,aa03,aa05,aa06 ]");
	DO_COMPACT_TEST("[ aa01, bb02 ,aa03,aa05,aa06 ]");
	DO_COMPACT_TEST("[ aa01, aa02 ,aa0003,aa05,aa06 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03,aa05,aa06,aa7 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03,aa05,a06,aa7 ]");
	DO_COMPACT_TEST("[ aa01,aa02,aa03,aa05,aaa06,aa7 ]");

	DO_COMPACT_TEST("[22,23,24,25,30,31,33,35]");
	DO_COMPACT_TEST("[22,23,24,25,30,31,33,35,36]");
	DO_COMPACT_TEST("[22,23,24,25,30,31,33,35,36,37]");
	DO_COMPACT_TEST("[22,23,24,25,30,31,33,35,36,37,37]");
	DO_COMPACT_TEST("[22,23,24,25,30,31,33,35,36,37,40]");
	DO_COMPACT_TEST("[22,23,23,25,30,31,30]");
	DO_COMPACT_TEST("[22,23,23,25,30,31,30,31]");
	DO_COMPACT_TEST("[22,23,23,25,30,31,30,31,31]");
	DO_COMPACT_TEST("[22,23,23,23,23,23,25,30,31,30,31,31]");
	
	DO_EXPAND_TEST("[ a,b,c ]");
	DO_EXPAND_TEST("[ aa01,aa06 ]");
	DO_EXPAND_TEST("[ aa01,aa02 ]");
	DO_EXPAND_TEST("[ aa01..aa02 ]");
	DO_EXPAND_TEST("[ aa01..aa03 ]");
	DO_EXPAND_TEST("[ aa01..aa03,aa04 ]");
	DO_EXPAND_TEST("[ aa01..aa03,aa05 ]");
	DO_EXPAND_TEST("[ aa02,aa05..aa07]");
	DO_EXPAND_TEST("[ aa02,aa05..aa04]");
	DO_EXPAND_TEST("[ aa02,aa05..aa05]");

	DO_EXPAND_TEST("[ aa01..ba03,aa05 ]");
	DO_EXPAND_TEST("[ aa01..aa03,ba05 ]");
	DO_EXPAND_TEST("[ aa01,ab03,aa05 ]");
	DO_EXPAND_TEST("[ aa01,ab03..aa05 ]");
	DO_EXPAND_TEST("[ aa01,aa03..ab05 ]");
	DO_EXPAND_TEST("[ aa01,bb03..bb05 ]");

	DO_EXPAND_TEST("[0..191]");
	DO_EXPAND_TEST("[0,1,2,6,3,4,10..19,16]");

	return (0);
}

