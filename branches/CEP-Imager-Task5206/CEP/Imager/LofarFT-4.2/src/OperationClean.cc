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
#include <LofarFT/OperationClean.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
// Register OperationClean with the OperationFactory. Use an unnamed
// namespace. This ensures that the variable `dummy' gets its own private
// storage area and is only visible in this compilation unit.
namespace
{
  bool dummy = OperationFactory::instance().
    registerClass<OperationClean>("clean");
}

OperationClean::OperationClean(ParameterSet& parset): Operation(parset)
{
    needsData=true;
    needsImage=true;
    needsWeight=true;
    needsFTMachine=true;
}

void OperationClean::init()
{
  Operation::init();

  Vector<Float> userVector(itsParset.getFloatVector("clean.uservector",std::vector<float>(0)));
  
  Int nscales = itsParset.getInt("clean.nscales", 0);
  if (nscales < 1) nscales = 1;
  
  String scaleMethod = "uservector"; // always set to uservector, see comment below
  
  if (userVector.size() == 0) 
  {
    userVector.resize(nscales);
    userVector(0) = 0;
    // This is how the scales are set in MFMSCleanImageSkyModel and MSCleanImageSkyModel.
    // WBCleanImageSkyModel does not set scales when no uservector is supplied
    // therefore we fill the uservector here.
    Float scaleInc = 2.0;
    for (Int scale=1; scale<nscales;scale++) 
    {
      userVector(scale) = scaleInc * pow(10.0, (Float(scale)-2.0)/2.0);
    }
  }
  
  itsImager->setscales(scaleMethod, nscales, userVector);


  Double cyclefactor   = itsParset.getDouble("clean.cyclefactor",1.5);
  Double cyclespeedup  = itsParset.getDouble("clean.cyclespeedup",-1);


  itsImager->setmfcontrol(
    cyclefactor,          //Float cyclefactor,
    cyclespeedup,         //Float cyclespeedup,
    0.8,                        //Float cyclemaxpsffraction,
    2,                          //Int stoplargenegatives,
    -1,                         //Int stoppointmode,
    "",                         //String& scaleType,
    0.1,                        //Float minPB,
    0.4,                        //loat constPB,
    Vector<String>(1, ""),      //Vector<String>& fluxscale,
    true);                      //Bool flatnoise);

  String imgName = itsParset.getString("output.imagename");

  Int nterms = itsParset.getInt("image.nterms",1);

  Vector<String> modelNames(nterms);
  Vector<String> residualNames(nterms);
  Vector<String> restoredNames(nterms);
  Vector<String> psfNames((nterms*(nterms+1))/2);
  
  if (nterms == 1)
  {
    modelNames(0) = imgName + ".model";
    residualNames(0) = imgName + ".residual";
    restoredNames(0) = imgName + ".restored";
    psfNames(0) = imgName + ".psf";
  }
  else
  {
    for(Int i=0;i<nterms;++i)
    {
      modelNames(i) = imgName + ".model.tt" + String::toString(i);
      residualNames(i) = imgName + ".residual.tt" + String::toString(i);
      restoredNames(i) = imgName + ".restored.tt" + String::toString(i);
    }
    for(Int i=0;i<((nterms*(nterms+1))/2);++i)
    {
      psfNames(i) = imgName + ".psf.tt" + String::toString(i);
    }
  }
  
  Int niter = itsParset.getInt("clean.niter",1000);
  Double gain = itsParset.getDouble("clean.gain",0.1);

  String threshStr = itsParset.getString("clean.threshold","0Jy");
  Quantity threshold = readQuantity (threshStr);

  Bool displayProgress = False;
  
  String maskName  = itsParset.getString("clean.maskimage","");
  
  itsImager->initClean(
    "msmfs",                     // algorithm,
    niter,                       // niter
    gain,                        // gain
    threshold,                   // threshold
    modelNames,
    Vector<Bool>(nterms, False), // fixed
    "",                          // complist
    Vector<String>(1, maskName), // mask
    restoredNames,               // restored
    residualNames,               // residual
    psfNames);                   // psf
}

void OperationClean::run()
{
  itsImager->doClean();
}


void OperationClean::showHelp (ostream& os, const string& name)
{
  Operation::showHelp(os,name);

  os<<
  "Operation \"clean\": perform a clean cycle                          "<<endl<<
  "Parameters:                                                         "<<endl<<
  "  clean.niter       : number of clean iterations                    "<<endl<<
  "                      int   ,  default 1000                         "<<endl<<
  "  clean.threshold   : flux level at which to stop cleaning          "<<endl<<
  "                      string,  \"0Jy\"                              "<<endl<<
  "  clean.maskimage   : name of the mask image to use in cleaning     "<<endl<<
  "                      string,  default \"\"                         "<<endl<<
  "  clean.cyclefactor : see casa documentation                        "<<endl<<
  "                      double,  default 1.5                          "<<endl<<
  "  clean.cyclespeedup: see casa documentation                        "<<endl<<
  "                      double,  default -1                           "<<endl<<
  "  clean.nscales     : number of scales for multiscale clean         "<<endl<<
  "                      int   ,  default 5                            "<<endl<<
  "  clean.uservector  : user-defined scales for multi-scale clean     "<<endl<<
  "                      float vector,  default [0.]                   "<<endl<<endl;
};


} //# namespace LofarFT

} //# namespace LOFAR
