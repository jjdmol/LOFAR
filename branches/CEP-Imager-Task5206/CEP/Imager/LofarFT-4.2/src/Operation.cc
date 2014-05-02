//# OperationClean.cc:
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
#include <LofarFT/Operation.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
Operation::Operation(ParameterSet& parset):
    itsParset(parset),
    needsData(false),
    needsImage(false),
    needsFTMachine(false)
{}

void Operation::init()
{
  itsMSName = itsParset.getString("ms");
  itsMS = MeasurementSet(itsMSName, Table::Update);

  itsImager = new Imager(itsMS, itsParset);

  if (needsData) initData();
  if (needsImage) initImage();
  if (needsFTMachine) initFTMachine();
}


void Operation::initData()
{
  String select = itsParset.getString("select","");
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

  String chanmode  = itsParset.getString("chanmode","channel");

  Vector<Int> chansel(1);
  chansel(0)=chfreq(0).shape()[0];

  Vector<Int> chanstart(itsParset.getIntVector("chanstart",std::vector<int>(1,0)));
  Vector<Int> chanstep(itsParset.getIntVector("chanstep",std::vector<int>(1,1)));

  String antenna = itsParset.getString("antenna","");
  String uvdist = itsParset.getString("uvdist","");

  Int fieldid = itsParset.getInt("field",0);

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

void Operation::initImage()
{
  Quantity qcellsize = readQuantity (itsParset.getString("cellsize","1arcsec"));
  MDirection phaseCenter;
  Bool doShift = False;
  Int fieldid = 0;
  Int nfacet = 0;

  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};


  itsImager->defineImage (
      itsParset.getInt("npix",256),        // nx
      itsParset.getInt("npix",256),        // ny
      qcellsize,                           // cellx
      qcellsize,                           // celly
      String("I"),                         // stokes
      phaseCenter,                         // phaseCenter
      doShift  ?  -1 : fieldid,            // fieldid
          itsParset.getString("mode","mfs"),   // mode
          itsParset.getInt("img_nchan",1),     // nchan
          itsParset.getInt("img_chanstart",0), // start
          itsParset.getInt("img_chanstep",1),  // step
          MFrequency(),                        // mFreqstart
          MRadialVelocity(),                   // mStart
          Quantity(1,"km/s"),                  // qstep, Def=1 km/s
          wind,//spwid,                        // spectralwindowids
          nfacet);                             // facets
}

void Operation::initFTMachine()
{
  Int nterms = itsParset.getInt("nterms",1);
  Double RefFreq = itsParset.getDouble("RefFreq");
  itsImager->settaylorterms(nterms,RefFreq);

  itsImager->createFTMachine();
}

void Operation::makeEmptyImage(const String& imgName, Int fieldid)
{
    CoordinateSystem coords;
    AlwaysAssert (itsImager->imagecoordinates(coords), AipsError);
    String name(imgName);
    itsImager->makeEmptyImage(coords, name, fieldid);
    itsImager->unlock();
}

void Operation::showHelpData(ostream& os, const string& name)
{
  os<<
  "Input parameters:"
  "  input.ms         : name of input measurement set with uv-data    "<<endl<<
  "                     string,  no default                           "<<endl<<
  "  input.datacolumn : data column to use                            "<<endl<<
  "                     string,  default \"DATA\"                     "<<endl<<
  "  input.select     : TaQL selection string for MS                  "<<endl<<
  "                     string,  default \"ANTENNA1 != ANTENNA2\"     "<<endl<<
  "  input.uvdist     : UV range in wavelengths                       "<<endl<<
  "                     string,  default \"\"                         "<<endl<<endl;
}

void Operation::showHelpImage(ostream& os, const string& name)
{
  os<<
  "Image pameters:"<<endl<<
  "  image.npix       : number of pixels                              "<<endl<<
  "                     int   ,  default 256                          "<<endl<<
  "  image.cellsize   : pixel width                                   "<<endl<<
  "                     string,  default \"1arcsec\"                  "<<endl<<
  "  image.nterms     : number of terms for wideband imaging          "<<endl<<
  "                     int   ,  default 1                            "<<endl<<endl;
}

void Operation::showHelpFTMachine(ostream& os, const string& name)
{
  os<<
  "Gridding pameters:"<<endl<<
  "  gridding.ftmachine  : FTMachine to use                           "<<endl<<
  "                        string, default FTMachineSimpleWB          "<<endl<<
  "  gridding.oversample : oversampling factor                        "<<endl<<
  "                        int   , default 8                          "<<endl<<endl;
}

// Show the help info.
void Operation::showHelp (ostream& os, const string& name) 
{
  os<<
  "General parameters:"<<endl<<
  "  operation        : operation name                                "<<endl<<
  "                     (string,  no default   )                      "<<endl<<
  "  displayprogress  : display progress                              "<<endl<<
  "                     (bool  ,  default false)                      "<<endl<<
  "  verbose          : verbosity level                               "<<endl<<
  "                     (int   ,  default 0    )                      "<<endl<<
  "  chunksize        : amount of data read at once                   "<<endl<<
  "                     (int   ,  default 0    )                      "<<endl<<endl;

  if (needsData) showHelpData(os,name);
  if (needsImage) showHelpImage(os,name);
  if (needsFTMachine) showHelpFTMachine(os,name);
};


Quantity Operation::readQuantity (const String& in)
{
  Quantity res;
  if (!Quantity::read(res, in)) {
    throw AipsError (in + " is an illegal quantity");
  }
  return res;
}

MDirection Operation::readDirection (const String& in)
{
  Vector<String> vals = stringToVector(in);
  if (vals.size() > 3) {
    throw AipsError ("MDirection value " + in + " is invalid;"
                     " up to 3 values can be given");
  }
  MDirection::Types tp;
  if (! MDirection::getType (tp, vals[0])) {
    throw AipsError(vals[0] + " is an invalid MDirection type");
  }
  Quantity v0(0, "deg");
  Quantity v1(90, "deg");     // same default as in measures.g
  if (vals.size() > 1  &&  !vals[1].empty()) {
    v0 = readQuantity(vals[1]);
  }
  if (vals.size() > 2  &&  !vals[2].empty()) {
    v1 = readQuantity(vals[2]);
  }
  return MDirection(v0, v1, tp);
}

void Operation::readFilter (const String& filter,
                 Quantity& bmajor, Quantity& bminor, Quantity& bpa)
{
  if (filter.empty()) {
    return;
  }
  Vector<String> strs = stringToVector(filter);
  if (strs.size() != 3) {
    throw AipsError("Specify gaussian tapering filter as bmajor,bminor,bpa");
  }
  if (! strs[0].empty()) {
    bmajor = readQuantity (strs[0]);
  }
  if (! strs[1].empty()) {
    bminor = readQuantity (strs[1]);
  }
  if (! strs[2].empty()) {
    bpa = readQuantity (strs[2]);
  }
}



} //# namespace LofarFT

} //# namespace LOFAR
