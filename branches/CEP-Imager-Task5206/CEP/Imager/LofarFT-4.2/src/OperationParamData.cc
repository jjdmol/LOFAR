//# OperationParamFTMachine.cc:
//#
//# Copyright (C) 2014
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
//# $Id: $

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <LofarFT/OperationParamData.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {

OperationParamData::OperationParamData(ParameterSet& parset): Operation(parset)
{
  String select = parset.getString("select","");
  if (select.empty()) 
  {
    select = "ANTENNA1 != ANTENNA2";
  }
  else 
  {
    select = '(' + select + ") && ANTENNA1 != ANTENNA2";
  }

  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

  ROArrayColumn<Double> chfreq(window.chanFreq());

  String chanmode  = parset.getString("chanmode","channel");
  
  Vector<Int> chansel(1);
  chansel(0)=chfreq(0).shape()[0];
  
  Vector<Int> chanstart(parset.getIntVector("chanstart",std::vector<int>(1,0)));
  Vector<Int> chanstep(parset.getIntVector("chanstep",std::vector<int>(1,1)));

  String antenna = parset.getString("antenna","");
  String uvdist = parset.getString("uvdist","");

  Int fieldid = parset.getInt("field",0);
  
  itsImager->setdata (
    chanmode,                       // mode
    chansel,//nchan,
    chanstart,
    chanstep,
    MRadialVelocity(),              // mStart
    MRadialVelocity(),              // mStep
    wind,//spwid,
    Vector<Int>(1,fieldid),
    select,                         // msSelect
    String(),                       // timerng
    String(),                       // fieldnames
    Vector<Int>(),                  // antIndex
    antenna,                        // antnames
    String(),                       // spwstring
    uvdist,                         // uvdist
    String(),                       // scan
    String(),                       // intent
    String(),                       // obs
    True);                          // useModelCol
  
  String weight("natural");
  String rmode;
  Double robust;
  Double noise;
  
  // Define weighting.
  itsImager->weight (
    weight,                      // type
    rmode,                       // rmode
    Quantity(noise, "Jy"),       // briggsabs noise
    robust,                      // robust
    Quantity(0, "rad"),          // fieldofview
    0);                          // npixels
}

void OperationParamData::init()
{
}


void OperationParamData::run()
{

}

void OperationParamData::showHelp (ostream& os, const string& name)
{
  //TODO
};

} //# namespace LofarFT
} //# namespace LOFAR
