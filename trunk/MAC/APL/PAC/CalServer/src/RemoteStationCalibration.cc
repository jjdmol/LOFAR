//#  -*- mode: c++ -*-
//#  RemoteStationCalibration.cc: class implementation of RemoteStationCalibration
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include "SourceCatalog.h"
#include "DipoleModel.h"
#include "RemoteStationCalibration.h"

#include <blitz/array.h>
#include <complex>

using namespace CAL;
using namespace blitz;
using namespace std;
using namespace LOFAR::RSP_Protocol;

RemoteStationCalibration::RemoteStationCalibration(const SourceCatalog& catalog, const DipoleModel& dipolemodel)
  : CalibrationAlgorithm(catalog, dipolemodel)
{
}

void RemoteStationCalibration::calibrate(const SubArray& subarray, const ACC& acc, CalibrationResult& result)
{
  const SpectralWindow&   spw = subarray.getSPW();        // get spectral window
  const Array<double, 3>& pos = subarray.getAntennaPos(); // get antenna positions

  const DipoleModel&   dipolemodel = getDipoleModel();    // get dipole model
  const SourceCatalog& sources     = getSourceCatalog();  // get sky model

  cout << "calibrate: spectral window name=" << spw.getName() << endl;
  cout << "calibrate: subband width=" << spw.getSubbandWidth() << " Hz" << endl;
  cout << "calibrate: num_subbnads=" << spw.getNumSubbands() << endl;
  cout << "calibrate: subarray name=" << subarray.getName() << endl;
  cout << "calibrate: num_antennas=" << subarray.getNumAntennas() << endl;

  //find_rfi_free_channels();
  for (int sb = 0; sb < spw.getNumSubbands(); sb++)
    {
      Timestamp acmtime;
      const Array<complex<double>, 4> acm = acc.getACM(sb, acmtime);

      //localsource = make_local_sky_model(sources, acmtime);
      //R0 = make_model_ACM(localsources, dipolemodel, );
      //compute_gains(acm, R0, pos, spw.getSubbandFreq(sb), Rtest, result);
      //compute_quality(Rtest, sb, result);
    }
  //interpolate_bad_subbands();
   
  result.setComplete(true); // when finished
}


