//  SelfcalEngineStub.h: one line description
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include "SelfcalEngineStub.h"

#include <unistd.h>
#include <iostream>
#include <ostream>


SelfcalEngineStub::SelfcalEngineStub():itsParamValues(NULL) {
  std::cout << "SelfcalEngineStub Constructor " << std::endl;
}

SelfcalEngineStub::~SelfcalEngineStub() {
  std::cout << "SelfcalEngineStub Destructor " << std::endl;
  if (itsParamValues != NULL) delete[] itsParamValues;
}

void SelfcalEngineStub::init(int len,
			     float parameters[]) {
  if (itsParamValues != NULL) delete[] itsParamValues;
  itsLen = len;

  itsParamValues = new float[len];
  for (int i=0; i<len; i++) {
    itsParamValues[i] = parameters[i];
  }
}

float * SelfcalEngineStub::Solve(bool    *workdef, 
                                  float   outparams[]) {

  std::cout << "start Solving" << std::flush;
  for (int i=0; i<itsLen; i++) {
    //sleep(1);
    std::cout << "." << std::flush;

    if (workdef[i] == true) {
      outparams[i] = itsParamValues[i] = 0.8*(itsParamValues[i] + i + 1) + 0.01*(itsParamValues[(i+1)%itsLen]);
      std::cout << i << " " << outparams[i] << "  ";
    }
  }
  std::cout << "OK" << std::endl;;
  
  return outparams;
  
}


void SelfcalEngineStub::dump() {
  for (int i=0; i<itsLen; i++) {
    std::cout << itsParamValues[i] << " ";
  }
  std::cout << std::endl;
}
