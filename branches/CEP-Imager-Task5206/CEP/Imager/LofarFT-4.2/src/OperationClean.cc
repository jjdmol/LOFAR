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

OperationClean::OperationClean()
{
  itsInputParSet.create (
    "niter", 
    "1000",
    "Number of clean iterations",
    "int");
  
  itsInputParSet.create (
    "gain", 
    "0.1",
    "Loop gain for cleaning",
    "float");
  
  itsInputParSet.create (
    "threshold", 
    "0Jy",
    "Flux level at which to stop cleaning",
    "quantity string");
  
  itsInputParSet.create (
    "fixed", 
    "False",
    "Keep clean model fixed",
    "bool");
  
  itsInputParSet.create (
    "cyclefactor", 
    "1.5",
    "Cycle Factor. See Casa definition.",
    "Double");
  
  itsInputParSet.create (
    "cyclespeedup", 
    "-1",
    "Cycle Factor. See Casa definition.",
    "Double");
  
  itsInputParSet.create (
    "PsfImage", 
    "",
    "Input PSF image for the cleaning",
    "string");
  
  itsInputParSet.create (
    "nscales", 
    "5",
    "Number of scales for MultiScale Clean",
    "int");
  
  itsInputParSet.create (
    "uservector", 
    "0",
    "user-defined scales for MultiScale clean",
    "float vector");
    
  itsInputParSet.create (
    "model", 
    "", 
    "Name of model image file (default is <imagename>.model", 
    "string");
  
  itsInputParSet.create (
    "restored", 
    "", 
    "Name of restored image file (default is <imagename>.restored", 
    "string");
  
  itsInputParSet.create (
    "residual", 
    "", 
    "Name of residual image file (default is <imagename>.residual", 
    "string");
  
  itsInputParSet.create (
    "psf", 
    "", 
    "Name of psf image file (default is <imagename>.psf", 
    "string");

  itsInputParSet.create (
      "mask", 
      "",
      "Name of the mask to use in cleaning",
      "string");
}

void OperationClean::run()
{
  OperationImageBase::run();
  OperationParamData::run();

  cout << "Hi, I am OperationClean::run" << endl;

  Vector<Double> userScaleSizes(itsInputParSet.getDoubleVector("uservector"));  
  String scaleMethod;
  Vector<Float> userVector(1); userVector(0)=0;
//   convertArray (userVector, userScaleSizes);
  
//   if (userScaleSizes.size() > 1) 
//   {
    scaleMethod = "uservector";
//   } 
//   else 
//   {
//     scaleMethod = "nscales";
//   }
  
  itsImager->setscales(scaleMethod, 1, userVector);

  String imgName = itsParameters.asString("imagename");  

  Int nterms = itsParameters.asInt("nterms");

  Vector<String> modelNames(nterms);
  Vector<String> residualNames(nterms);
  Vector<String> restoredNames(nterms);
  Vector<String> psfNames(nterms);
  
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
      psfNames(i) = imgName + ".psf.tt" + String::toString(i);
    }
  }
  
  Int niter = itsInputParSet.getInt("niter");
  Double gain = itsInputParSet.getDouble("gain");
  
  String threshStr = itsInputParSet.getString("threshold");
  Quantity threshold = readQuantity (threshStr);

  Bool displayProgress = False;
  
  String maskName  = itsInputParSet.getString("mask");
  
  Double cyclefactor   = itsInputParSet.getDouble("cyclefactor");
  Double cyclespeedup  = itsInputParSet.getDouble("cyclespeedup");
  
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

  
  itsImager->clean(
    "msmfs",                     // algorithm,
    niter,                         // niter
    gain,                          // gain
    threshold,                     // threshold
    displayProgress,               // displayProgress
    modelNames,
    Vector<Bool>(nterms, False),        // fixed
    "",                            // complist
    Vector<String>(1, maskName),   // mask
    restoredNames,  // restored
    residualNames,  // residual
    psfNames);   // psf

}

} //# namespace LofarFT

} //# namespace LOFAR
