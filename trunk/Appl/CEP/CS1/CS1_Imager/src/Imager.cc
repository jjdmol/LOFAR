//#  Imager.cc: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#  @author Adriaan Renting, renting@astron.nl
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

#include <PLC/ACCmain.h>
#include <casa/Exceptions.h>
#include <CS1_Imager/ImagerProcessControl.h>

int main(int argc, char *argv[])
{
  try
  {
    LOFAR::CS1::ImagerProcessControl myProcess;
    return LOFAR::ACC::PLC::ACCmain(argc, argv, &myProcess);
  } //try
  catch(casa::AipsError& err)
  {
    std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
    return -2;
  }
}
