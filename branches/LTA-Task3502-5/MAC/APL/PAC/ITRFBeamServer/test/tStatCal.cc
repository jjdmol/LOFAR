//#  Class.cc: one_line_description
//#
//#  Copyright (C) 2010
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
//#  $Id: tJ2000Converter.cc 14866 2010-01-23 00:07:02Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>

#include <blitz/array.h>
#include <ITRFBeamServer/StatCal.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;

int main(int, char*	argv[]) 
{
	INIT_LOGGER(argv[0]);

	StatCal		theCalTable(1,12);
	blitz::Array<std::complex<double>,3> theData = theCalTable();

	cout << "RCU 5: X,Y for subband 10..15: " << endl;
	for (int sub = 10; sub < 16; sub++) {
		cout << "sub " << sub;
		for (int pol = 0; pol < N_POL; pol++) {
			cout << theData(5,pol,sub) << " ";
		}
		cout << endl;
	}
	cout << endl;

	cout << "Subband 10: X,Y for RCU 5..10: " << endl;
	for (int rcu = 5; rcu <= 10; rcu++) {
		cout << "rcu " << rcu;
		for (int pol = 0; pol < N_POL; pol++) {
			cout << theData(rcu,pol,10) << " ";
		}
		cout << endl;
	}
	cout << endl;

}
