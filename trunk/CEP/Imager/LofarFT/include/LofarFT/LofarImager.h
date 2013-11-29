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

#ifndef LOFAR_LOFARFT_LOFARIMAGER_H
#define LOFAR_LOFARFT_LOFARIMAGER_H

#include <LofarFT/LofarFTMachine.h>
#include <LofarFT/LofarFTMachineOld.h>
#include <synthesis/MeasurementEquations/Imager.h>
#include <casa/Containers/Record.h>

namespace LOFAR
{
  // @brief Imager for LOFAR data correcting for DD effects

  class LofarImager : public casa::Imager
  {
  public:

    // Construct from the Imager object.
    explicit LofarImager (casa::MeasurementSet&,
                          const casa::Record& parameters);

    virtual ~LofarImager();

    // Create the LofarFTMachine and fill ft_p in the parent.
    virtual casa::Bool createFTMachine();

    virtual void setSkyEquation();

    // Get the average primary beam.
    const Matrix<Float>& getAveragePB() const
    { return itsMachine ? itsMachine->getAveragePB() : itsMachineOld->getAveragePB(); }

    // Get the spheroidal cut.
    const Matrix<Float>& getSpheroidCut() const
    { return itsMachine ? itsMachine->getSpheroidCut() : itsMachineOld->getSpheroidCut(); }

    // Show the relative timings of the various steps.
    void showTimings (std::ostream&, double duration) const;

  private:
    //# Data members.
    casa::Record       itsParameters;
    LofarFTMachine*    itsMachine;
    LofarFTMachineOld* itsMachineOld;
    vector<Array<Complex> > itsGridsParallel;
    vector<Array<DComplex> > itsGridsParallel2;
   };

} //# end namespace

#endif
