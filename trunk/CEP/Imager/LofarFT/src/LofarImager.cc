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
#include <LofarFT/LofarImager.h>
#include <LofarFT/LofarVisResampler.h>
#include <casa/Utilities/CountedPtr.h>
#include <synthesis/MeasurementComponents/SimpleComponentFTMachine.h>
#include <msvis/MSVis/VisSet.h>
#include <LofarFT/LofarCubeSkyEquation.h>

#include <tables/Tables/TableIter.h>
#include <assert.h>

using namespace casa;

namespace LOFAR
{
  // @brief Imager for LOFAR data correcting for DD effects

  LofarImager::LofarImager (MeasurementSet& ms, const Record& parameters)
    : Imager(ms),
      itsParameters (parameters)
  {
    cout << itsParameters<<endl;
  }

  LofarImager::~LofarImager()
  {}

  Bool LofarImager::createFTMachine()
  {
    CountedPtr<VisibilityResamplerBase> visResampler;
    Bool useDoublePrecGrid = False;
    itsMachine = new LofarFTMachine(cache_p/2, tile_p,
                                    visResampler, gridfunction_p,
                                    *ms_p, wprojPlanes_p, mLocation_p,
                                    padding_p, false, useDoublePrecGrid,
                                    itsParameters.asDouble("wmax"));
    ft_p  = itsMachine;
    cft_p = new SimpleComponentFTMachine();

    //setClarkCleanImageSkyModel();

    // Determine nr of baselines and time interval.
    TableIterator iter(*ms_p, "TIME", TableIterator::Ascending,
                       TableIterator::NoSort);
    uInt nrowPerTime = iter.table().nrow();
    double interval  = ROScalarColumn<double>(iter.table(),"INTERVAL")(0);
    Int ntime = itsParameters.asDouble("timewindow") / interval;
    Int nrowBlock = nrowPerTime * max(1,ntime);
    // Set row blocking in VisIter.
    rvi_p->setRowBlocking (nrowBlock);
/*    os << LogIO::NORMAL
       << "vi.setRowBlocking(" << nrowBlock << ")"
       << LogIO::POST;*/
    return True;
  }

  void LofarImager::setSkyEquation()
  {
    se_p = new LofarCubeSkyEquation(*sm_p, *rvi_p, *ft_p, *cft_p,
                                    !useModelCol_p);
    return;
  }

//   void LofarImager::setSkyEquation()
//   {
//     vs_p = new VisSet(*rvi_p);
//     se_p = new SkyEquation(*sm_p, *vs_p, *ft_p, *cft_p, !useModelCol_p);
//     return;
//   }

} //# end namespace

