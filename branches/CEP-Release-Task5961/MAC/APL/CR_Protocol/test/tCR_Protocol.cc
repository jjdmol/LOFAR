//#  Class.cc: one_line_description
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <APL/CR_Protocol/CR_Protocol.ph>

using namespace LOFAR;
using namespace RTC;
using namespace CR_Protocol;


// main
int main (int argc, char* argv[])
{
	CRstopVector	CV;
	cout << CV << endl;

	CRstopRequest	CR1("[CS001,CS002..CS006]", "[0..191]", NsTimestamp(676767.2223));
	cout << CR1 << endl;

	CV.requests.push_back(CR1);
	CV.requests.push_back(CRstopRequest("[]", "[]", NsTimestamp(98867356.0)));
	cout << CV << endl;

	char	buffer[10240];
	CR1.pack(buffer);
	CRstopRequest	C1copy;
	C1copy.unpack(buffer);
	cout << C1copy << endl;

	CV.pack(buffer);
	CRstopVector	V1copy;
	V1copy.unpack(buffer);
	cout << V1copy << endl;

	return (0);
}
