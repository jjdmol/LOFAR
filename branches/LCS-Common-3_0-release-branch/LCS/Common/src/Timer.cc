//# Timer.cc: Accurate timer
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>

#include <Common/Timer.h>


using namespace std;

namespace LOFAR {


double NSTimer::CPU_speed_in_MHz = NSTimer::get_CPU_speed_in_MHz();


double NSTimer::get_CPU_speed_in_MHz()
{
    // first a few sanity checks
    assert(sizeof(int) == 4);
    assert(sizeof(long long) == 8);

#if defined __linux__ && \
    (defined __i386__ || defined __x86_64__ || defined __ia64__ || defined __PPC__) && \
    (defined __GNUC__ || defined __INTEL_COMPILER || defined __PATHSCALE__ || defined __xlC__)
    ifstream infile("/proc/cpuinfo");
    char     buffer[256], *colon;

    while (infile.good()) {
	infile.getline(buffer, 256);

#if defined __PPC__
	if (strncmp("timebase", buffer, 8) == 0 && (colon = strchr(buffer, ':')) != 0)
	    return atof(colon + 2) / 1e6;
#else
	if (strncmp("cpu MHz", buffer, 7) == 0 && (colon = strchr(buffer, ':')) != 0)
	    return atof(colon + 2);
#endif
    }

    return 0.0;
#elif defined __blrts__ // BlueGene/L
    return 700.0;
#else
#warning unsupported architecture
    return 0.0;
#endif
}


double NSTimer::getElapsed() const
{
  double time = total_time / 1e6;
  if (CPU_speed_in_MHz > 0) {
    time /= CPU_speed_in_MHz;
  }
  return time;
}


void NSTimer::print_time(ostream &str, const char *which, double time) const
{
    static const char *units[] = { " ns", " us", " ms", "  s", " ks", 0 };
    const char	      **unit   = units;

    time = 1000.0 * time / CPU_speed_in_MHz;

    while (time >= 999.5 && unit[1] != 0) {
	time /= 1000.0;
	++ unit;
    }

    str << which << " = " << setprecision(3) << setw(4) << time << *unit;
}


ostream &NSTimer::print(ostream &str) const
{
    if (name == 0) {
      str << "timer: ";
    } else {
      str << left << setw(25) << name << ": " << right;
    }

    if (CPU_speed_in_MHz == 0)
	str << "could not determine CPU speed\n";
    else if (count > 0) {
	double total = static_cast<double>(total_time);

	print_time(str, "avg", total / static_cast<double>(count));
	print_time(str, ", total", total);
	str << ", count = " << setw(9) << count << '\n';
    }
    else
	str << "not used\n";

    return str;
}


}  // end namespace LOFAR
