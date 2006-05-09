//#  CS1_BGL_Processing_main.cc:
//#
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

#include <lofar_config.h>

#include <PLC/ACCmain.h>
#include <Common/LofarLogger.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <CS1_BGLProc/AH_BGL_Processing.h>

using namespace LOFAR;
using namespace LOFAR::CS1;

int main(int argc, char **argv) {
  INIT_LOGGER("CS1_BGL_Processing");

  AH_BGL_Processing myAH;
  ApplicationHolderController myAHController(myAH, 1); //listen to ACC every 1 runs
  return ACC::PLC::ACCmain(argc, argv, &myAHController);
}
