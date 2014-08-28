//#  tTimestamp.cc: test Timestamp class
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
//#  $Id: tTimestamp.cc 11044 2008-03-21 08:38:09Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>
#include <Common/hexdump.h>
#include <APL/RTCCommon/Timestamp.h>

using namespace LOFAR;
using namespace RTC;

int main (int, char*	argv[])
{
	INIT_LOGGER(argv[0]);

	cout << "\n--- Testing constructors ---" << endl;
	Timestamp		TS1;
	cout << "TS1: " << TS1 << endl;

	struct timeval	tv;
	tv.tv_sec = 57566757;
	tv.tv_usec= 125;
	Timestamp		TS2(tv);
	cout << "TS2: " << TS2 << endl;

	Timestamp	TS3(57566757, 125);
	cout << "TS3: " << TS3 << endl;

	Timestamp	TS4(57566757.000125);
	cout << "TS4: " << TS4 << endl;

	Timestamp	TS5 = TS3;
	cout << "TS5: " << TS5 << endl;

	cout << "\n--- Setters and getters ---" << endl;
	Timestamp		TS6;
	TS6.setNow();
	cout << TS6 << endl;
	TS6.setNow(100.01);
	cout << TS6 << endl;
	TS6.setNow(-100.01);
	cout << TS6 << endl;

	struct timeval tv2;
	TS5.get(&tv2);
	cout << formatString("%d.%06d", tv2.tv_sec, tv2.tv_usec) << endl;

	double	D1 = TS5;
	cout << formatString("%20.6f", D1) << endl;
	TS1.set(D1);
	cout << TS1 << endl;

	cout << "\n--- Testing operators ---" << endl;
	Timestamp		NT1(123456789, 546372819);
	Timestamp		NT2 = NT1;

	if ( (NT1 <  NT2)) cout << "operator < on equal timestamps failed" << endl;
	if ( (NT1 >  NT2)) cout << "operator > on equal timestamps failed" << endl;
	if (!(NT1 <= NT2)) cout << "operator <= on equal timestamps failed" << endl;
	if (!(NT1 >= NT2)) cout <<  "operator >= on equal timestamps failed" << endl;
	if (!(NT1 == NT2)) cout <<  "operator == on equal timestamps failed" << endl;
	if ( (NT1 != NT2)) cout << "operator != on equal timestamps failed" << endl;

	// test on fracton of second.
	NT2 += 0.003;
	cout << formatString("T1.sec=%d, T1.usec=%06d", NT1.sec(), NT1.usec()) << endl;
	cout << formatString("T2.sec=%d, T2.usec=%06d", NT2.sec(), NT2.usec()) << endl;
	if (!(NT1 <  NT2)) cout << "operator < on T1<T2 timestamps failed" << endl;
	if ( (NT1 >  NT2)) cout << "operator > on T1<T2 timestamps failed" << endl;
	if (!(NT1 <= NT2)) cout << "operator <= on T1<T2 timestamps failed" << endl;
	if ( (NT1 >= NT2)) cout <<  "operator >= on T1<T2 timestamps failed" << endl;
	if ( (NT1 == NT2)) cout <<  "operator == on T1<T2 timestamps failed" << endl;
	if (!(NT1 != NT2)) cout << "operator != on T1<T2 timestamps failed" << endl;

	NT2 = NT1 - 0.003;
	cout << formatString("T1.sec=%d, T1.usec=%06d", NT1.sec(), NT1.usec()) << endl;
	cout << formatString("T2.sec=%d, T2.usec=%06d", NT2.sec(), NT2.usec()) << endl;
	if ( (NT1 <  NT2)) cout << "operator < on T1>T2 timestamps failed" << endl;
	if (!(NT1 >  NT2)) cout << "operator > on T1>T2 timestamps failed" << endl;
	if ( (NT1 <= NT2)) cout << "operator <= on T1>T2 timestamps failed" << endl;
	if (!(NT1 >= NT2)) cout <<  "operator >= on T1>T2 timestamps failed" << endl;
	if ( (NT1 == NT2)) cout <<  "operator == on T1>T2 timestamps failed" << endl;
	if (!(NT1 != NT2)) cout << "operator != on T1>T2 timestamps failed" << endl;

	// test on whole seconds too.
	NT2 = NT1 + 1.8;
	cout << formatString("T1.sec=%d, T1.usec=%06d", NT1.sec(), NT1.usec()) << endl;
	cout << formatString("T2.sec=%d, T2.usec=%06d", NT2.sec(), NT2.usec()) << endl;
	if (!(NT1 <  NT2)) cout << "operator < on T1<T2 timestamps failed" << endl;
	if ( (NT1 >  NT2)) cout << "operator > on T1<T2 timestamps failed" << endl;
	if (!(NT1 <= NT2)) cout << "operator <= on T1<T2 timestamps failed" << endl;
	if ( (NT1 >= NT2)) cout <<  "operator >= on T1<T2 timestamps failed" << endl;
	if ( (NT1 == NT2)) cout <<  "operator == on T1<T2 timestamps failed" << endl;
	if (!(NT1 != NT2)) cout << "operator != on T1<T2 timestamps failed" << endl;

	NT2 -= 3.5;
	cout << formatString("T1.sec=%d, T1.usec=%06d", NT1.sec(), NT1.usec()) << endl;
	cout << formatString("T2.sec=%d, T2.usec=%06d", NT2.sec(), NT2.usec()) << endl;
	if ( (NT1 <  NT2)) cout << "operator < on T1>T2 timestamps failed" << endl;
	if (!(NT1 >  NT2)) cout << "operator > on T1>T2 timestamps failed" << endl;
	if ( (NT1 <= NT2)) cout << "operator <= on T1>T2 timestamps failed" << endl;
	if (!(NT1 >= NT2)) cout <<  "operator >= on T1>T2 timestamps failed" << endl;
	if ( (NT1 == NT2)) cout <<  "operator == on T1>T2 timestamps failed" << endl;
	if (!(NT1 != NT2)) cout << "operator != on T1>T2 timestamps failed" << endl;

	Timestamp		maxNT = Timestamp::maxTime();
	cout << maxNT << endl;

	return (0);
}
