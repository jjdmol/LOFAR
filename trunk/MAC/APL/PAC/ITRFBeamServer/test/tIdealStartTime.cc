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
//#  $Id: tOverlap.cc 14866 2010-01-23 00:07:02Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

using namespace LOFAR;

void idealStartTime (int now, int t1, int d1, int t2, int d2, int p2, int expAnswer)
{
	int	t1start = t1-d1;				// ideal starttime
	if (t1start < now) 					// not before now ofcourse
		t1start = now;
	int nearestt2 = (t1start<=t2 ? t2 : t2+((t1-t2)/p2)*p2);
	if (t1start > nearestt2 && t1start < nearestt2+d2)	// not during heartbeat period
		t1start = nearestt2;

	ASSERTSTR(t1start == expAnswer, 
		formatString("now=%d, t1=%d, d2=%d, t2=%d, nearestt2=%d, d2=%d, p2=%d gives %d iso %d", 
					now, t1, d1, t2, nearestt2, d2, p2, t1start, expAnswer));

}

int main(int, char*	argv[]) 
{
	INIT_LOGGER(argv[0]);

	//  n    t2 d2        t2`
	// -o----+--->+-------+------
	//  w
	//        d1 t1
	// ------+<==+--------

	// enough time before next heartbeat starts, test overlap code
	//           now, t1, d1, t2, d2, p2, answer
	idealStartTime ( 90, 100, 4, 100, 5, 13,  96);		// fits before, t2 will also be active
	idealStartTime ( 90, 102, 4, 100, 5, 13,  98);		// fits before, t2 will be skipped
	idealStartTime ( 90, 104, 4, 100, 5, 13, 100);		// start at same time
	idealStartTime ( 90, 106, 4, 100, 5, 13, 100);		// start t1 during heartbeat
	idealStartTime ( 90, 108, 4, 100, 5, 13, 100);		// start t1 during heartbeat
	idealStartTime ( 90, 110, 4, 100, 5, 13, 106);		// start just after heartbeat
	
	// just before next heartbeat starts, test correction for 'now'
	idealStartTime ( 99,  98, 4, 100, 5, 13,  99);		// start immediately, t2 will be skipped
	idealStartTime ( 99, 100, 4, 100, 5, 13,  99);		// start immediately, t2 will be skipped
	idealStartTime ( 99, 102, 4, 100, 5, 13,  99);		// start immediately, t2 will be skipped
	idealStartTime ( 99, 104, 4, 100, 5, 13, 100);		// start at same time
	idealStartTime ( 99, 106, 4, 100, 5, 13, 100);		// start t1 during heartbeat
	idealStartTime ( 99, 108, 4, 100, 5, 13, 100);		// start t1 during heartbeat
	idealStartTime ( 99, 110, 4, 100, 5, 13, 106);		// start just after heartbeat

	// just before next heartbeat starts (t2 is from 490-495, 503-508), test periodic code
	idealStartTime ( 99, 504, 4, 100, 5, 13, 500);		// fits before, t2 will be skipped
	idealStartTime ( 99, 506, 4, 100, 5, 13, 502);		// fits before, t2 will be skipped
	idealStartTime ( 99, 508, 4, 100, 5, 13, 503);		// start t1 during heartbeat
	idealStartTime ( 99, 510, 4, 100, 5, 13, 503);		// start t1 during heartbeat
	idealStartTime ( 99, 512, 4, 100, 5, 13, 508);		// start just after heartbeat
	idealStartTime ( 99, 514, 4, 100, 5, 13, 510);		// start just after heartbeat

	cout << "Program finished succesfully" << endl;
	return (0);
}

