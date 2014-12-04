//# Operation.cc:
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
#include <LofarFT/util.h>
#include <casa/OS/Directory.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
Operation::Operation(ParameterSet& parset):
    itsParset(parset),
    needsData(false),
    needsImage(false),
    needsFTMachine(false),
    needsWeight(false)
{}

void Operation::init()
{
  itsMSName = itsParset.getString("data.ms");
  itsMS = MeasurementSet(itsMSName, Table::Update);

  itsImager = new Imager(itsMS, itsParset);

  if (needsData) initData();
  if (needsImage) initImage();
  if (needsWeight) initWeight();
  if (needsFTMachine) initFTMachine();
}


void Operation::initData()
{
  String select = itsParset.getString("data.query","");
  if (select.empty())
  {
    select = "ANTENNA1 != ANTENNA2";
  }
  else
  {
    select = '(' + select + ") && ANTENNA1 != ANTENNA2";
  }

//   if (!itsMS.tableDesc().isColumn("CORRECTED_DATA"))
//   {
//     throw(AipsError("CORRECTED_DATA column not found."));
//   }
  
  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

  ROArrayColumn<Double> chfreq(window.chanFreq());

  String chanmode  = "channel";//itsParset.getString("chanmode","channel");

  Vector<Int> chansel(1);
  chansel(0)=chfreq(0).shape()[0];

  Vector<Int> chanstart(itsParset.getIntVector("data.chanstart",std::vector<int>(1,0)));
  Vector<Int> chanstep(itsParset.getIntVector("data.chanstep",std::vector<int>(1,1)));

  String antenna = itsParset.getString("data.baselines","");
  String uvdist = itsParset.getString("data.uvrange","");

  Int fieldid = itsParset.getInt("data.field",0);

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

}

void Operation::initWeight()
{
  // Define weighting.
  itsImager->set_imaging_weight(itsParset);
}

void Operation::initImage()
{
  Quantity qcellsize = readQuantity (itsParset.getString("image.cellsize","1arcsec"));
  MDirection phaseCenter;
  Bool doShift = False;
  Int fieldid = 0;
  Int nfacet = 0;

  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};


  itsImager->defineImage (
      itsParset.getInt("image.npix",256),        // nx
      itsParset.getInt("image.npix",256),        // ny
      qcellsize,                           // cellx
      qcellsize,                           // celly
      String("I"),                         // stokes
      phaseCenter,                         // phaseCenter
      doShift  ?  -1 : fieldid,            // fieldid
          itsParset.getString("image.mode","mfs"),   // mode
          itsParset.getInt("image.img_nchan",1),     // nchan
          itsParset.getInt("image.img_chanstart",0), // start
          itsParset.getInt("image.img_chanstep",1),  // step
          MFrequency(),                        // mFreqstart
          MRadialVelocity(),                   // mStart
          Quantity(1,"km/s"),                  // qstep, Def=1 km/s
          wind,//spwid,                        // spectralwindowids
          nfacet);                             // facets
}

void Operation::initFTMachine()
{
  Int nterms = itsParset.getInt("image.nterms",1);
  Double reffreq = get_reference_frequency(itsParset, itsMS);
  itsImager->settaylorterms(nterms,reffreq);

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
  "Data parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "data.ms" << COLOR_RESET<< "          : name of input measurement set with uv-data    "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "no default" << COLOR_RESET << endl <<
  "  " << COLOR_PARAMETER << "data.query" << COLOR_RESET<< "       : TaQL selection string for MS                  "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "default \"\"" << COLOR_RESET <<endl <<
  "  " << COLOR_PARAMETER << "data.uvrange" << COLOR_RESET<< "     : UV range, for example 1~10klambda      "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "default \"\"" << COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "data.baselines" << COLOR_RESET<< "   : baseline selection string                     "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "default \"\"" << COLOR_RESET << endl<<endl;
}

void Operation::showHelpImage(ostream& os, const string& name)
{
  os<<
  "Image parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "image.npix" << COLOR_RESET<< "       : number of pixels                              "<<endl<<
  "                     int   ,  " << COLOR_DEFAULT << "default 256" << COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "image.cellsize" << COLOR_RESET<< "   : pixel width                                   "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "default \"1arcsec\"" << COLOR_RESET << endl <<
  "  " << COLOR_PARAMETER << "image.reffreq" << COLOR_RESET<< "    : reference frequency (Hz), only used for multi-term images"<<endl<<
  "                     double,  " << COLOR_DEFAULT << 
  "default is reference frequency from ms" << COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "image.nterms" << COLOR_RESET<< "     : number of terms for wideband imaging          "<<endl<<
  "                     int   ,  " << COLOR_RESET << "default 1" << COLOR_RESET << endl << endl;
}

void Operation::showHelpFTMachine(ostream& os, const string& name)
{
  os<<
  "Gridding parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "gridding.ftmachine" << COLOR_RESET<< "  : FTMachine to use                           "<<endl<<
  "                        string, " << COLOR_DEFAULT << 
  "default FTMachineSplitBeamWStackWB" << COLOR_RESET << endl <<
  "  " << COLOR_PARAMETER << "gridding.oversample" << COLOR_RESET<< " : oversampling factor                        "<<endl<<
  "                        int   , " << COLOR_DEFAULT << "default 9" << 
  COLOR_RESET << endl<<
  endl;
}

void Operation::showHelpWeight(ostream& os, const string& name)
{
  os<<
  "Weight parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "weight.type" << COLOR_RESET << "       : weighting scheme,                            "<<endl<<
  "                        string, " << COLOR_DEFAULT << "default natural" << COLOR_RESET << endl<<
  "                        (natural, robust, uniform)                 "<<endl<<endl;
  os<<
  "  " << COLOR_PARAMETER << "weight.robust" << COLOR_RESET << "     : robustness,                                  "<<endl<<
  "                        float, " << COLOR_DEFAULT << "default 0.0" << COLOR_RESET << endl<<endl;
  os<<
  "  " << COLOR_PARAMETER << "weight.mode" << COLOR_RESET << "       : robust weighting mode,                       "<<endl<<
  "                        string, " << COLOR_DEFAULT << "default norm" << COLOR_RESET << endl<<
  "                        (norm, abs)                                "<<endl<<endl;
  os<<
  "  " << COLOR_PARAMETER << "weight.noise" << COLOR_RESET<< "       : robust abs noise,                           "<<endl<<
  "                        string, " << COLOR_DEFAULT << "default 0Jy" << COLOR_RESET << endl<<endl;
}

// Show the help info.
void Operation::showHelp (ostream& os, const string& name) 
{
  os<<
  "General parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "operation" << COLOR_RESET<< "        : operation name                                "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "no default" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "verbose" << COLOR_RESET<< "          : verbosity level                               "<<endl<<
  "                     int   ,  " << COLOR_DEFAULT << "default 0" << COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "chunksize" << COLOR_RESET<< "        : amount of data read at once (0 means automatic)"<<endl<<
  "                     int   ,  " << COLOR_DEFAULT << "default 0" << COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "numthreads" << COLOR_RESET<< "       : maximum number of threads to use              "<<endl<<
  "                     int   ,  " << COLOR_DEFAULT << "default 0 (= use sytem default)" << COLOR_RESET <<endl<<endl;

  os<<
  "Output parameters:"<<endl<<
  "  " << COLOR_PARAMETER << "output.imagename" << COLOR_RESET << " : base name for output image                    "<<endl<<
  "                     string,  " << COLOR_DEFAULT << "no default" << COLOR_RESET << endl<<endl;

  if (needsData) showHelpData(os,name);
  if (needsImage) showHelpImage(os,name);
  if (needsFTMachine) showHelpFTMachine(os,name);
  if (needsFTMachine) showHelpWeight(os,name);
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

void Operation::normalize(String imagename_in, String avgpb_name, String imagename_out)
{
  Directory imagedir_in(imagename_in);
  imagedir_in.copy (imagename_out);
  
  PagedImage<Float> image_in(imagename_in);
  PagedImage<Float> image_out(imagename_out);
  PagedImage<Float> avgpb(avgpb_name);

  for(Int i = 0; i < image_in.shape()[3]; ++i)
  {
    for(Int j = 0; j < image_in.shape()[2]; ++j)
    {
      for(Int k = 0; k < image_in.shape()[1]; ++k)
      {
        for(Int l = 0; l < image_in.shape()[0]; ++l)
        {
          IPosition pos(4,l,k,j,i);
          IPosition pos1(4,l,k,0,0);
          
          Float v = image_in.getAt(pos);
          Float f = avgpb.getAt(pos1);
          if (f>0.02)
          {
            image_out.putAt(v/f, pos);
          }
          else
          {
            image_out.putAt(0.0, pos);
          }
        }
      }
    }
  }
}

} //# namespace LofarFT

} //# namespace LOFAR
