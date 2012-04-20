//# ModelImageFFT.h: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: ModelImageFFT.cc 20029 2012-04-19 15:50:23Z duscha $

#include <lofar_config.h>

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageFFT.h>

using namespace std;
using namespace casa;

using namespace LOFAR;
using namespace BBS;

//*********************************************
//
// Constructors and destructor
//
//*********************************************

ModelImageFft::ModelImageFft( const casa::String &name,
                              const casa::Vector<casa::Double> &frequencies,
                              int oversampling,
                              double uvscaleX, double uvscaleY)
{

}

ModelImageFft::~ModelImageFft(void)
{
}

void ModelImageFft::setConvType(const casa::String type)
{
  if(type=="SF" || type=="BOX")
  {
    itsOptions.ConvType=type;
  }
  else
  {
    THROW(BBSKernelException, "Convolution type " << type << " is unknown.");
  }
}

void setUVscale(double uvscaleX, double uvscaleY)
{

}

void setDegridMuellerMask(const casa::Matrix<Bool> &muellerMask)
{
  if(muellerMask.nrow() != 4 && muellerMask.ncolumn() != 4)
  {
    setDegridMuellerMask(muellerMask);
  }
  else
  {
    setDegridMuellerMask(muellerMask);
  }
}