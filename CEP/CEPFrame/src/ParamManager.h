//# ParamManager.h: Controls access to ParamHolders
//#
//# Copyright (C) 2000-2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$ 

#ifndef CEPFRAME_PARAMMANAGER_H
#define CEPFRAME_PARAMMANAGER_H

/*  #include <lofar_config.h> */

#include "CEPFrame/ParamHolder.h"
#include <Common/lofar_map.h>
#include "CEPFrame/CyclicOWBuffer.h"

namespace LOFAR
{

/**
  Class ParamManager controls access to a WorkHolders ParamHolders.
*/

class ParamManager
{
  // struct containing ParamHolder and its input and output buffer.
  struct ParameterBuffer
  {
  public:
    ParameterBuffer();
    ~ParameterBuffer();
    CyclicOWBuffer<ParamHolder*> inputBuf;
    ParamHolder*               param;
    ParamHolder*               outputBuf;
  };

public:
  /** The constructor 
  */
  ParamManager ();

  virtual ~ParamManager();

  void preprocess();

  void postprocess();
  
  // Get the ParamHolder object
  ParamHolder* getParamHolder(const string& name);
  
  // Add ParamHolder
  bool addParamHolder(ParamHolder* phptr, const string& name, 
                      bool isParamOwner=false);
  
  // Publish the parameter
  void publishParam(string paramName);

  // Get the last published value of the parameter
  void getLastValue(string paramName);

  // Wait for newly published value of the parameter
  bool getNewValue(string paramName);

  // Set the Step the Parameters belong to.
  void setStep(StepRep& step);

  // Disconnect named Parameter
  void disconnectParam(const string& name);

private:
  /// Copy constructor.
  ParamManager (const ParamManager&);

  map<string, ParameterBuffer*> itsParamBuffers;

};

}

#endif

