//  Simulate.cc:
//
//  Copyright (C) 2000-2002
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
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <Common/KeyParser.h>
#include <Common/Debug.h>
#include <string>
#include <iostream>
#include <fstream>
#include "3BlockPerf/AH_3BlockPerf.h"

using namespace LOFAR;
using namespace std;

int main (int argc, const char** argv)
{
  try {

    AH_3BlockPerf simulator;
    
    simulator.setarg (argc, argv);

    // Get input script.
    string name = "3BlockPerf.in";
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
    // Add the nriter if not defined.
    cout << params << endl;

    simulator.baseDefine(params);
    simulator.baseRun(params.getInt("numberOfRuns", 100));
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
    cout << "Unexpected exception in 3BlockPerf" << endl;
  }
}
