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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "Source.h"
#include "DipoleModel.h"
#include "RemoteStationCalibration.h"

#include <blitz/array.h>
#include <complex>
#include <cmath>
#include <time.h>
#include <cpplapack.h>
#include "BlitzMath.h"

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace CPPL;
using namespace AMC;
using namespace BLITZ_MATH;


// fileformat for writing matrix or vector's to a fileformat
// 0 = do nothing
// 1 = ascii
// 2 = binair
int fileFormat = 1;
clock_t c_start1, c_start2;

RemoteStationCalibration::RemoteStationCalibration(Sources& sources, DipoleModels& dipolemodels, AMC::Converter& converter)
  : CalibrationAlgorithm(sources, dipolemodels, converter),
    logfile("CalLog.txt")
{
  itsBaseLineRestriction = 4.0; // baseline restriction in wavelengths
  itsMaxBaseLine = 20.0;  // max base line in meters
  itsSpeedOfLight = 2.99792e8;
  
}
//---------------------------------------------------------------------------------------------------------------------

double getClock()
{
  return ((double)(clock() - c_start1) / CLOCKS_PER_SEC);
}

void RemoteStationCalibration::calibrate(const SubArray& subarray, ACC& acc, AntennaGains& gains)
{
  //
  // BIG WARNING: The order of the axes in the acc array have changed.
  // The order now is: nsubbands x npol x npol x nantennas x nantennas array of complex doubles.
  //
  // This warning can be removed when the code in this file has been adapted to that change.
  //
  double start_freq = 10e6;
  double stop_freq = 90e6;
  
  const SpectralWindow& spw = subarray.getSPW();          // get spectral window
  DipoleModels&         dipolemodels = getDipoleModels(); // get dipole models
  const Sources&        sources      = getSources();      // get sky model
  const DipoleModel*    dipolemodel  = dipolemodels.getByName("LBAntenna");
  const Array<double, 1>      geoLoc( subarray.getGeoLoc() );
  const Array<double, 3>      antennaPos( subarray.getAntennaPos() );
  
  time_t obstime;
  struct tm* timeinfo;
  
  time(&obstime);
  timeinfo = gmtime(&obstime);
  logfile << "filling timeinfo" << endl;
  timeinfo->tm_year = (2008 - 1900);
  timeinfo->tm_mon = (1 - 1);
  timeinfo->tm_mday = 18;
  timeinfo->tm_hour = 8;
  timeinfo->tm_min = 32;
  timeinfo->tm_sec = 29; 
  logfile << "setting obstime " << asctime(timeinfo) << endl;
  obstime = mktime( timeinfo );
  logfile << "setting obstime done " << obstime << endl;   
  
  if (!dipolemodel) {
    LOG_FATAL("Failed to load dipolemodel 'LBAntenna'");
    exit(EXIT_FAILURE);
  }

  LOG_INFO_STR("calibrate: spectral window name=" << spw.getName());
  LOG_INFO_STR("calibrate: subband width=" << spw.getSubbandWidth() << " Hz");
  LOG_INFO_STR("calibrate: num_subbands=" << spw.getNumSubbands());
  LOG_INFO_STR("calibrate: subarray name=" << subarray.getName());
  LOG_INFO_STR("calibrate: num_antennas=" << subarray.getNumAntennas());

    
  int npolarizations = 2;
  c_start1 = clock();
  for (int pol = 0; pol < npolarizations; pol++) {
    logfile << "calibrate: working on polarization " << pol << " of " << npolarizations << endl;
    
    blitz::Array<bool, 1> isClean(detectRFI(acc, pol, spw.getSubbandWidth()) );
    logfile << "detectRFI done" << " time=" << getClock()  <<  endl;
    
    blitz::Array<bool, 1> isInBand(setPassBand(start_freq, stop_freq, spw) );
    logfile << "setPassBand done" << " time=" << getClock()  <<  endl;
    
    
    blitz::Array<bool, 1> isSelected( where(isClean && isInBand, true, false));
    logfile << "isSelected done" << " time=" << getClock()  <<  endl;
    //bmToFile(isSelected,"isSelected",fileFormat);  
   
    blitz::Array<bool, 1> isGood(detectBadElements(acc, pol, isSelected)); 
    logfile << "detectBadElements done" << " time=" << getClock()  << endl;
     
    for (int sb = 0; sb < spw.getNumSubbands(); sb++) {
      logfile << "sb = " << sb << " time=" << getClock() << endl;

      // process only if sb = clean and in band
      if (isSelected(sb)) {
        //logfile << "calibrate: working on subband " << sb + 1 << " of " << spw.getNumSubbands() << endl;
        
        double freq = sb * spw.getSubbandWidth() + spw.getSamplingFrequency() * (spw.getNyquistZone() - 1);
	      logfile << "freq = " << freq << endl;
        
        Timestamp acmtime;
        Array<complex<double>, 2> acm = acc.getACM(sb, pol, pol, acmtime);
        
        //obstime += sb;
        logfile << "obstime=" << obstime+sb << endl;
        acmtime = Timestamp(obstime+sb, 0);
        
        // construct local sky model (LSM)
	      vector<Source> LSM = make_local_sky_model(sources, geoLoc, acmtime);
        logfile << "LSM done" << " time=" << getClock() << endl;
        
        // geef alleen geldige elementen door
	      // antennePos, acm
	      int nelem = acm.extent(firstDim);
	      int gelem = sum(isGood);  
        Array<double, 3> goodAntPos(gelem,antennaPos.extent(secondDim),antennaPos.extent(thirdDim));
        Array<complex<double>, 2> goodAcm(gelem,gelem);
        int id1 = 0;
        for (int idx1 = 0; idx1 < nelem; idx1++) {
          if (isGood(idx1)) {
            goodAntPos(id1,Range::all(),Range::all()) = antennaPos(idx1,Range::all(),Range::all());
            int id2 = 0;
            for (int idx2 = 0; idx2 < nelem; idx2++) {
              if (isGood(idx2)) {
                goodAcm(id1,id2) = acm(idx1,idx2);
                id2++;   
              }
            }
            id1++;
          }  
        } 	      
	      
        Array<bool, 2> restriction( setRestriction(goodAntPos, freq) );
        logfile << "restriction done" << " time=" << getClock() << endl;
	      
	      Array<complex<double>, 2> asrc(computeA(LSM, goodAntPos, freq));
	      logfile << "asrc done" << " time=" << getClock() << endl;
	     
 	      Array<double, 1> flux( computeFlux(goodAcm, asrc, restriction));
      	logfile << "flux done" << " time=" << getClock() << endl;      
	      
	      int maxIter = 5;
	      double diffStop = 0.001;
	      Array<complex<double>, 1> gain( computeGain(goodAcm, asrc, flux,  restriction, maxIter, diffStop));
        logfile << "gain done" << " time=" << getClock() << endl;
        
        Array<std::complex<double>, 1> cal( conj(1.0 / gain));
        //bmToFile(cal,"cal",fileFormat);
        
        // checkCal() ??
        //Array<complex<double>, 1> cal0( checkCal(cal, isGood, spw.getNumSubbands(), goodAcm.rows()));
        
        // convert computed gain to full array, and set bad elemts to zero
        int ngains = gains.getGains().extent(firstDim);
        Array<complex<double>, 1> gain_full(ngains);
        gain_full = 0.0;
        id1 = 0;
        for (int idx1 = 0;idx1 < ngains; idx1++) {
          if (isGood(idx1)) { 
            gain_full(idx1) = gain(id1);
            id1++;
          }
        }
                
        ASSERT(gains.getGains().extent(firstDim) == gain_full.extent(firstDim));
	      gains.getGains()(Range::all(), pol, sb) = gain_full;
	      
	      logfile << "gains[" << sb << "]=" << gain_full << endl;
	
      } else {
	      //logfile << "calibrate: subband " << sb+1 << " contains RFI and is not calibrated" << endl;
      }
    }
  }
  //interpolate_bad_subbands();
   
  gains.setDone(true); // when finished
}
//---------------------------------------------------------------------------------------------------------------------

// detects RFI on each channel
// returns true if clean
//  
blitz::Array<bool,1> RemoteStationCalibration::detectRFI(ACC& acc, int pol, double N) 
{
  // TODO put in conf file
  double rfiThresholdFactor = 10.0;
  
  int nelem = acc.getNAntennas();
  int nch = acc.getNSubbands();
  Timestamp acmtime;
  
  blitz::Array<double,1> teststat(nch);
    
  teststat = 0;
  logfile << "nelem=" << nelem << "   N=" << N << endl;
  double threshold = sqrt(2.0 * nelem / N);
  logfile << "threshold=" << threshold << endl;
  
  for (int ch = 0; ch < nch; ch++) {
    blitz::Array<std::complex<double>,2> Rhat(acc.getACM(ch, pol, pol, acmtime));
    blitz::Array<std::complex<double>,1> diagRhat( bmDiag(Rhat) );
    blitz::Array<std::complex<double>,2> Rwhite(Rhat(tensor::i, tensor::j) / sqrt( diagRhat(tensor::i) * diagRhat(tensor::j)));
    
    teststat(ch) = pow2( bmNorm(Rwhite, fro));
  } 
  //bmToFile(teststat,"teststat",fileFormat);
  
  blitz::Array<double,1> teststatL(nch);
  teststatL = 0;
  teststatL(Range(0,nch-2)) = teststat(Range(1,nch-1));
 
  blitz::Array<double,1> teststatR(nch);
  teststatR = 0;
  teststatR(Range(1,nch-1)) = teststat(Range(0,nch-2)); 
      
  blitz::Array<double,2> absLR(nch,2);
  absLR(Range::all(),0) = abs(teststat(Range::all()) - teststatL(Range::all()));
  absLR(Range::all(),1) = abs(teststat(Range::all()) - teststatR(Range::all())); 
  blitz::Array<double,1> maxLR( max(absLR(tensor::i,tensor::j),tensor::j) );
  //bmToFile(maxLR,"maxLR",fileFormat);
  
  blitz::Array<bool,1> clean( where(maxLR < (rfiThresholdFactor * threshold), true, false));
  //bmToFile(clean,"cleanBands",fileFormat);
  return (clean);  
}
//---------------------------------------------------------------------------------------------------------------------


blitz::Array<bool,1> RemoteStationCalibration::setPassBand(double start_freq, double stop_freq , const SpectralWindow& spw) 
{
  double freq;
  blitz::Array<bool,1> inpassband(spw.getNumSubbands());
  
  inpassband = true;
  for (int sb = 0; sb < spw.getNumSubbands(); sb++) {
    freq = sb * spw.getSubbandWidth() + spw.getSamplingFrequency() * (spw.getNyquistZone() - 1);
    if ((freq < start_freq) || (freq > stop_freq)) {
      inpassband(sb) = false;
    }
  }
  //bmToFile(inpassband,"inPassBand",fileFormat);
  return(inpassband);  
}
//---------------------------------------------------------------------------------------------------------------------

blitz::Array<bool,1> RemoteStationCalibration::detectBadElements(ACC& acc, int pol, blitz::Array<bool, 1> selected )
{
  int nelem = acc.getNAntennas();
  int nch = acc.getNSubbands();
  int nsel = sum(selected);
  logfile << "nsel=" << nsel << endl;
  blitz::Array<double,2> ac(nelem,nsel);
  Timestamp acmtime;
  
  int sel = 0;
  for (int ch = 0; ch < nch; ch++) {
    if (selected(ch) == true) {
      ac(Range::all(),sel) = abs( bmDiag(acc.getACM(ch, pol, pol, acmtime).copy()));
      sel++;  
    }
  }
  
  blitz::Array<double,2> med2(nelem,nsel);
  blitz::Array<double,1> med1(bmMedian(ac));
  med2 = med1(tensor::j);   
  blitz::Array<double,2> diff( abs(ac - med2) );
  
  blitz::Array<double,2> st2(nelem,nsel);
  blitz::Array<double,1> st1(bmStd(ac));
  st2 = st1(tensor::j);     
  blitz::Array<double,2> threshold( 5 * st2);
  
  blitz::Array<int,2> inband( where(diff < threshold, 1, 0) );
  blitz::Array<int,1> sum_ch( sum(inband(tensor::i,tensor::j),tensor::j) );
  blitz::Array<bool,1> good( where(sum_ch == nsel, true, false));
  
  //bmToFile(good,"goodElements",fileFormat);  
  return (good);
}
//---------------------------------------------------------------------------------------------------------------------


vector<Source> RemoteStationCalibration::make_local_sky_model(
                       const Sources& sources, 
                       const Array<double, 1>& geoloc, 
                       Timestamp& acmtime )
{
  double obstime = acmtime.sec() + acmtime.usec() / 1e6;
  logfile << "obstime=" << obstime << endl; 
  
  std::vector<Source> skymodel = sources.getSources();
#if 0
  AMC::Converter& converter    = getConverter(); // can be used for coordinate conversions
#endif

  double mjd = 0.0, mjdfraction = 0.0;
  acmtime.convertToMJD(mjd, mjdfraction);

  // obstime is in UTC seconds since Jan 1, 1970, 0h0m0s
  // This was Julian Day 2440587.5
  // number of seconds in a day: 86400
  double JulianDay = obstime / 86400 + 2440587.5;
  logfile << "JulianDay=" << JulianDay << endl;
  
  LOG_INFO_STR("calibrate: time of observation in Julian Days: " << JulianDay - 2453500);

  // geographical location of station needed
  //double geolon = geoloc(0);   
  //double geolat = geoloc(1);  
  
  // used fixed values to compare with MatLab code
  double geolon = 6 + 52.0/60 + 3.17/3600;
  double geolat = 52 + 54.0/60 + 42.6/3600;
  
  logfile << "geolon=" << geolon << logfile << " ,  geolat=" << geolat << endl;

  // conversion from JD to Greenwich Star Time (GST) in seconds
  double
    TU = (JulianDay - 2451545) / 36525,
    GST = (JulianDay + 0.5)*86400 + 24110.54841 + 8640184.812866 * TU + 
          0.093104 * TU * TU + -6.2e-6 * TU * TU * TU;
  //logfile << "GST=" << GST << endl;

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

#if 0
    // it seems that result is not used!
    SkyCoord result = converter.j2000ToAzel(SkyCoord((skymodel.begin() + idx)->getRA(), (skymodel.begin() + idx)->getDEC()),
					    EarthCoord(geolon, geolat), TimeCoord(mjd, mjdfraction));
#endif

    if (el > 0)
      local_skymodel.push_back(Source((skymodel.begin() + idx)->getName(), -::cos(el) * ::sin(az), ::cos(el) * ::cos(az), const_cast<blitz::Array<double, 2>&>((skymodel.begin() + idx)->getFluxes())));
  
  /*
  logfile << "local_skymodel-" << idx << "=" << (skymodel.begin() + idx)->getName() << ", "  
                                             << (-::cos(el) * ::sin(az)) << ", "
                                             << (::cos(el) * ::cos(az)) << ", "
                                             << const_cast<blitz::Array<double, 2>&>((skymodel.begin() + idx)->getFluxes()) << endl;
  */
  }
  return (local_skymodel);
}
//---------------------------------------------------------------------------------------------------------------------

Array<complex<double>, 2> RemoteStationCalibration::computeA(
                            vector<Source>& LSM,
                            const Array<double, 3>& AntennaPos,
                            double freq )
{
  int nelem = AntennaPos.extent(firstDim);
  int nsrc = LSM.size();
  
  
  Array<complex<double>, 2> asrc(nelem, nsrc);
  asrc = 0.0;
    
  Array<double, 1> xpos(AntennaPos(Range::all(), 0, 0));
  Array<double, 1> ypos(AntennaPos(Range::all(), 0, 1));
  //logfile << "freq=" << freq << endl;
  //logfile << "xpos=" << xpos << endl;
  //logfile << "ypos=" << ypos << endl;
      
  double k = 2 * M_PI * freq / itsSpeedOfLight;
  
  for (int idx = 0; idx < nsrc; idx++) {
    double l = (LSM.begin() + idx)->getRA();
    double m = (LSM.begin() + idx)->getDEC();
    //logfile << "k=" << k << endl;
    //logfile << "l=" << l << endl;
    //logfile << "m=" << m << endl;
    asrc(Range::all(),idx) = exp(-complex<double>(0, 1) * k * (l * xpos + m * ypos));
   }
   //bmToFile(asrc,"asrc",fileFormat);
   return (asrc);
}
//---------------------------------------------------------------------------------------------------------------------

blitz::Array<bool, 2> RemoteStationCalibration::setRestriction(
                        const Array<double, 3>& AntennaPos, 
                        double freq )
{
  // three dimensions, Nantennas x Npol x 3 (x,y,z)
  Array<double, 1> xpos( AntennaPos(Range::all(), 0, 0) );
  Array<double, 1> ypos( AntennaPos(Range::all(), 0, 1) );
  // zpos not needed, since the baseline restriction is taken to be a
  // baseline restriction with the phase center in the zenith

  Array<double, 2> u( xpos(tensor::j) - xpos(tensor::i) );
  Array<double, 2> v( ypos(tensor::j) - ypos(tensor::i) );
  Array<double, 2> uvdist( sqrt(u * u + v * v) ); 
  
  double minBaseLine = min( (itsBaseLineRestriction * (itsSpeedOfLight / freq)), itsMaxBaseLine); 
  
  blitz::Array<bool, 2> restriction( where(uvdist < minBaseLine, true, false));
  
  // diagonal is always 1
  for (int idx1 = 0; idx1 < uvdist.rows(); idx1++) {
    restriction(idx1, idx1) = 1;
  }
  //bmToFile(restriction,"restriction",fileFormat);
  return (restriction);
}
//---------------------------------------------------------------------------------------------------------------------


void RemoteStationCalibration::setSelection(
       blitz::Array<double, 2>& Isel,
       blitz::Array<double, 2>& pinvIsel,
       blitz::Array<bool, 2>& restriction,
       int nelem )
{
  int dim1, dim2;
  blitz::Array<double, 2> isDiag(nelem*nelem, nelem);
  blitz::Array<double, 2> eye( bmEye<double>(nelem));  
  isDiag = bmKhatrirao( eye, eye);
  //bmToFile(isDiag,"isDiag",fileFormat);
  dim1 = isDiag.extent(firstDim);
  dim2 = isDiag.extent(secondDim);
  
  blitz::Array<int, 2> sel1(nelem, nelem);
  blitz::Array<int, 1> sel2(nelem*nelem);
  blitz::Array<int, 2> sel3(nelem*nelem, nelem*nelem);
  int selectedPoints;
  int idx;
  
  sel1 = where( bmTriu(restriction, 1) == true, 1, 0);
  sel2 = bmMatrixToVector(sel1);
  sel3 = bmDiag(sel2);
  selectedPoints = sum(sel2);
  //logfile << "Selected points Re=" << selectedPoints << endl;
  
  dim2 += selectedPoints;
  blitz::Array<double, 2> seltriu(nelem*nelem, selectedPoints);
  idx = 0;
  for (int idx1 = 0; idx1 < selectedPoints; idx1++) {
    while (sel2(idx) == 0) { idx++; }
    seltriu(Range::all(), idx1) = cast<double>(sel3(Range::all(),idx));
    idx++;  
  }
  //bmToFile(seltriu,"seltriu",fileFormat);
  
  sel1 = where(bmTril(restriction, -1) == true, 1, 0);
  sel2 = bmMatrixToVector(sel1);
  sel3 = bmDiag(sel2);
  
  selectedPoints = sum(sel2);
  dim2 += selectedPoints;
  //logfile << "Selected points Im=" << selectedPoints << endl;
  
  blitz::Array<double, 2> seltril(nelem*nelem, selectedPoints);
  idx = 0;
  for (int idx1 = 0; idx1 < selectedPoints; idx1++) {
    while (sel2(idx) == 0) { idx++; }
    seltril(Range::all(), idx1) = cast<double>(sel3(Range::all(),idx));
    idx++;  
  }
  //bmToFile(seltril,"seltril",fileFormat);
  
  blitz::Array<double, 2> isRe(seltriu + seltril);
  blitz::Array<double, 2> isIm(seltriu - seltril);
  //bmToFile(isRe,"isRe",fileFormat);
  //bmToFile(isIm,"isIm",fileFormat);
  
  //logfile << "set Isel and pinvIsel" << endl;
  int t1,t2;
  Isel.resize(dim1, dim2); 
  t1 = 0; t2 = isDiag.cols() - 1;
  Isel(Range::all(), Range(t1, t2)) = isDiag(Range::all(), Range::all()).copy();
  t1 = isDiag.cols(); t2 = isDiag.cols() + isRe.cols() - 1;   
  Isel(Range::all(), Range(t1, t2)) = isRe(Range::all(), Range::all()).copy();
  t1 = isDiag.cols() + isRe.cols(); t2 = isDiag.cols() + isRe.cols() + isIm.cols() - 1;  
  Isel(Range::all(), Range(t1, t2)) = isIm(Range::all(), Range::all()).copy();    
  
  blitz::Array<double, 2> pinv_isel(dim1, dim2);
  isRe *= 0.5;
  isIm *= 0.5;
  t1 = 0; t2 = isDiag.cols() - 1; 
  pinv_isel(Range::all(), Range(t1, t2)) = isDiag(Range::all(), Range::all()).copy();
  t1 = isDiag.cols(); t2 = isDiag.cols() + isRe.cols() - 1;  
  pinv_isel(Range::all(), Range(t1, t2)) = isRe(Range::all(), Range::all()).copy();
  t1 = isDiag.cols() + isRe.cols(); t2 = isDiag.cols() + isRe.cols() + isIm.cols() - 1;   
  pinv_isel(Range::all(), Range(t1, t2)) = isIm(Range::all(), Range::all()).copy();
  pinvIsel.resize(dim2,dim1);
  pinvIsel = bmTrans(pinv_isel);
}
//---------------------------------------------------------------------------------------------------------------------

// setSelection_2 is used for the speed experiment
void RemoteStationCalibration::setSelection_2(
       blitz::Array<int,2>& posIsel,
       blitz::Array<double,2>& valIsel,
       blitz::Array<int,2>& posInvIsel,
       blitz::Array<double,2>& valInvIsel, 
       blitz::Array<bool, 2>& restriction,
       int nelem )
{
  int dim1, dim2;
  blitz::Array<int, 2> isDiag(nelem*nelem, nelem);
  blitz::Array<int, 2> eye( bmEye<int>(nelem));  
  isDiag = bmKhatrirao( eye, eye);
  dim1 = isDiag.extent(firstDim);
  dim2 = isDiag.extent(secondDim);
  
  blitz::Array<int, 2> sel1(where(bmTriu(restriction, 1) == true, 1, 0));
  blitz::Array<int, 1> sel2(bmMatrixToVector(sel1));
  blitz::Array<int, 2> sel3(bmDiag(sel2));
  int selectedPoints;
  int idx;
   
  selectedPoints = sum(sel2);
  dim2 += selectedPoints;
  logfile << "Selected points Re=" << selectedPoints << endl;
  
  
  blitz::Array<int, 2> seltriu(nelem*nelem, selectedPoints);
  idx = 0;
  for (int idx1 = 0; idx1 < selectedPoints; idx1++) {
    while (sel2(idx) == 0) { idx++; }
    seltriu(Range::all(), idx1) = sel3(Range::all(),idx);
  }
  
  sel1 = where(bmTril(restriction, -1) == true, 1, 0);
  sel2 = bmMatrixToVector(sel1);
  sel3 = bmDiag(sel2);
  
  selectedPoints = sum(sel2);
  dim2 += selectedPoints;
  logfile << "Selected points Im=" << selectedPoints << endl;
  
  blitz::Array<int, 2> seltril(nelem*nelem, selectedPoints);
  idx = 0;
  for (int idx1 = 0; idx1 < selectedPoints; idx1++) {
    while (sel2(idx) == 0) { idx++; }
    seltril(Range::all(), idx1) = sel3(Range::all(),idx);
  }
  
  blitz::Array<int, 2> isRe(seltriu + seltril);
  blitz::Array<int, 2> isIm(seltriu - seltril);

  // experiment, dont use the hole selection matrix
  // each row contain max 2 values
  // so use 2 matrix one with the position and one with the value
  posIsel.resize(dim1,3);
  valIsel.resize(dim1,3);
  posIsel = 0;
  valIsel = 0.0;
  for (int idx1 = 0; idx1 < dim1; idx1++) {
    int id = 0;
    for (int idx2 = 0; idx2 < isDiag.cols(); idx2++) {
      if (isDiag(idx1,idx2) != 0) {
        posIsel(idx1,id) = idx2;
        valIsel(idx1,id) = static_cast<double>(isDiag(idx1,idx2));
        id++;
      }
    }
    for (int idx2 = 0; idx2 < isRe.cols(); idx2++) {
       if (isRe(idx1,idx2) != 0) {
        posIsel(idx1,id) = isDiag.cols() + idx2;
        valIsel(idx1,id) = static_cast<double>(isRe(idx1,idx2));
        id++;
      }
    }
    for (int idx2 = 0; idx2 < isIm.cols(); idx2++) {
      if (isIm(idx1,idx2) != 0) {
        posIsel(idx1,id) = isDiag.cols() + isRe.cols() + idx2;
        valIsel(idx1,id) = static_cast<double>(isIm(idx1,idx2));
        id++;
      }
    }
  }
  logfile << "Isel done" << endl;

  posInvIsel.resize(dim2,10);
  valInvIsel.resize(dim2,10);
  posInvIsel = 0;
  valInvIsel = 0.0;
  idx = 0;
  int id;
  for (int idx1 = 0; idx1 < isDiag.rows(); idx1++) {
    id = 0;
    for (int idx2 = 0; idx2 < isDiag.cols(); idx2++) {
      if (isDiag(idx2,idx1) != 0) {
        posInvIsel(idx+idx1,id) = idx2;
        valInvIsel(idx+idx1,id) = static_cast<double>(isDiag(idx2,idx1));
        id++;
      }
    }
  }
  idx += isDiag.rows();
  for (int idx1 = 0; idx1 < isRe.rows(); idx1++) {
    id = 0;
    for (int idx2 = 0; idx2 < isRe.cols(); idx2++) {
       if (isRe(idx1,idx2) != 0) {
        posIsel(idx+idx1,id) = isDiag.cols() + idx2;
        valIsel(idx+idx1,id) = 0.5 * static_cast<double>(isRe(idx1,idx2));
        id++;
      }
    }
  }
  idx += isRe.rows();
  for (int idx1 = 0; idx1 < isIm.rows(); idx1++) {
    id = 0;
    for (int idx2 = 0; idx2 < isIm.cols(); idx2++) {
      if (isIm(idx1,idx2) != 0) {
        posIsel(idx+idx1,id) = isDiag.cols() + isRe.cols() + idx2;
        valIsel(idx+idx1,id) = 0.5 * static_cast<double>(isIm(idx1,idx2));
        id++;
      }
    }
  }
  logfile << "InvIsel done" << endl;
  
}
//---------------------------------------------------------------------------------------------------------------------


blitz::Array<std::complex<double>,1> RemoteStationCalibration::multIsel(
                            blitz::Array<int,2>& selpos,
                            blitz::Array<double,2>& selval,
                            blitz::Array<std::complex<double>,1>& x) {
  blitz::Array<std::complex<double>,1> res(x.rows());
  res = 0.0;
  for (int idx1 = 0; idx1 < x.rows(); idx1++) {
    for (int idx2 = 0; idx2 < selpos.cols(); idx2++) {
      res(idx1) += x(selpos(idx1,idx2)) * selval(idx1,idx2);
    }
  }
  return (res);
}
//---------------------------------------------------------------------------------------------------------------------

blitz::Array<double,1> RemoteStationCalibration::computeFlux(
                         Array<std::complex<double>, 2>& acm,
                         Array<std::complex<double>, 2>& asrc, 
                         Array<bool, 2>& restriction )
{
  Array<std::complex<double>, 2> tmp0( cast<std::complex<double> >( bmSqr( bmAbs( bmMult( bmTransH(asrc), asrc)))));
  Array<std::complex<double>, 2> tmp1( bmInv(tmp0));
    
  Array<std::complex<double>, 2> tmp2( bmTransH( bmKhatrirao( bmConj(asrc), asrc)));
  Array<std::complex<double>, 2> tmp3(acm.rows()*acm.cols(),1);
  tmp3(Range::all(),0) = where( bmMatrixToVector(restriction) == false, bmMatrixToVector(acm), 0.0);
  Array<double,2> tmp4( bmReal( bmMult(tmp1, tmp2, tmp3)));
  
  Array<double,1> flux( tmp4(Range::all(),0));
  
  flux = flux / flux(0);
  //flux = where(restriction, 0, flux);
  flux = where(flux < 0.0, 0.0 , flux);
  
  //bmToFile(flux,"flux",fileFormat);
  return (flux);
}
//---------------------------------------------------------------------------------------------------------------------


Array<std::complex<double>, 2> RemoteStationCalibration::computeAlpha(
                            Array<std::complex<double>, 2>& sigma,
	                          Array<std::complex<double>, 2>& acm,
	                          Array<std::complex<double>, 2>& asrc, 
	                          Array<bool, 2>& restriction )
{
  int nelem = acm.extent(firstDim);
  Array<std::complex<double>, 2> alpha(nelem, nelem);
  alpha = 0.0;
  
  Array<std::complex<double>, 2> Rhat(acm.copy());
    
  // .* (1 - mask)
  Rhat = where(restriction == false, Rhat, 0.0); 
  
  // R0 --> asrc * flux * asrc' 
  Array<std::complex<double>, 2> R0( bmMult(asrc, sigma, bmTransH(asrc)));
  // .* (1 - mask)
  R0 = where(restriction == false, R0, 0.0);
   
  Array<std::complex<double>, 1> w(nelem);
    
  for (int idx1 = 0; idx1 < nelem; idx1++) {
    for (int idx2 = 0; idx2 < nelem; idx2++) {
      w = 0.0;
      for (int idx3 = 0; idx3 < nelem; idx3++) {
        
        if ((R0(idx1,idx3) != 0.0) && (R0(idx2,idx3) != 0.0)) {
          if ((idx1 != idx3) || (idx2 != idx3)) {
            w(idx3) = abs( pow2( R0(idx1,idx3) * Rhat(idx2,idx3)));
	          alpha(idx1,idx2) += w(idx3) * conj((Rhat(idx1,idx3) * R0(idx2,idx3)) / (Rhat(idx2,idx3) * R0(idx1,idx3)));         
	        }
	      }
      }
      
      if (sum(w) != 0.0) {
        alpha(idx1, idx2) /= sum(w);
      }
    }
  }
  //bmToFile(alpha,"alpha",fileFormat);
  return (alpha);
}
//---------------------------------------------------------------------------------------------------------------------


Array<std::complex<double>, 1> RemoteStationCalibration::computeGain(
                            Array<std::complex<double>, 2>& acm, 
                            Array<std::complex<double>, 2>& asrc,
                            Array<double, 1>& flux, 
                            Array<bool, 2>& restriction,
                            int maxiter,
                            double diffstop )
{
  //bmToFile(acm,"acm",fileFormat);
  //bmToFile(asrc,"asrc",fileFormat);
  //bmToFile(flux,"flux",fileFormat);
  
  logfile << "start of computeGain" << " time=" << getClock() << endl;
  
  int nelem = asrc.extent(firstDim);
  int nsrc = asrc.extent(secondDim);
  int nrestriction = sum(restriction);
  logfile << "asrc=" << asrc.shape() << endl;
  
  
  // for experimental Isel
  blitz::Array<int,2> posIsel;
  blitz::Array<double,2> valIsel;
  blitz::Array<int,2> posInvIsel;
  blitz::Array<double,2> valInvIsel;
  setSelection_2(posIsel, valIsel, posInvIsel ,valInvIsel , restriction, nelem);   
  bmToFile(posIsel,"posIsel",fileFormat);
  bmToFile(valIsel,"valIsel",fileFormat);
  bmToFile(posInvIsel,"posInvIsel",fileFormat);
  bmToFile(valInvIsel,"valInvIsel",fileFormat);
  
        
  blitz::Array<double, 2> Isel; 
  blitz::Array<double, 2> pinvIsel;
  setSelection(Isel, pinvIsel, restriction, nelem);
  bmToFile(Isel,"Isel",fileFormat);
  bmToFile(pinvIsel,"pinvIsel",fileFormat); 
  
  logfile << "selection done" << endl;
   
  Array<std::complex<double>, 2> gHat(nelem, maxiter+1);
  Array<std::complex<double>, 2> sigmaHat(nsrc, maxiter+1);
  Array<std::complex<double>, 2> sigmanHat(nrestriction, maxiter+1);
  Array<std::complex<double>, 2> sigman(nelem, nelem);
  gHat = 0.0;
  sigmaHat(Range::all(), 0) = cast<std::complex<double> >(flux);
  sigmanHat(Range(0, nelem-1),0) = 1.0;    
  sigman = 0.0;
      
  // implementation using WALS
  int iter;
  for (iter = 1; iter < (maxiter+1); iter++) {
 	  logfile << "iteration=" << iter << " time=" << getClock() << endl;
 	  
 	  // estimate alpha = g * (1 ./ g)
 	  Array<std::complex<double>, 2> sigma( bmDiag( sigmaHat(Range::all(),iter-1)));
	  Array<complex<double>, 2> alpha( computeAlpha( sigma, acm, asrc, restriction));
    logfile << "alpha done" << " time=" << getClock() << endl;
    
    
    // Note: the result reduced by this  EVD algorithm 
    // differ from MatLab's eig-function for the lower eigenvalues
    Array<std::complex<double>,2> eigenvec; // V
    Array<std::complex<double>,2> eigenval; // D
    bmEig(alpha, eigenvec, eigenval);
    logfile << "eig done" << " time=" << getClock() << endl;

    //bmToFile(eigenvec,"eigenvec",fileFormat);
    //bmToFile(eigenval,"eigenval",fileFormat);
    
    TinyVector<int, 1> maxidx( maxIndex( abs( bmDiag(eigenval))));
    logfile << "maxidx=" << maxidx << endl;
    
    gHat(Range::all(), iter) = bmConj( eigenvec(Range::all(), maxidx(0)));
    //bmToFile(gHat,"gHat1",fileFormat);  
    Array<std::complex<double>,2> GA( bmMult( bmDiag( gHat(Range::all(),iter)), asrc));
    Array<std::complex<double>,2> Rest(bmMult( GA, bmDiag( sigmaHat(Range::all(), iter-1)), bmTransH(GA)));
    //bmToFile(GA,"GA",fileFormat);
    //bmToFile(Rest,"Rest",fileFormat);
    
    // convert Matrix Rest and acm into a Vector and leave out diagonal
    Array<std::complex<double>,2> RestV((Rest.rows()-1)*Rest.cols(),1);
    Array<std::complex<double>,2> RhatV((acm.rows()-1)*acm.cols(),1);
    int idx = 0;
    for (int idx1 = 0; idx1 < nelem; idx1++) {
      for (int idx2 = 0; idx2 < nelem; idx2++) {
        if (idx1 != idx2) {
          RestV(idx,0) = Rest(idx1, idx2);
          RhatV(idx,0) = acm(idx1, idx2);
          idx++;
        }          
      }
    }
    logfile << "MatrixToVector done" << " time=" << getClock() << endl;
    
    
    Array<double,2> normg( abs( sqrt( bmMult( bmPinv(RestV), RhatV))));
    //bmToFile(normg,"normg",fileFormat);
    
    logfile << "normg done" << " time=" << getClock() << endl;
    gHat(Range::all(),iter) = normg(0,0) * gHat(Range::all(), iter) / (gHat(0, iter) / abs( gHat(0, iter)));
    //bmToFile(gHat,"gHat2",fileFormat);
    
    Array<std::complex<double>,2> R0( pow2(normg(0,0)) * Rest);
    Array<std::complex<double>,2> sigmanHatM(sigmanHat.rows(),1);
    sigmanHatM(Range::all(),0) = sigmanHat(Range::all(), iter-1);   
    
    Array<std::complex<double>,2> tmp1( bmMult(Isel, sigmanHatM));
    sigman = bmVectorToMatrix(tmp1(Range::all(),0));
    // speed experiment
    //Array<std::complex<double>,1> tmp01( sigmanHat(Range::all(), iter-1));
    //Array<std::complex<double>,1> tmp1( multIsel(posIsel, valIsel, tmp01));  
    //sigman = bmVectorToMatrix(tmp1);
      
    
    //bmToFile(sigman,"sigman",fileFormat);
    logfile << "sigman tmp1 done" << " time=" << getClock() << endl;
    
    Array<std::complex<double>,2> tmp2(acm.rows()*acm.cols(),1);
    tmp2(Range::all(),0) = (bmMatrixToVector(acm) - bmMatrixToVector(R0));     
    logfile << "tmp2 done" << " time=" << getClock() << endl;
    
    Array<std::complex<double>,2> tmp3( bmMult( pinvIsel, tmp2));  
    sigmanHat(Range::all(), iter) = tmp3(Range::all(),0);
    // speed experiment
    //Array<std::complex<double>,1> tmp03( bmMatrixToVector(acm) - bmMatrixToVector(R0));
    //Array<std::complex<double>,1> tmp3( multIsel(posInvIsel, valInvIsel, tmp03));
    //sigmanHat(Range::all(), iter) = tmp3(Range::all());

    
    //bmToFile(sigmanHat,"sigmanHat",fileFormat);
    logfile << "tmp3 done" << " time=" << getClock() << endl;
    
    Array<std::complex<double>,2> tmp4(sigmanHat.rows(),1);
    tmp4(Range::all(),0) = sigmanHat(Range::all(), iter);  
    logfile << "tmp4 done" << " time=" << getClock() << endl;
    
    Array<std::complex<double>,2> tmp5( bmMult( Isel, tmp4));
    sigman = bmVectorToMatrix(tmp5(Range::all(),0));
    // speed experiment
    //Array<std::complex<double>,1> tmp05( sigmanHat(Range::all(), iter));
    //Array<std::complex<double>,1> tmp5( multIsel(posIsel, valIsel, tmp05));
    //sigman = bmVectorToMatrix(tmp5(Range::all()));
          
   
   //bmToFile(sigman,"sigman",fileFormat);
      
    logfile << "sigman tmp5 done" << " time=" << getClock() << endl;
    Array<std::complex<double>,2> R(R0 + sigman);
   //bmToFile(R,"R",fileFormat);
    Array<std::complex<double>,2> r1( cast<std::complex<double> >( bmAbs( bmSqr( bmConj( bmMult(bmTransH(GA), bmInv(R), GA)))))); 
    Array<std::complex<double>,2> r1a( bmInv( r1));
    logfile << "r1a done" << " time=" << getClock() << endl;   
    Array<std::complex<double>,2> r2(acm - sigman);  
    Array<std::complex<double>,2> r2a( bmMult(bmTransH(GA), bmInv(R), r2));
    logfile << "r2a done" << " time=" << getClock() << endl; 
    Array<std::complex<double>,1> r2b( bmDiag( bmMult(r2a, bmInv(R), GA)));
    Array<std::complex<double>,2> r2c(r2b.rows(),1);
    r2c(Range::all(),0) = r2b(Range::all());    
    logfile << "r2b done" << " time=" << getClock() << endl;  
    Array<double,2> sh( bmReal( bmMult(r1a, r2c)));           
   //bmToFile(sh,"sigmaHat1",fileFormat);
   
    Array<double,1> sh1( sh(Range::all(),0) / sh(0,0));
    sh1 = where(sh1 > 0.0, sh1, 0.0);
    
    sigmaHat(Range::all(), iter) = cast<std::complex<double> >(sh1(Range::all()));
      
   //bmToFile(sigmaHat,"sigmaHat2",fileFormat);
    logfile << "sigmaHat done" << " time=" << getClock() << endl;

    Array<std::complex<double>,2> thetaPrev(gHat.rows()+sigmaHat.rows()+sigmanHat.rows(),1);
    Array<std::complex<double>,2> theta(gHat.rows()+sigmaHat.rows()+sigmanHat.rows(),1);   
    int start, stop;
    start = 0;
    stop = gHat.rows() - 1;
    thetaPrev(Range(start, stop),0) = gHat(Range::all(), iter-1);
    theta(Range(start, stop),0) = gHat(Range::all(), iter);
    start = stop + 1;
    stop = start + sigmaHat.rows() - 1;
    thetaPrev(Range(start, stop),0) = sigmaHat(Range::all(), iter-1);
    theta(Range(start, stop),0) = sigmaHat(Range::all(), iter);
    start = stop + 1;
    stop = start + sigmanHat.rows() - 1;  
    thetaPrev(Range(start, stop),0) = sigmanHat(Range::all(), iter-1);
    theta(Range(start, stop),0) = sigmanHat(Range::all(), iter);
   //bmToFile(thetaPrev,"thetaPrev",fileFormat);
   //bmToFile(theta,"theta",fileFormat);
      
    Array<double,2> difftest( abs( bmMult( bmPinv(thetaPrev), theta) - 1.0));
   //bmToFile(difftest,"difftest",fileFormat);
    //logfile << "difftest done" << " time=" << getClock() << endl;
    //logfile << "difftest=" << difftest << endl;
    if (difftest(0,0) < diffstop) { logfile << "(diff < diffstop) --> break" << endl; break; }
  } // end wals
  if (iter == (maxiter+1)) { iter--; }
  
  //bmToFile(gHat,"gHat",fileFormat);
  //bmToFile(sigmaHat,"sigmaHat",fileFormat);
  
  // only for printing sigman
  //Array<std::complex<double>,2> tmp4(sigmanHat.rows(),1);
  //tmp4(Range::all(),0) = sigmanHat(Range::all(), iter);  
  //Array<std::complex<double>,2> tmp5( bmMult( Isel, tmp4));
  //sigman = bmVectorToMatrix(tmp5(Range::all(),0)); 
  //bmToFile(sigman,"sigman",fileFormat);
  
  return (gHat(Range::all(), iter));
}
//---------------------------------------------------------------------------------------------------------------------
/*
Array<std::complex<double>, 1> RemoteStationCalibration::checkCal(
                            Array<std::complex<double>, 1>& cal, 
                            blitz::Array<bool, 1>& good,
                            int nch,
                            int nelem )
{
  Array<double, 1> cal0( cal / bmRepmat( bmMedian( bmAbs(cal))));
  cal0 = where(cal0 == NAN, 0.0, cal0);
       
}
*/
//---------------------------------------------------------------------------------------------------------------------



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
//---------------------------------------------------------------------------------------------------------------------


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
//---------------------------------------------------------------------------------------------------------------------


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

