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

#include "Source.h"
#include "DipoleModel.h"
#include "RemoteStationCalibration.h"

#include <Common/LofarLogger.h>
#include <blitz/array.h>
#include <complex>
#include <math.h>
#include "cpplapack.h"

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace blitz;
using namespace std;
//using namespace LOFAR::RSP_Protocol;
using namespace CPPL;

RemoteStationCalibration::RemoteStationCalibration(const Sources& sources, DipoleModels& dipolemodels)
  : CalibrationAlgorithm(sources, dipolemodels),
    logfile("CalLog.txt")
{
}

void RemoteStationCalibration::calibrate(const SubArray& subarray, const ACC& acc, AntennaGains& gains)
{
  //
  // BIG WARNING: The order of the axes in the acc array have changed.
  // The order now is: nsubbands x nantennas x nantennas x npol x npol array of complex doubles.
  //
  // This warning can be removed when the code in this file has been adapted to that change.
  //
  const SpectralWindow& spw = subarray.getSPW();         // get spectral window
  DipoleModels&   dipolemodels = getDipoleModels(); // get dipole models
  const Sources&        sources      = getSources();      // get sky model
  const DipoleModel*    dipolemodel  = dipolemodels.getByName("LBAntenna");

  if (!dipolemodel) {
    LOG_FATAL("Failed to load dipolemodel 'LBAntenna'");
    exit(EXIT_FAILURE);
  }

  LOG_INFO_STR("calibrate: spectral window name=" << spw.getName());
  LOG_INFO_STR("calibrate: subband width=" << spw.getSubbandWidth() << " Hz");
  LOG_INFO_STR("calibrate: num_subbnads=" << spw.getNumSubbands());
  LOG_INFO_STR("calibrate: subarray name=" << subarray.getName());
  LOG_INFO_STR("calibrate: num_antennas=" << subarray.getNumAntennas());

  // check for RFI free channels
  Array<bool, 1> isclean(issuitable(acc, spw.getNumSubbands()));

  int npolarizations = 1;
  for (int pol = 0; pol < npolarizations; pol++) {
    cout << "calibrate: working on polarization " << pol << " of "
	 << npolarizations << endl;
    for (int sb = 0; sb < spw.getNumSubbands(); sb++) {
      logfile << "sb = " << sb << endl;

      Timestamp acmtime;
      const Array<complex<double>, 2> acm = acc.getACM(sb, pol, pol, acmtime);

      // time of test observation since 1-1-1970
      acmtime = Timestamp(1111584323, 160000);

      if (isclean(sb)) {
	// construct local sky model (LSM)
	const vector<Source> LSM = make_local_sky_model(sources, acmtime);
    
	Array<double, 3> AntennaPos = subarray.getAntennaPos();
      
	cout << "calibrate: working on subband " << sb + 1 << " of "
	     << spw.getNumSubbands() << endl;
	double freq = sb * spw.getSubbandWidth() + spw.getSamplingFrequency() * (spw.getNyquistZone() - 1);

	logfile << "freq = " << freq << endl;

	Array<complex<double>, 2> R0(make_ref_acm(LSM, AntennaPos, *dipolemodel, freq));
	// mark baselines of at least 40m
	Array<bool, 2> mask(set_restriction(AntennaPos, 40));
    
	// estimate alpha = g * (1 ./ g)
	Array<complex<double>, 2> alpha(computeAlpha(acm, R0, mask));
	// extract g from alpha
	Array<complex<double>, 1> gain(computeGain(alpha, acm, R0, mask));
	ASSERT(gains.getGains().extent(firstDim) == gain.extent(firstDim));
	gains.getGains()(Range::all(), 0/*X-pol*/, sb) = gain;

	logfile << "gain = " << endl << gain << endl;
	cout << "gains[" << sb << "]=" << gain << endl;
	
      } else
	logfile << "Subband contains RFI and is not calibrated" << endl;
	cout << "calibrate: subband " << sb + 1 << " was not processed" << endl;
    }
  }
  //interpolate_bad_subbands();
   
  gains.setDone(true); // when finished
}

Array<bool, 1> RemoteStationCalibration::issuitable(const ACC& acc, int nsb)
{
  Timestamp acmtime;
  Array<complex<double>, 1> test(nsb);
  for (int idx = 0; idx < nsb; idx++) {
    Array<complex<double>, 2> acm = acc.getACM(idx, 0, 0, acmtime).copy();
    Array<complex<double>, 1> ac(acm.extent(firstDim));
    ac = acm(tensor::i, tensor::i);
    acm = acm(tensor::i, tensor::j) / sqrt(ac(tensor::i) * ac(tensor::j));
    Array<complex<double>, 2> prod(acm.extent(firstDim), acm.extent(firstDim));
    Array<complex<double>, 2> acmH(conj(acm.transpose(1, 0)));
    prod = matmultc(acm, acmH);
    Array<complex<double>, 1> diagprod(acm.extent(firstDim));
    diagprod = prod(tensor::i, tensor::i);
    test(idx) = sum(diagprod);
  }

  Array<bool, 1> isclean(nsb);
  isclean(0) = false;
  for (int idx = 1; idx < nsb - 1; idx++) {
    isclean(idx) = (abs(test(idx-1) - test(idx)) < 0.05) ||
                   (abs(test(idx) - test(idx+1)) < 0.05);
  }
  isclean(nsb-1) = false;
  return isclean;
}

const vector<Source> RemoteStationCalibration::make_local_sky_model(const Sources& sources, Timestamp& acmtime)
{
  double obstime = acmtime.sec() + acmtime.usec() / 1e6;
  const std::vector<Source> skymodel = sources.getSources();

  // obstime is in UTC seconds since Jan 1, 1970, 0h0m0s
  // This was Julian Day 2440587.5
  // number of seconds in a day: 86400
  double JulianDay = obstime / 86400 + 2440587.5;
  LOG_INFO_STR("calibrate: time of observation in Julian Days: " << JulianDay - 2453500);

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
      el = ::asin(::sin(delta) * ::sin(delta0) + ::cos(delta0) * ::cos(delta) * ::cos(alpha0 - alpha)),
      az = ::acos((::sin(delta) - ::sin(el) * ::sin(delta0)) / (::cos(el) * ::cos(delta0))) * ((::sin(alpha0 - alpha) < 0) ? -1 : 1);

    if (el > 0)
      local_skymodel.push_back(Source((skymodel.begin() + idx)->getName(), -::cos(el) * ::sin(az), ::cos(el) * ::cos(az), const_cast<blitz::Array<double, 2>&>((skymodel.begin() + idx)->getFluxes())));
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
  for (int idx = 0; idx < nsrc; idx++) {
    double
      l = (LSM.begin() + idx)->getRA(),
      m = (LSM.begin() + idx)->getDEC(),
      n = ::sqrt(1 - l * l - m * m);
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
    fgrid = (tensor::i + 1) * 10e6;
    lgrid = tensor::i * 0.04 - 1;
    mgrid = tensor::i * 0.04 - 1;

    double
      att = interp3d(lgrid, mgrid, fgrid, dipoleRespons(0, Range::all(), Range::all(), Range::all()), l, m, freq);
    res += flux * att * asrc(tensor::i) * conj(asrc(tensor::j));
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

  Array<double, 2>
    u(xpos.size(), xpos.size()),
    v(xpos.size(), xpos.size());

  u = xpos(tensor::j) - xpos(tensor::i);
  v = ypos(tensor::j) - ypos(tensor::i);

  Array<bool, 2>
    mask(xpos.size(), xpos.size());
  mask = (sqrt(u * u + v * v) > minbaseline);
  return mask;
}

Array<complex<double>, 2> RemoteStationCalibration::computeAlpha(const Array<complex<double>, 2>& acm, Array<complex<double>, 2>& R0, Array<bool, 2>& restriction)
{
  int nelem = acm.extent(firstDim);
  Array<complex<double>, 2> alpha(nelem, nelem);
  for (int idx1 = 0; idx1 < nelem; idx1++) {
    for (int idx2 = 0; idx2 < nelem; idx2++) {
      int Nnonzero = 0;
      for (int idx3 = 0; idx3 < nelem; idx3++) {
	if (restriction(idx1, idx3) && restriction(idx2, idx3)) {
	  alpha(idx1, idx2) += (acm(idx1, idx3) / R0(idx1, idx3)) / (acm(idx2, idx3) / R0(idx2, idx3));
	  Nnonzero++;
	}
      }
      alpha(idx1, idx2) /= Nnonzero;
    }
  }
  return alpha;
}

Array<complex<double>, 1> RemoteStationCalibration::computeGain(Array<complex<double>, 2>& alpha, const Array<complex<double>, 2>& acm, Array<complex<double>, 2>& R0, Array<bool, 2> restriction)
{
  int nelem = alpha.extent(firstDim);

  // blitz to Lapack conversion
  zgematrix alphaCPPL(nelem, nelem);
  for (int idx1 = 0; idx1 < nelem; idx1++)
    for (int idx2 = 0; idx2 < nelem; idx2++)
      alphaCPPL(idx1, idx2) = alpha(idx1, idx2);

  // extraction of the gains
  vector<complex<double> > eigenval;
  vector<zcovector> eigenvec;
  alphaCPPL.zgeev(eigenval, eigenvec);
  int maxidx = 0;
  for (int idx = 1; idx < nelem; idx++)
    if (abs(eigenval[maxidx]) < abs(eigenval[idx])) maxidx = idx;
  Array<complex<double>, 1> gain(nelem);
  for (int idx = 0; idx < nelem; idx++)
    gain(idx) = eigenvec[maxidx](idx);

  // normalization
  Array<complex<double>, 2> Rtest(nelem, nelem);

  Rtest = gain(tensor::i) * conj(gain(tensor::j)) * R0(tensor::i, tensor::j);
  int Nnonzero = 0;
  double total = 0;
  for (int idx1 = 0; idx1 < nelem; idx1++) {
    for (int idx2 = 0; idx2 < nelem; idx2++) {
      if (restriction(idx1, idx2)) {
	total += abs(Rtest(idx1, idx2) / acm(idx1, idx2));
	Nnonzero++;
      }
    }
  }
  double norm = 1.0 / ::sqrt(total / static_cast<double>(Nnonzero));
  gain = norm * gain / (gain(0) / abs(gain(0)));
  return gain;
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

Array<complex<double>, 2> RemoteStationCalibration::matmultc(Array<complex<double>, 2> A, Array<complex<double>, 2> B)
{
  ASSERT(A.extent(secondDim) == B.extent(firstDim));
  firstIndex i;
  secondIndex j;
  thirdIndex k;
  Array<complex<double>, 2> res(A.extent(firstDim), B.extent(secondDim));
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
