//  tBBS3.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>

#include <BBS3/BlackBoardDemo.h>
#include <Common/KeyParser.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace LOFAR;
using namespace std;

int main (int argc, const char** argv)
{
  try {
    // To try out different (serial) experiments without the CEP
    // framework, use following two statements:
    INIT_LOGGER("tBBS3.log_prop");

    BlackBoardDemo simulator;
    
    simulator.setarg (argc, argv);

    // Get input script.
    string name = "tBBS3.in";
    if (argc > 1) {
      name = argv[1];
    }
    int nriter = 1;
    if (argc > 2) {
      istringstream istr(argv[2]);
      istr >> nriter;
    }
    string usernm = "test";
    if (argc > 3) {
      usernm = argv[3];
    }
    // Read the input script until eof.
    // Remove // comments.
    // Combine it into a single key=value command.
    ifstream ifstr(name.c_str());
    string keyv;
    string str;
    while (getline(ifstr, str)) {
      if (str.size() > 0) {
	// Remove possible comments.
	string::size_type pos = str.find("//");
	if (pos == string::npos) {
	  keyv += str;
	} else if (pos > 0) {
	  keyv += str.substr(0,pos);
	}
      }
    }
    // Parse the command.
    KeyValueMap params = KeyParser::parse (keyv);

    KeyValueMap cmap(params["CTRLparams"].getValueMap()); 
    int nrStrategies = cmap.getInt("nrStrategies", 0);

    // Loop over all strategies
    for (int i=1; i<=nrStrategies; i++)
    {
      char nrStr[32];
      sprintf(nrStr, "%i", i);
      string name = "SC" + string(nrStr) + "params";
      // Add the nriter if not defined.   
      KeyValueMap smap(cmap[name].getValueMap());
      if (! smap.isDefined("nrIterations")) {
	smap["nrIterations"] = nriter;
	cmap[name] = smap;
	params["CTRLparams"] = cmap;
      }
      // Add the dbname if not defined.
      KeyValueMap msdbmap(smap["MSDBparams"].getValueMap());
      if (! msdbmap.isDefined("DBName")) {
	msdbmap["DBName"] = usernm;
	smap["MSDBparams"] = msdbmap;
	cmap[name] = smap; 
	params["CTRLparams"] = cmap;     
      }
      if (! params.isDefined("BBDBname")) {
	params["BBDBname"] = usernm;
      }
    }
    cout << params << endl;

    int nrRuns = params.getInt("nrRuns", 1);

    simulator.baseDefine(params);
    simulator.baseRun(nrRuns);
    simulator.baseQuit();

  }
  catch (LOFAR::Exception& e)
  {
    cout << "Lofar exception: " << e.what() << endl;
  }
  catch (std::exception& e)
  {
    cout << "Standard exception: " << e.what() << endl;
  }
  catch (...) {
    cout << "Unexpected exception in tBBS3" << endl;
  }

}

