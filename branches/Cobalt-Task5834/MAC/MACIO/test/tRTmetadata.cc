//#  tRTmetadata.cc
//#
//#  Copyright (C) 2014
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
#include <MACIO/RTmetadata.h>
#include <MACIO/KVT_Protocol.ph>

#include <boost/date_time/gregorian/gregorian.hpp>

using namespace LOFAR;
using namespace LOFAR::MACIO;

void test()
{
	RTmetadata myLogger(3125, "ObservationControl", "localhost");

	KVpair pair;
	pair.first  = "pietjepuk";
	pair.second = "0521";
	myLogger.log(pair);
	pair.second = "521";
	myLogger.log(pair);
	pair.first  = "ExampleDP_Arg1";
	pair.second = "521.0";
	myLogger.log(pair);

	myLogger.start(); // start thread

	vector<KVpair> pairs;
	pair.first  = "Eucalypta";
	pair.second = "Witch";
	pairs.push_back(pair);
	pair.first  = "Oehoeboeroe";
	pair.second = "Owl";
	pairs.push_back(pair);
	pair.first  = "Reintje";
	pair.second = "Fox";
	pairs.push_back(pair);
	myLogger.log(pairs);

	usleep(600000); // give started thread some time before cancelling in destructor
}

int main (int	/*argc*/, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	test();

	return 0;
}
