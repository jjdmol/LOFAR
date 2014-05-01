//# OperationPredict.h: Concrete "predict" operation
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_LOFARFT_OPERATIONPREDICT_H
#define LOFAR_LOFARFT_OPERATIONPREDICT_H

// \file
// Concrete "predict" operation

#include <LofarFT/Operation.h>

namespace LOFAR {
namespace LofarFT {
  
    class OperationPredict : public virtual Operation
    {
    public:
      // Default constructor.
      OperationPredict(ParameterSet& parset);

      virtual void init();

      virtual void run();

      virtual void showHelp (ostream& os, const string& name);
    };

} //# namespace LofarFT
} //# namespace LOFAR

#endif
