//# MeqJonesSum.cc: A sum of MeqJonesExpr results
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqJonesSum.h>
#include <BBS3/MNS/MeqJonesResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>

namespace LOFAR {

  MeqJonesSum::MeqJonesSum (const std::vector<MeqJonesExpr>& expr)
    : itsExpr (expr)
  {}

  MeqJonesSum::~MeqJonesSum()
  {}

  MeqJonesResult MeqJonesSum::getResult (const MeqRequest& request)
  {
    PERFPROFILE(__PRETTY_FUNCTION__);

    int nx = request.nx();
    int ny = request.ny();
    MeqJonesResult res(request.nspid());
    MeqResult& res11 = res.result11();
    MeqResult& res12 = res.result12();
    MeqResult& res21 = res.result21();
    MeqResult& res22 = res.result22();
    // Create arrays in the result elements and set to 0.
    res11.setValue (MeqMatrix(makedcomplex(0,0), nx, ny));
    res12.setValue (MeqMatrix(makedcomplex(0,0), nx, ny));
    res21.setValue (MeqMatrix(makedcomplex(0,0), nx, ny));
    res22.setValue (MeqMatrix(makedcomplex(0,0), nx, ny));
    // Loop through all expressions.
    for (std::vector<MeqJonesExpr>::iterator iter=itsExpr.begin();
	 iter != itsExpr.end();
	 ++iter) {
      MeqJonesResult er = iter->getResult (request);
      const MeqResult& er11 = er.getResult11();
      const MeqResult& er12 = er.getResult12();
      const MeqResult& er21 = er.getResult21();
      const MeqResult& er22 = er.getResult22();
      // First handle the perturbed values.
      for (int spinx=0; spinx<request.nspid(); ++spinx) {
	// If the value is already perturbed in the result, add the
	// perturbed value from the expression.
	if (res11.isDefined (spinx)) {
	  res11.getPerturbedValueRW(spinx) += er11.getPerturbedValue(spinx);
	} else if (er11.isDefined (spinx)) {
	  // Otherwise if expression is perturbed, it is the first one.
	  // So set perturbed in result to sum of main and perturbed.
	  res11.setPerturbedValue(spinx, res11.getValue() +
				         er11.getPerturbedValue(spinx));
	  res11.setPerturbation (spinx, er11.getPerturbation(spinx));
	}
	if (res12.isDefined (spinx)) {
	  res12.getPerturbedValueRW(spinx) += er12.getPerturbedValue(spinx);
	} else if (er12.isDefined (spinx)) {
	  res12.setPerturbedValue(spinx, res12.getValue() +
				         er12.getPerturbedValue(spinx));
	  res12.setPerturbation (spinx, er12.getPerturbation(spinx));
	}
	if (res21.isDefined (spinx)) {
	  res21.getPerturbedValueRW(spinx) += er21.getPerturbedValue(spinx);
	} else if (er21.isDefined (spinx)) {
	  res21.setPerturbedValue(spinx, res21.getValue() +
				         er21.getPerturbedValue(spinx));
	  res21.setPerturbation (spinx, er21.getPerturbation(spinx));
	}
	if (res22.isDefined (spinx)) {
	  res22.getPerturbedValueRW(spinx) += er22.getPerturbedValue(spinx);
	} else if (er22.isDefined (spinx)) {
	  res22.setPerturbedValue(spinx, res22.getValue() +
				         er22.getPerturbedValue(spinx));
	  res22.setPerturbation (spinx, er22.getPerturbation(spinx));
	}
      }
      // Now add to the main value.
      res11.getValueRW() += er11.getValue();
      res12.getValueRW() += er12.getValue();
      res21.getValueRW() += er21.getValue();
      res22.getValueRW() += er22.getValue();
    }
    return res;
  }

}
