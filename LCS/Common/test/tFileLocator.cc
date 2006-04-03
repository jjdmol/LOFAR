//#  tFileLocator.cc: Program to test the hexdump functions
//#
//#  Copyright (C) 2006
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

#include <Common/LofarLogger.h>
#include <Common/FileLocator.h>

using namespace std;
using namespace LOFAR;


int main (int, char *argv[]) {

	// Read in the log-environment configuration
	INIT_LOGGER(argv[0]);

	// Show operator were are on the air
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_INFO ("Creating fileLocator with path: /usr");
	FileLocator		Locator1("/usr");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());

	LOG_INFO ("Adding '/usr/bin:./' at end of chain");
	Locator1.addPathAtBack("/usr/bin:./:../");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	
	LOG_INFO ("Adding '/usr/local:/usr/local/bin/' at begin of chain");
	Locator1.addPathAtFront("/usr/local:/usr/local/bin/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	
	bool	path1 = Locator1.hasPath("/usr/local/bin/");
	bool	path2 = Locator1.hasPath("/usr/nonlocal/bin/");
	LOG_INFO (formatString("Path /usr/local/bin/ is %sin the chain", path1 ? "" : "NOT "));
	LOG_INFO (formatString("Path /usr/nonlocal/bin/ is %sin the chain", path2 ? "" : "NOT "));
	path1 = Locator1.hasPath("/usr/local/bin");
	path2 = Locator1.hasPath("/usr/nonlocal/bin");
	LOG_INFO (formatString("Path /usr/local/bin is %sin the chain", path1 ? "" : "NOT "));
	LOG_INFO (formatString("Path /usr/nonlocal/bin is %sin the chain", path2 ? "" : "NOT "));

	LOG_INFO ("removing path '/usr/'");
	Locator1.removePath ("/usr/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());

	LOG_INFO ("Adding '/' at end of chain");
	Locator1.addPathAtBack("/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	path2 = Locator1.hasPath("/");
	LOG_INFO (formatString("Path / is %sin the chain", path2 ? "" : "NOT "));
	LOG_INFO ("Searching file 'CLUSTER_INFO'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("CLUSTER_INFO"));

	LOG_INFO ("Searching file 'autoscan'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("autoscan"));

	LOG_INFO ("Searching file 'doesnotexist'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("doesnotexist"));

	LOG_INFO ("Searching file '../namewithslash'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("../namewithslash"));

	LOG_INFO ("Searching myself");
	LOG_INFO_STR ("fullname = " << Locator1.locate(basename(argv[0])));

	LOG_INFO("Normal termination of program");
	return (0);
}

