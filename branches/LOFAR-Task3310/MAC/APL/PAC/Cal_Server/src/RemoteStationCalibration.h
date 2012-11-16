//#  -*- mode: c++ -*-
//#  RemoteStationCalibration.h: class definition for the Beam Server task.
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

#ifndef REMOTESTATIONCALIBRATION_H_
#define REMOTESTATIONCALIBRATION_H_

#include "Source.h"
#include "DipoleModel.h"
#include "CalibrationAlgorithm.h"
#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/CAL_Protocol/SubArray.h>
#include <blitz/array.h>

// for debugging
#include <fstream>

namespace LOFAR {
  namespace CAL {

    class RemoteStationCalibration : public CalibrationAlgorithm
    {
    public:
      RemoteStationCalibration(Sources& sources, DipoleModels& dipolemodels, AMC::Converter& converter);

      /**
       * Destructor: delete any dynamically allocated member variables
       */
      virtual ~RemoteStationCalibration() {}

      virtual void calibrate(const SubArray& subarray, ACC& acc, AntennaGains& result);
      
    private:
      double itsBaseLineRestriction; // baseline restriction in wavelengths
      double itsMaxBaseLine;  // max base line in meters
      double itsSpeedOfLight;
      
      //blitz::Array<double, 2> itsIsel;
      //blitz::Array<double, 2> itsInvIsel;  
      
      //blitz::Array<std::complex<double>, 2> itsAlpha;
      //blitz::Array<std::complex<double>, 1> itsGain;
        
      std::vector<Source> make_local_sky_model(
          const Sources& sources, 
          const blitz::Array<double, 1>& geoloc, 
          RTC::Timestamp& acmtime );
      
      blitz::Array<std::complex<double>, 2> computeA(
          std::vector<Source>& LSM,
          const blitz::Array<double, 3>& AntennaPos,
          double freq );
      
      blitz::Array<bool, 2> setRestriction(
          const blitz::Array<double, 3>& AntennaPos, 
          double freq );
      
      blitz::Array<std::complex<double>,1> multIsel(
          blitz::Array<int,2>& selpos,
          blitz::Array<double,2>& selval,
          blitz::Array<std::complex<double>,1>& x );
      
      void setSelection(
          blitz::Array<double, 2>& Isel, 
          blitz::Array<double, 2>& pinvIsel,
          blitz::Array<bool, 2>& restriction, 
          int nelem );
      
      void setSelection_2(
          blitz::Array<int,2>& posIsel,
          blitz::Array<double,2>& valIsel,
          blitz::Array<int,2>& posInvIsel,
          blitz::Array<double,2>& valInvIsel, 
          blitz::Array<bool, 2>& restriction,
          int nelem );
             
      blitz::Array<double,1> computeFlux(
          blitz::Array<std::complex<double>, 2>& acm, 
          blitz::Array<std::complex<double>, 2>& R0, 
          blitz::Array<bool, 2>& restriction );
                                
      blitz::Array<std::complex<double>, 2> computeAlpha(
          blitz::Array<std::complex<double>, 2>& sigma,
          blitz::Array<std::complex<double>, 2>& acm, 
          blitz::Array<std::complex<double>, 2>& R0, 
          blitz::Array<bool, 2>& restriction );
                                                
      blitz::Array<std::complex<double>, 1> computeGain(
          blitz::Array<std::complex<double>, 2>& acm, 
          blitz::Array<std::complex<double>, 2>& R0,
          blitz::Array<double, 1>& flux, 
          blitz::Array<bool, 2>& restriction,
          int maxiter,
          double diffstop );

      blitz::Array<bool,1> detectRFI(
          ACC& acc, 
          int pol, 
          double N );
      
      blitz::Array<bool,1> setPassBand(
          double start_freq, 
          double stop_freq , 
          const SpectralWindow& spw ); 
          
      blitz::Array<bool,1> detectBadElements(
          ACC& acc, 
          int pol,
          blitz::Array<bool, 1> selected ); 

      double interp1d(blitz::Array<double, 1> xval, blitz::Array<double, 1> yval, double xinterp);
      double interp2d(blitz::Array<double, 1> xgrid, blitz::Array<double, 1> ygrid, blitz::Array<double, 2> dataval, double xinterp, double yinterp);
      double interp3d(blitz::Array<double, 1> xgrid, blitz::Array<double, 1> ygrid, blitz::Array<double, 1> zgrid, blitz::Array<double, 3> dataval, double xinterp, double yinterp, double zinterp);

      // member variables needed to store local state
      ofstream logfile;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* REMOTESTATIONCALIBRATION_H_ */

