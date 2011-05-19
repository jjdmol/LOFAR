//# LofarImager.h: Imager for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2011
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
//# $Id$
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#include <lofar_config.h>
#include <CasaGridder/LofarImager.h>
#include <CasaGridder/LofarFTMachine.h>
#include <CasaGridder/LOFARConvolutionFunction.h>

#include <casa/Utilities/CountedPtr.h>

using namespace casa;

namespace LOFAR
{
  // @brief Imager for LOFAR data correcting for DD effects

  LofarImager::LofarImager (MeasurementSet& ms, const Record& parameters)
    : Imager(ms),
      itsParameters (parameters)
  {
  }

  LofarImager::~LofarImager()
  {}

  Bool LofarImager::createFTMachine()
  {
    // todo use nwplanes instead of 200

 (new LOFARConvolutionFunction
       (*ms_p, 200, itsParameters.asuInt("AtermTimeInterval")));
    // According to Sanjay Imager cannot fully handle double precision grid yet.
    CountedPtr<VisibilityResamplerBase> visResampler = new AWVisResampler();
    CountedPtr<CFCache> cfcache=new CFCache();
    cfcache->setCacheDir(cfCacheDirName_p.data());
    cfcache->initCache();
    // Pointing offsets no; beam correction yes.
    doPointing = False;
    doPBCorr   = True;
    LofarFTMachine* lfm = new LofarFTMachine(wprojPlanes_p, cache_p/2,
                                             cfcache, convFunc,
                                             visResampler,
                                             tile_p, pbLimit_p, True);
    ft_p = lfm;
    lfm->setObservatoryLocation(mLocation_p);
    // No parallactic angle stepping.
    paStep_p = 0;
    Quantity paInc(paStep_p,"deg");
    lfm->setPAIncrement(paInc);
    return True;
  }

} //# end namespace
