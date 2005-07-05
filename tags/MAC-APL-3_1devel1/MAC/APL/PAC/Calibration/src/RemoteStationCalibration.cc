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

#include <Common/LofarLogger.h>
#include <blitz/array.h>
#include <complex>
#include <math.h>

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
  const DipoleModel&   dipolemodel = getDipoleModel();    // get dipole model
  const SourceCatalog& sources     = getSourceCatalog();  // get sky model

  cout << "calibrate: spectral window name=" << spw.getName() << endl;
  cout << "calibrate: subband width=" << spw.getSubbandWidth() << " Hz" << endl;
  cout << "calibrate: num_subbnads=" << spw.getNumSubbands() << endl;
  cout << "calibrate: subarray name=" << subarray.getName() << endl;
  cout << "calibrate: num_antennas=" << subarray.getNumAntennas() << endl;

  //find_rfi_free_channels();
  for (int sb = 0; sb < spw.getNumSubbands(); sb++) {
    Timestamp acmtime;
    const Array<complex<double>, 4> acm = acc.getACM(sb, acmtime);
    
    // since acmtime is currently returning 0, we set it to the current
    // time for debug purposes, i.e. these lines need to be removed in the
    // final version!
    struct timeval tv;
    acmtime.setNow();
    
    // check whether channel is RFI free

    // construct local sky model (LSM)
    acmtime.get(&tv);
    const vector<Source> LSM = make_local_sky_model(sources, tv.tv_sec);
    
    Array<double, 3> AntennaPos = subarray.getAntennaPos();
    
    cout << "calibrate: working on subband " << sb + 1 << " of "
	   << spw.getNumSubbands() << endl;
    double freq = sb * spw.getSubbandWidth() + spw.getSamplingFrequency() * (spw.getNyquistZone() - 1);
    // for testing purposes we overrule the calculation above
    freq = 30e6;
    
    Array<complex<double>, 2> R0(make_ref_acm(LSM, AntennaPos, dipolemodel, freq));
    // mark baselines of at least 40m
    Array<bool, 2> mask(set_restriction(AntennaPos, 40));
    
    cout << acm << endl;
    Array<complex<double>, 2> acm1pol(acm(Range::all(), Range::all(), 0, 0));
    cout << acm1pol << endl;
    Array<complex<double>, 2> alpha(computeAlpha(acm1pol, R0, mask));
    cout << alpha << endl;
    //compute_gains(acm, R0, pos, spw.getSubbandFreq(sb), Rtest, result);
    //compute_quality(Rtest, sb, result);
  }

  //interpolate_bad_subbands();
   
  result.setComplete(true); // when finished
}

const vector<Source> RemoteStationCalibration::make_local_sky_model(const SourceCatalog& catalog, double obstime)
{
  const std::vector<Source> skymodel = catalog.getCatalog();

  // obstime is in UTC seconds since Jan 1, 1970, 0h0m0s
  // This was Julian Day 2440587.5
  // number of seconds in a day: 86400
  double JulianDay = obstime / 86400 + 2440587.5;
  cout << "calibrate: time of observation in Julian Days: " << JulianDay - 2453500 << endl;

  // geographical location if station needed
  // currently location Dwingeloo is taken
  double
    geolon = 6 + 23.0/60 + 41.0/3600,
    geolat = 52 + 48.0/60 + 42.0/3600;

  // conversion from JD to Greenwich Star Time (GST) in seconds
  double
    TU = (JulianDay - 2451545) / 36525,
    GST = (JulianDay + 0.5)*86400 + 24110.54841 + 8640184.812866 * TU + 
          0.093104 * TU * TU + -6.2e-6 * TU * TU * TU;

  // conversion from GST to zenith projection in radians
  double
    alpha0 = ((GST + geolon * 240) / 240) * M_PI / 180,
    delta0 = geolat * M_PI / 180;

  // conversion from (ra, dec) to (l, m)
  // construct local sky model
  // Note that the ra and dec fields are currently misused for l and m
  std::vector<Source> local_skymodel;
  for (int idx = 0; skymodel.begin() + idx < skymodel.end(); idx++) {
    double
      alpha = (skymodel.begin() + idx)->getRA(),
      delta = (skymodel.begin() + idx)->getDEC(),
      el = asin(sin(delta) * sin(delta0) + cos(delta0) * cos(delta) * cos(alpha0 - alpha)),
      az = acos((sin(delta) - sin(el) * sin(delta0)) / (cos(el) * cos(delta0))) * ((sin(alpha0 - alpha) < 0) ? -1 : 1);

    if (el > 0)
      local_skymodel.push_back(Source((skymodel.begin() + idx)->getName(), -cos(el) * sin(az), cos(el) * cos(az), const_cast<blitz::Array<double, 2>&>((skymodel.begin() + idx)->getFluxes())));
  }

  return local_skymodel;
}

Array<complex<double>, 2> RemoteStationCalibration::make_ref_acm(const vector<Source>& LSM, Array<double, 3>& AntennaPos, const DipoleModel& dipolemodel, double freq)
{
  int
    nelem = AntennaPos.extent(firstDim),
    nsrc = LSM.size();

  // matrix to store the result
  Array<complex<double>, 2> res(nelem, nelem);

  Array<double, 1>
    xpos(AntennaPos(Range::all(), 0, 0)),
    ypos(AntennaPos(Range::all(), 0, 1)),
    zpos(AntennaPos(Range::all(), 0, 2));

  double k = 2 * M_PI * freq / 2.99792e8;

  // actual calculation of the reference ACM
  firstIndex it1;
  secondIndex it2;
  for (int idx = 0; idx < nsrc; idx++) {
    double
      l = (LSM.begin() + idx)->getRA(),
      m = (LSM.begin() + idx)->getDEC(),
      n = sqrt(1 - l * l - m * m);
    Array<complex<double>, 1>
      asrc(exp(-complex<double>(0, 1) * k * (l * xpos + m * ypos + n * zpos)));
    Array<double, 2> fluxdata((LSM.begin() + idx)->getFluxes());
    Array<double, 1>
      freqval(fluxdata(Range::all(), 0)),
      fluxval(fluxdata(Range::all(), 1));
    double flux = interp1d(freqval, fluxval, freq);

    Array<double, 4> dipoleRespons(abs(dipolemodel.getModel()));
    // Next lines contain meta information. It would be nice to put this in a
    // config file
    Array<double, 1> fgrid(10), lgrid(51), mgrid(51);
    fgrid = (it1 + 1) * 10e6;
    lgrid = it1 * 0.04 - 1;
    mgrid = it1 * 0.04 - 1;

    double
      att = interp3d(lgrid, mgrid, fgrid, dipoleRespons(0, Range::all(), Range::all(), Range::all()), l, m, freq);
    res += flux * att * asrc(it1) * conj(asrc(it2));
  }
  return res;
}

Array<bool, 2> RemoteStationCalibration::set_restriction(Array<double, 3>& AntennaPos, double minbaseline)
{
  Array<double, 1>
    xpos(AntennaPos(Range::all(), 0, 0)),
    ypos(AntennaPos(Range::all(), 0, 1));
  // zpos not needed, since the baseline restriction is taken to be a
  // baseline restriction with the phase center in the zenith

  firstIndex it1;
  secondIndex it2;

  Array<double, 2>
    u(xpos.size(), xpos.size()),
    v(xpos.size(), xpos.size());

  u = xpos(it2) - xpos(it1);
  v = ypos(it2) - ypos(it1);

  Array<bool, 2>
    mask(xpos.size(), xpos.size());
  mask = (sqrt(u * u + v * v) > minbaseline);
  return mask;
}

Array<complex<double>, 2> RemoteStationCalibration::computeAlpha(Array<complex<double>, 2>& acm, Array<complex<double>, 2>& R0, Array<bool, 2> restriction)
{
  int nelem = acm.extent(firstDim);
  Array<complex<double>, 2> alpha(nelem, nelem);
  for (int idx1 = 0; idx1 < nelem; idx1++) {
    for (int idx2 = 0; idx2 < nelem; idx2++) {
      int Nnonzero = 0;
      for (int idx3 = 0; idx3 < nelem; idx3++) {
	if (restriction(idx1, idx3) && restriction(idx2, idx3)) {
	  alpha(idx1, idx2) += (acm(idx1, idx3) * R0(idx2, idx3)) / (acm(idx2, idx3) * R0(idx1, idx3));
	  Nnonzero++;
	}
      }
      alpha(idx1, idx2) /= Nnonzero;
    }
  }
  return alpha;
}

Array<double, 2> RemoteStationCalibration::matmult(Array<double, 2> A, Array<double, 2> B)
{
  ASSERT(A.extent(secondDim) == B.extent(firstDim));
  firstIndex i;
  secondIndex j;
  thirdIndex k;
  Array<double, 2> res(A.extent(firstDim), B.extent(secondDim));
  res = sum(A(i, k) * B(k, j), k);
  return res;
}

double RemoteStationCalibration::interp1d(Array<double, 1> xval, Array<double, 1> yval, double xinterp)
{
  if (xinterp < xval(0))
    return yval(0);
  if (xinterp > xval(xval.size()-1))
    return yval(xval.size()-1);
  int idx = 0;
  while (xval(idx) < xinterp) idx++;
  return yval(idx-1) + (yval(idx) - yval(idx-1)) * (xinterp - xval(idx-1)) / (xval(idx) - xval(idx-1));
}

double RemoteStationCalibration::interp2d(Array<double, 1> xgrid, Array<double, 1> ygrid, Array<double, 2> dataval, double xinterp, double yinterp)
{
  if (xinterp < xgrid(0))
    return interp1d(ygrid, dataval(0, Range::all()), yinterp);
  if (xinterp > xgrid(xgrid.size()-1))
    return interp1d(ygrid, dataval(xgrid.size()-1, Range::all()), yinterp);
  if (yinterp < ygrid(0))
    return interp1d(xgrid, dataval(Range::all(), 0), xinterp);
  if (yinterp > ygrid(ygrid.size()-1))
    return interp1d(xgrid, dataval(Range::all(), ygrid.size()-1), xinterp);
  int xidx = 0;
  while (xgrid(xidx) < xinterp) xidx++;
  int yidx = 0;
  while (ygrid(yidx) < yinterp) yidx++;
  double
    xrel = (xinterp - xgrid(xidx-1)) / (xgrid(xidx) - xgrid(xidx-1)),
    yrel = (yinterp - ygrid(yidx-1)) / (ygrid(yidx) - ygrid(yidx-1));
  return (1 - xrel) * (1 - yrel) * dataval(xidx-1, yidx-1) + xrel * (1 - yrel) * dataval(xidx, yidx-1) + xrel * yrel * dataval(xidx, yidx) + (1 - xrel) * yrel * dataval(xidx-1, yidx);
}

double RemoteStationCalibration::interp3d(Array<double, 1> xgrid, Array<double, 1> ygrid, Array<double, 1> zgrid, Array<double, 3> dataval, double xinterp, double yinterp, double zinterp)
{
  if (zinterp < zgrid(0))
    return interp2d(xgrid, ygrid, dataval(Range::all(), Range::all(), 0), xinterp, yinterp);
  if (zinterp > zgrid(zgrid.size()-1))
    return interp2d(xgrid, ygrid, dataval(Range::all(), Range::all(), zgrid.size()-1), xinterp, yinterp);
  int zidx = 0;
  while (zgrid(zidx) < zinterp) zidx++;
  double zrel = (zinterp - zgrid(zidx-1)) / (zgrid(zidx) - zgrid(zidx-1));
  Array<double, 2> dataval2d(dataval(Range::all(), Range::all(), zidx-1) + (dataval(Range::all(), Range::all(), zidx) - dataval(Range::all(), Range::all(), zidx-1)) * zrel);
  return interp2d(xgrid, ygrid, dataval2d, xinterp, yinterp);
}
