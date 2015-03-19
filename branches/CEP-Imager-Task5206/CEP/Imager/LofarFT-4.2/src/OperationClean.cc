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

OperationClean::OperationClean(ParameterSet& parset): Operation(parset),
  itsModelNames(0),
  itsResidualNames(0),
  itsRestoredNames(0),
  itsModelNames_normalized(0),
  itsResidualNames_normalized(0),
  itsRestoredNames_normalized(0),
  itsPsfNames(0)
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

  itsModelNames.resize(nterms);
  itsResidualNames.resize(nterms);
  itsRestoredNames.resize(nterms);
  itsModelNames_normalized.resize(nterms);
  itsResidualNames_normalized.resize(nterms);
  itsRestoredNames_normalized.resize(nterms);
  itsPsfNames.resize((nterms*(nterms+1))/2);
  
  itsAvgpbName = imgName + ".avgpb";

  if (nterms == 1)
  {
    itsModelNames(0) = imgName + ".model.flatnoise";
    itsModelNames_normalized(0) = imgName + ".model.flatgain";
    itsResidualNames(0) = imgName + ".residual.flatnoise";
    itsResidualNames_normalized(0) = imgName + ".residual.flatgain";
    itsRestoredNames(0) = imgName + ".restored.flatnoise";
    itsRestoredNames_normalized(0) = imgName + ".restored.flatgain";
    itsPsfNames(0) = imgName + ".psf";
  }
  else
  {
    for(Int i=0;i<nterms;++i)
    {
      itsModelNames(i) = imgName + ".model.tt" + String::toString(i) + ".flatnoise";
      itsModelNames_normalized(i) = imgName + ".model.tt" + String::toString(i) + ".flatgain";
      itsResidualNames(i) = imgName + ".residual.tt" + String::toString(i)  + ".flatnoise";
      itsResidualNames_normalized(i) = imgName + ".residual.tt" + String::toString(i)  + ".flatgain";
      itsRestoredNames(i) = imgName + ".restored.tt" + String::toString(i)  + ".flatnoise";
      itsRestoredNames_normalized(i) = imgName + ".restored.tt" + String::toString(i)  + ".flatgain";
    }
    for(Int i=0;i<((nterms*(nterms+1))/2);++i)
    {
      itsPsfNames(i) = imgName + ".psf.tt" + String::toString(i);
    }
  }
  
  Int niter = itsParset.getInt("clean.niter",1000);
  Double gain = itsParset.getDouble("clean.gain",0.1);

  String threshStr = itsParset.getString("clean.threshold","0Jy");
  Quantity threshold = readQuantity (threshStr);

  Bool displayProgress = False;
  
  String maskName  = itsParset.getString("clean.maskimage","");
  
  if ((maskName != "") && !Table::isReadable(maskName))
  {
    throw(AipsError("Mask " + maskName + " is not readable."));
  }
  
  itsImager->initClean(
    "msmfs",                     // algorithm,
    niter,                       // niter
    gain,                        // gain
    threshold,                   // threshold
    itsModelNames,
    Vector<Bool>(nterms, False), // fixed
    "",                          // complist
    Vector<String>(1, maskName), // mask
    itsRestoredNames,               // restored
    itsResidualNames,               // residual
    itsPsfNames);                   // psf
}

void OperationClean::run()
{
  itsImager->doClean();
  
  // make flat gain images
  for(Int i; i<itsModelNames.shape()(0); ++i)
  {
    normalize(itsModelNames(i), itsAvgpbName, itsModelNames_normalized(i));
    normalize(itsResidualNames(i), itsAvgpbName, itsResidualNames_normalized(i));
    normalize(itsRestoredNames(i), itsAvgpbName, itsRestoredNames_normalized(i));
  }  
}

void OperationClean::showHelp (ostream& os, const string& name)
{
  os<<
  COLOR_OPERATION <<
  "Operation \"clean\": perform a clean cycle                          "<<endl<<
  COLOR_RESET << endl <<
  "Parameters:                                                         "<<endl<<
  "  " << COLOR_PARAMETER << "clean.niter" << COLOR_RESET << "       : number of clean iterations                    "<<endl<<
  "                      int   ,  " << COLOR_DEFAULT << "default 1000" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "clean.threshold" << COLOR_RESET << "   : flux level at which to stop cleaning          "<<endl<<
  "                      string,  " << COLOR_DEFAULT << "default \"0Jy\"" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "clean.maskimage" << COLOR_RESET << "   : name of the mask image to use in cleaning     "<<endl<<
  "                      string,  " << COLOR_DEFAULT << "default \"\"" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "clean.cyclefactor" << COLOR_RESET << " : see casa documentation                        "<<endl<<
  "                      double,  " << COLOR_DEFAULT << "default 1.5" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "clean.cyclespeedup" << COLOR_RESET << ": see casa documentation                        "<<endl<<
  "                      double,  " << COLOR_DEFAULT << "default -1"<< COLOR_RESET << endl<<
  "  " << COLOR_PARAMETER << "clean.nscales" << COLOR_RESET << "     : number of scales for multiscale clean         "<<endl<<
  "                      int   ,  " << COLOR_DEFAULT << "default 1" << COLOR_RESET <<endl<<
  "  " << COLOR_PARAMETER << "clean.uservector" << COLOR_RESET << "  : user-defined scales for multi-scale clean     "<<endl<<
  "                      float vector,  " << COLOR_DEFAULT << "default [0.]" << COLOR_RESET<< endl<<endl;
  Operation::showHelp(os,name);
};


} //# namespace LofarFT

} //# namespace LOFAR
