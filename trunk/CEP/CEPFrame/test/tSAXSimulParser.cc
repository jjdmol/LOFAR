//  simparser.cc: Simple tester for SAXParser class
//
//  Copyright (C) 2000, 2001
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
//  $Log$
//  Revision 1.4  2002/05/24 07:51:42  wierenga
//  %[BugId: 12]%
//  Return 0 (success) if XERCES is not configured.
//
//  Revision 1.3  2002/05/07 08:54:36  gvd
//  Added package to includes
//
//  Revision 1.2  2002/05/03 11:21:33  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.1  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/SAXSimulParser.h"
#include "BaseSim/Simul2XML.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/WH_Example.h"
#include "BaseSim/WH_Tester.h"
#include <Common/lofar_iostream.h>
#include <string.h>

#if HAVE_XERCES
int main(int argc, char* argv[])
{
  try {
    cout << "SAXSimulParser. XML parser for LOFARSim simulations." << endl;
    string fname = "tSAXSimulParser.in";
    if (argc > 1) {
      fname = argv[1];
    }
    WorkHolder::registerConstruct ("WH_Empty",   WH_Empty::construct);
    WorkHolder::registerConstruct ("WH_Example", WH_Example::construct);
    WorkHolder::registerConstruct ("WH_Tester",  WH_Tester::construct);
    SAXSimulParser parser(fname);
    Simul simul = parser.parseSimul();
    Simul2XML xml(simul);
    xml.write ("tSAXSimulParser_tmp.xml");
    return 0;
  } catch (XMLException& x) {
    cerr << "Ucaught SAX exception: "
	 << SAXLocalStr(x.getMessage()) << endl;
  } catch (std::exception& x) {
    cerr << "Uncaught std exception: " << x.what() << endl;
  } catch (...) {
    cerr << "Uncaught unknown exception" << endl;
  }
  return 1;
}
#else
int main(int,char**)
{
  cout << "XERCES is not configured, therefore "
       << "this test doesn't do anything.\n" << endl;
  return 0;
}
#endif
