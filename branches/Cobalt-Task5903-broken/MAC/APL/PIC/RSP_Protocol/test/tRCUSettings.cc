//#  Class.cc: one_line_description
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/RCUSettings.h>

namespace LOFAR {
  namespace RSP_Protocol {

void logMode(RCUSettings&	RS)
{
	cout << formatString("mode %d: Nyquist:%d, LBAfilter:%s, LBLinput:%s, LBHinput:%s, HBAinput:%s\n", 
					RS()(0).getMode(), 
					RS()(0).getNyquistZone(), 
					RS()(0).LBAfilter() ? "ON " : "OFF",
					RS()(0).LBLinput()  ? "ON " : "OFF",
					RS()(0).LBHinput()  ? "ON " : "OFF",
					RS()(0).HBAinput()  ? "ON " : "OFF");
}

  } // namespace RSP_Protocol
} // namespace LOFAR

using namespace LOFAR;
using namespace RSP_Protocol;

// main
int main (int, char*	argv[])
{
	INIT_LOGGER(argv[0]);

	RCUSettings		RS;
	RS().resize(1);

	RS()(0).setMode((RCUSettings::Control::RCUMode)0); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)1); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)2); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)3); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)4); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)5); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)6); logMode(RS);
	RS()(0).setMode((RCUSettings::Control::RCUMode)7); logMode(RS);

	return (0);
}



