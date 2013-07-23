//# tFileLocator.cc: Program to test the hexdump functions
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <cstdlib>                // for setenv()

#define CHECK(cond)						\
	do {							\
		if (!(cond)) {					\
			errors++;				\
			LOG_ERROR("Check '" #cond "' failed.");	\
		}						\
	} while(0)

using namespace LOFAR;

int errors;

int main (int, char *argv[]) {
 
	using LOFAR::basename;

	// Read in the log-environment configuration
	INIT_LOGGER(argv[0]);

	// Show operator were are on the air
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_INFO ("Creating fileLocator with path: /usr");
	FileLocator		Locator1("/usr");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	CHECK(Locator1.getPath() == "/usr");

	LOG_INFO ("Adding '/usr/bin:./' at end of chain");
	Locator1.addPathAtBack("/usr/bin:./");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	CHECK(Locator1.getPath() == "/usr:/usr/bin:.");

	LOG_INFO ("Adding '/usr/local:/usr/local/bin/' at begin of chain");
	Locator1.addPathAtFront("/usr/local:/usr/local/bin/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	CHECK(Locator1.getPath() == "/usr/local:/usr/local/bin:/usr:/usr/bin:.");

	bool	path1 = Locator1.hasPath("/usr/local/bin/");
	bool	path2 = Locator1.hasPath("/usr/nonlocal/bin/");
	LOG_INFO (formatString("Path /usr/local/bin/ is %sin the chain", path1 ? "" : "NOT "));
	LOG_INFO (formatString("Path /usr/nonlocal/bin/ is %sin the chain", path2 ? "" : "NOT "));
	CHECK(path1);
	CHECK(!path2);

	path1 = Locator1.hasPath("/usr/local/bin");
	path2 = Locator1.hasPath("/usr/nonlocal/bin");
	LOG_INFO (formatString("Path /usr/local/bin is %sin the chain", path1 ? "" : "NOT "));
	LOG_INFO (formatString("Path /usr/nonlocal/bin is %sin the chain", path2 ? "" : "NOT "));
	CHECK(path1);
	CHECK(!path2);

	LOG_INFO ("removing path '/usr/'");
	Locator1.removePath ("/usr/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	CHECK(Locator1.getPath() == "/usr/local:/usr/local/bin:/usr/bin:.");
	LOG_INFO ("Adding '/' at end of chain");
	Locator1.addPathAtBack("/");
	LOG_INFO_STR ("registered path = " << Locator1.getPath());
	CHECK(Locator1.getPath() == "/usr/local:/usr/local/bin:/usr/bin:.:/");

	path2 = Locator1.hasPath("/");
	LOG_INFO (formatString("Path / is %sin the chain", path2 ? "" : "NOT "));
	CHECK(path2);

	LOG_INFO ("Searching file 'CLUSTER_INFO'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("CLUSTER_INFO"));
	CHECK(Locator1.locate("CLUSTER_INFO") == "");

	LOG_INFO ("Searching file 'test'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("wc"));
	CHECK(Locator1.locate("wc") == "/usr/bin/wc");

	LOG_INFO ("Searching file 'doesnotexist'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("doesnotexist"));
	CHECK(Locator1.locate("doesnotexist") == "");

	LOG_INFO ("Searching file '../namewithslash'");
	LOG_INFO_STR ("fullname = " << Locator1.locate("../namewithslash"));
	CHECK(Locator1.locate("../namewithslash") == "");

	LOG_INFO ("Searching myself");
	LOG_INFO_STR ("fullname = " << Locator1.locate(basename(argv[0])));
	CHECK(Locator1.locate(basename(argv[0])) != "");

#if RESOLVE_INPUT_NOT_PRIVATE
	LOG_INFO_STR("'$iserniet': " <<  Locator1.resolveInput("$iserniet"));
	LOG_INFO_STR("'$lofarroot': " <<  Locator1.resolveInput("$lofarroot"));
	LOG_INFO_STR("'$lofarroot/bin': " <<  
						Locator1.resolveInput("$lofarroot/bin"));
	LOG_INFO_STR("'/sbin:$lofarroot/bin': " <<  
						Locator1.resolveInput("/sbin:$lofarroot/bin"));
	LOG_INFO_STR("'/sbin:$lofarroot/bin:/usr/sbin': " <<  
						Locator1.resolveInput("/sbin:$lofarroot/bin:/usr/sbin"));
#endif	

	LOG_INFO ("FOR THE NEXT TESTS THE ENVVAR $lofarroot IS SET TO /opt/lofar");
	setenv("lofarroot", "/opt/lofar", 1);

	LOG_INFO ("Creating default fileLocator");
	FileLocator		Locator2;
	LOG_INFO_STR ("registered path = " << Locator2.getPath());
	CHECK(Locator2.getPath() == string(BASE_SEARCH_DIR) + ":" +
		dirname(getExecutablePath()) + ":" +
		dirname(dirname(getExecutablePath())));

	path1 = Locator2.hasPath("$lofarroot");
	path2 = Locator2.hasPath("/opt/lofar/");
	LOG_INFO (formatString("Path $lofarroot is %sin the chain", path1 ? "" : "NOT "));
	LOG_INFO (formatString("Path /opt/lofar/ is %sin the chain", path2 ? "" : "NOT "));
	CHECK(path1);
	CHECK(path2);

	path1 = Locator2.hasPath("$unexisting_envvar");
	LOG_INFO (formatString("Path $unexisting_envvar is %sin the chain", path1 ? "" : "NOT "));
	CHECK(!path1);

	LOG_INFO("Setting subdir to 'foo'");
	Locator2.setSubdir("foo");
	LOG_INFO_STR ("registered path = " << Locator2.getPath());
	LOG_INFO_STR ("registered subdir = " << Locator2.getSubdir());
	CHECK(Locator2.getPath() == string(BASE_SEARCH_DIR) + ":" +
		dirname(getExecutablePath()) + ":" + 
		dirname(dirname(getExecutablePath())));
	CHECK(Locator2.getSubdir() == "foo");

	path1 = Locator2.hasPath("/opt/lofar/foo");
	LOG_INFO (formatString("Path /opt/lofar/foo is %sin the chain", path1 ? "" : "NOT "));
	CHECK(path1);

	LOG_INFO ("Searching file 'ServiceBroker.conf'");
	LOG_INFO_STR ("fullname = " << Locator2.locate("ServiceBroker.conf"));
	CHECK(Locator2.locate("ServiceBroker.conf") == "");

	LOG_INFO ("Testing ConfigLocator");
	ConfigLocator	aCL;
	LOG_INFO_STR ("registered path = " << aCL.getPath());
	LOG_INFO_STR ("registered subdir = " << aCL.getSubdir());
	CHECK(aCL.getPath() == string(BASE_SEARCH_DIR) + ":" +
		dirname(getExecutablePath()) + ":" + 
		dirname(dirname(getExecutablePath())));
	CHECK(aCL.getSubdir() == "etc");


	if(errors) {
		LOG_FATAL_STR("**** " << errors << " error" << 
			  (errors > 1 ? "s" : "") << " detected.");
        } else {
		LOG_INFO("Normal termination of program");
        }
	return (errors ? 1 : 0);
}

