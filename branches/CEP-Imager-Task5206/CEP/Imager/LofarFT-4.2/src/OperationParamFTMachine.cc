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
#include <LofarFT/OperationParamFTMachine.h>


using namespace casa;


namespace LOFAR {
namespace LofarFT {

namespace
{
  Matrix<Bool> readMueller (const String& str, String stokes, Bool grid);
}

OperationParamFTMachine::OperationParamFTMachine()
{
  itsInputParSet.create (
    "ApplyElement", 
    "0",
    "If turned to true, apply the element beam every TWElement.",
    "int");
  
  itsInputParSet.create (
    "TWElement", 
    "3600.",
    "Timewindow for applying the element beam in seconds. The actual number of rows will never exceed RowBlock",
    "Double");
  
  itsInputParSet.create (
    "RowBlock", 
    "1000000",
    "Maximum number of rows to process together. If ApplyBeam=1 then the actual time span will not exceed TWElement",
    "int");
  
  itsInputParSet.create ("wprojplanes", "0",
                  "if >0 specifies nr of convolution functions to use in W-projection",
                  "int");
  itsInputParSet.create (
    "padding", 
    "1.0",
    "padding factor in image plane (>=1.0)",
    "float");
  
  itsInputParSet.create (
    "timewindow", 
    "300.0",
    "width of time window (in sec) where AW-term is constant",
    "double");
  
  itsInputParSet.create ("wmax", "10000.0",
                  "omit data with w-term > wmax (in meters)",
                  "float");
  itsInputParSet.create ("maxsupport", "1024",
                   "maximum support size for W convolution functions",
                   "int");
  itsInputParSet.create ("oversample", "8",
                   "oversampling for convolution functions",
                   "int");
  itsInputParSet.create ("applyIonosphere", "false",
                   "apply ionospheric correction",
                   "bool");
  itsInputParSet.create ("parmdbname", "instrument",
                   "Name of parmdb (default is instrument",
                   "string");
  
  itsInputParSet.create ("splitbeam", "true",
                   "Evaluate station beam and element beam separately (splitbeam = true is faster)",
                   "bool");
  
  itsInputParSet.create (
    "PBCut", 
    "1e-2",
    "Level below which the dirty images will be set to zero. Expressed in units of peak primary beam.",
    "Double");
  
  itsInputParSet.create (
    "FindNWplanes", 
    "true",
    "If set to true, then find the optimal number of W-planes, given spheroid support, wmax and field of view.",
    "bool");
  
  itsInputParSet.create (
    "RefFreq", 
    "",
    "Reference Frequency (Hz)",
    "Double");
  
  itsInputParSet.create (
    "nterms", 
    "1",
    "Number of Taylor terms",
    "int");
  
  itsInputParSet.create (
    "ChanBlockSize", 
    "0",
    "Channel block size. Use if you want to use a different CF per block of channels.",
    "int");
  
  
  itsInputParSet.create ("ATerm", "ATerm",
                  "ATerm class (ATerm, ATermPython)",
                  "string");

  itsInputParSet.create ("ATermPython.module", "",
                  "Name of python module containing ATerm class",
                  "string");

  itsInputParSet.create ("ATermPython.class", "",
                  "Name of ATerm Python class",
                  "string");
  
}


void OperationParamFTMachine::run()
{
  
  Int nterms = itsInputParSet.getInt("nterms");
  itsParameters.define("nterms", nterms);
  Double RefFreq = itsInputParSet.getDouble("RefFreq");
  itsImager->settaylorterms(nterms,RefFreq);
  itsParameters.define("RefFreq", RefFreq);

  itsParameters.define("padding", itsInputParSet.getDouble("padding"));

  Int StepApplyElement = itsInputParSet.getInt("ApplyElement");
  if (StepApplyElement) StepApplyElement |= 1;
  itsParameters.define("StepApplyElement", StepApplyElement);

  itsParameters.define("TWElement", itsInputParSet.getDouble("TWElement"));
  itsParameters.define("splitbeam", itsInputParSet.getBool("splitbeam"));
  itsParameters.define("wmax", itsInputParSet.getDouble("wmax"));
  itsParameters.define("maxsupport", itsInputParSet.getInt("maxsupport"));
  itsParameters.define("oversample", itsInputParSet.getInt("oversample"));
  itsParameters.define("RowBlock", itsInputParSet.getInt("RowBlock"));
  itsParameters.define("FindNWplanes", itsInputParSet.getBool("FindNWplanes"));
  itsParameters.define("ChanBlockSize", itsInputParSet.getInt("ChanBlockSize"));
  itsParameters.define("ATerm", itsInputParSet.getString("ATerm"));
  
  Matrix<Bool> muelgrid = readMueller ("ALL", "IQUV", true);
  itsParameters.define ("mueller.grid", muelgrid);
  itsParameters.define ("mueller.degrid", muelgrid);
  
}

namespace
{
  Matrix<Bool> readMueller (const String& str, String stokes, Bool grid)
  {
    Matrix<Bool> mat(4,4, True);
    String s(str);
    s.upcase();
    if (s == "FULL") {
      s = "ALL";
    }
    if (s == "DIAGONAL") 
    {
      mat = False;
      mat.diagonal() = True;
    } 
    else if (s != "ALL" ) 
    {
      mat(0,4) = mat(4,0) = False;
      if (s == "BAND1") 
      {
        mat(0,3) = mat(1,4) = mat(3,0) = mat(4,1) = False;
      } 
      else if (s != "BAND2") 
      {
        throw AipsError (str + " is an invalid Mueller specification");
      }
    }
    if((stokes=="I")&&(grid)){
      for(uInt i=0;i<4;++i){
        mat(1,i)=0;
        mat(2,i)=0;
      };
    }
    if((stokes=="I")&&(!grid)){
      for(uInt i=0;i<4;++i){
        mat(1,i)=0;
        mat(2,i)=0;
        // mat(i,1)=0;
        // mat(i,2)=0;
      };
    };
    return mat;
  }
}


} //# namespace LofarFT
} //# namespace LOFAR
