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

OperationParamData::OperationParamData()
{
  itsInputParSet.create (
    "select",
    "",
    "TaQL selection string for MS",
    "string");
  
  itsInputParSet.create (
    "antenna", "",
    "Baseline selection string.",
    "string");
  
  itsInputParSet.create (
    "field", 
    "0",
    "field id to be used",
    "int");
  
  itsInputParSet.create (
    "spwid", 
    "0",
    "spectral window id(s) to be used",
    "int vector");

  itsInputParSet.create (
    "chanmode", 
    "channel",
    "frequency channel mode",
    "string");
  
  itsInputParSet.create (
    "nchan", 
    "1",
    "number of frequency channels to select from each spectral window (one number per spw)",
    "int vector");
  
  itsInputParSet.create (
    "chanstart", 
    "0",
    "first frequency channel per each spw (0-relative)",
    "int vector");
  
  itsInputParSet.create (
    "chanstep", 
    "1",
    "frequency channel step per each spw",
    "int vector");
  
  itsInputParSet.create (
    "uvdist", 
    "",
    "UV Range",
    "string");

  
}


void OperationParamData::run()
{
  String select = itsInputParSet.getString("select");
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

  String chanmode  = itsInputParSet.getString("chanmode");
  
  Vector<Int> chansel(1);
  chansel(0)=chfreq(0).shape()[0];
  
  Vector<Int> chanstart(itsInputParSet.getIntVector("chanstart"));
  Vector<Int> chanstep(itsInputParSet.getIntVector("chanstep"));

  String antenna = itsInputParSet.getString("antenna");
  String uvdist = itsInputParSet.getString("uvdist");

  Int fieldid = itsInputParSet.getInt("field");
  
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
    uvdist,                       // uvdist
    String(),                       // scan
    String(),                      // intent
    String(),                       // obs
    True);                         // useModelCol
  
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

} //# namespace LofarFT
} //# namespace LOFAR
