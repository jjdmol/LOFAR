//# AntennaFieldHBA.cc: Representation of an HBA antenna field.
//#
//# Copyright (C) 2013
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

#include <lofar_config.h>
#include <StationResponse/AntennaFieldHBA.h>
#include <StationResponse/MathUtil.h>
#include <StationResponse/MathUtil.h>
#include <Common/LofarLogger.h>
#include <fits/FITS/BasicFITS.h>
#include <Common/LofarLocators.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>

#include <wcslib/wcslib.h>

namespace LOFAR
{
namespace StationResponse
{

AntennaFieldHBA::AntennaFieldHBA(const string &name,
    const CoordinateSystem &coordinates, const AntennaModelHBA::ConstPtr &model)
    :   AntennaField(name, coordinates),
        itsAntennaModel(model),
        itsRotation(theirRotationMap.find(name)->second)
{
  casa::Bool ok=true;
  casa::String message;

  FileLocator locator("$LOFARROOT/share/beamnorms");
  string fitsFile=locator.locate("beamintmap-"+name+".fits");
  itsIntegrals = casa::ReadFITS(fitsFile.c_str(),ok,message);
  if (!ok) {
    LOG_WARN_STR("Could not read beam normalization fits file: " << message);
  }
}

matrix22c_t AntennaFieldHBA::response(real_t time, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    matrix22c_t response=itsAntennaModel->response(freq, itrf2field(direction),
        itrf2field(direction0)) * rotation(time, direction);

    return response;
}

diag22c_t AntennaFieldHBA::arrayFactor(real_t, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    diag22c_t af=itsAntennaModel->arrayFactor(freq, itrf2field(direction),
        itrf2field(direction0));

    return af;
}

raw_response_t AntennaFieldHBA::rawResponse(real_t time, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    raw_response_t result = itsAntennaModel->rawResponse(freq,
        itrf2field(direction), itrf2field(direction0));

    real_t norm=getNormalization(freq, direction);
    result.response = result.response * rotation(time, direction);
    result.response[0][0]/=norm; result.response[1][0]/=norm;
    result.response[0][1]/=norm; result.response[1][1]/=norm;
    return result;
}

raw_array_factor_t AntennaFieldHBA::rawArrayFactor(real_t, real_t freq,
    const vector3r_t &direction, const vector3r_t &direction0) const
{
    return itsAntennaModel->rawArrayFactor(freq, itrf2field(direction),
        itrf2field(direction0));
}

matrix22c_t AntennaFieldHBA::elementResponse(real_t time, real_t freq,
    const vector3r_t &direction) const
{
    return itsAntennaModel->elementResponse(freq, itrf2field(direction))
        * rotation(time, direction);
}

real_t AntennaFieldHBA::getNormalization(real_t freq,
                                         const vector3r_t &direction) const
{
  int status;

  /* Initialize the wcsprm struct, also taking control of memory allocated by
   * fits_read_wcstab(). */
  if ((status = wcsset(theirWCS_p.get()))) {
    fprintf(stderr, "wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
    return 1;
  }

  /* Print the struct. */
  if ((status = wcsprt(theirWCS_p.get()))) return status;

  // Get indices for azimuth and elevation
  const uint gridsize=50;

  std::pair<double,double> azel=getAzEl(position(), direction);
  double az=azel.first;
  double el=azel.second;

  uint x_index=0;
  uint y_index=0;
  uint freq_index=0;

  double xx_pix=sin(az)*gridsize*(.5-el/casa::C::pi);
  double yy_pix=cos(az)*gridsize*(.5-el/casa::C::pi);

  // Round to int, so add 0.5 and then truncate
  x_index=int(xx_pix+.5*gridsize+0.5);
  y_index=int(yy_pix+.5*gridsize+0.5);

  // Get index for frequency
  const double freq_min=100.e6;
  const double freq_max=250.e6;
  const uint numfreqs=16;

  // Round to int, so add 0.5 and then truncate
  freq_index=int((freq-freq_min)/(freq_max-freq_min)*(numfreqs-1)+0.5);

  ASSERTSTR(!itsIntegrals.empty(), "No beamnorms found for station "<<name());

  //cout<<"Name="<<name()<<", itsIntegrals.shape="<<itsIntegrals.shape()<<endl;
  //cout<<"Name="<<name()<<", freq="<<freq<<", az="<<az<<", el="<<el
  //    <<", index=["<<x_index<<", "<<y_index<<", "<<freq_index<<"], norm="
  //    <<itsIntegrals(casa::IPosition(3,y_index,x_index,freq_index))<<endl;

  // Todo: interpolation for frequency
  return real_t(itsIntegrals(casa::IPosition(3,y_index,x_index,freq_index)));
}


std::pair<double,double> AntennaFieldHBA::getAzEl(const vector3r_t &position,
                                                  const vector3r_t &direction)
{
     using namespace casa;
     MDirection dir_itrf(MVDirection(direction[0],direction[1],direction[2]),
                         MDirection::ITRF);

     casa::MVPosition mvPosition(position[0], position[1], position[2]);
     casa::MPosition pos(mvPosition, casa::MPosition::ITRF);

     MeasFrame frame(pos);

     MDirection::Convert itrf2azel(
         dir_itrf,
         MDirection::Ref(MDirection::AZEL, frame));

     MDirection dir_azel = itrf2azel();

     std::pair<double,double> azel;

     azel.first =dir_azel.getAngle().getValue()[0];
     azel.second=dir_azel.getAngle().getValue()[1];
     return azel;
}

casa::CountedPtr<fitsfile> AntennaFieldHBA::readFITS(const string &filename)
{
  int status=0;
  fitsfile *fptr;

  FileLocator locator("$LOFARROOT/share/beamnorms");
  string fitsFile=locator.locate(filename);

  /* Open the FITS test file */
  fits_open_file(&fptr, fitsFile.c_str(), READONLY, &status);
  if (status!=0) {
    fits_report_error(stderr, status);
    ASSERT(1==0);
  }

  return casa::CountedPtr<fitsfile>(fptr);
}

map<string,double> AntennaFieldHBA::readRotationMap()
{
  map<string,double> tmpMap;
  tmpMap.insert(pair<string,double>("CS001HBA0", 24.));
  tmpMap.insert(pair<string,double>("CS001HBA1", 24.));
  tmpMap.insert(pair<string,double>("CS002HBA0", 52.));
  tmpMap.insert(pair<string,double>("CS002HBA1",  0.));
  tmpMap.insert(pair<string,double>("CS003HBA0", 30.));
  tmpMap.insert(pair<string,double>("CS003HBA1", 52.));
  tmpMap.insert(pair<string,double>("CS004HBA0", 36.));
  tmpMap.insert(pair<string,double>("CS004HBA1", 60.));
  tmpMap.insert(pair<string,double>("CS005HBA0", 66.));
  tmpMap.insert(pair<string,double>("CS005HBA1", 46.));
  tmpMap.insert(pair<string,double>("CS006HBA0", 76.));
  tmpMap.insert(pair<string,double>("CS006HBA1", 52.));
  tmpMap.insert(pair<string,double>("CS007HBA0", 82.));
  tmpMap.insert(pair<string,double>("CS007HBA1", 16.));
  tmpMap.insert(pair<string,double>("CS011HBA0",  8.));
  tmpMap.insert(pair<string,double>("CS011HBA1",  8.));
  tmpMap.insert(pair<string,double>("CS012HBA0",  4.));
  tmpMap.insert(pair<string,double>("CS012HBA1",  4.));
  tmpMap.insert(pair<string,double>("CS013HBA0", 54.));
  tmpMap.insert(pair<string,double>("CS013HBA1", 54.));
  tmpMap.insert(pair<string,double>("CS016HBA0", 54.));
  tmpMap.insert(pair<string,double>("CS016HBA1", 54.));
  tmpMap.insert(pair<string,double>("CS017HBA0", 38.));
  tmpMap.insert(pair<string,double>("CS017HBA1", 38.));
  tmpMap.insert(pair<string,double>("CS018HBA0", 86.));
  tmpMap.insert(pair<string,double>("CS018HBA1", 86.));
  tmpMap.insert(pair<string,double>("CS020HBA0", 84.));
  tmpMap.insert(pair<string,double>("CS020HBA1", 84.));
  tmpMap.insert(pair<string,double>("CS021HBA0", 68.));
  tmpMap.insert(pair<string,double>("CS021HBA1", 68.));
  tmpMap.insert(pair<string,double>("CS023HBA0",  4.));
  tmpMap.insert(pair<string,double>("CS023HBA1",  4.));
  tmpMap.insert(pair<string,double>("CS024HBA0", 20.));
  tmpMap.insert(pair<string,double>("CS024HBA1", 20.));
  tmpMap.insert(pair<string,double>("CS026HBA0", 50.));
  tmpMap.insert(pair<string,double>("CS026HBA1", 50.));
  tmpMap.insert(pair<string,double>("CS028HBA0", 64.));
  tmpMap.insert(pair<string,double>("CS028HBA1", 64.));
  tmpMap.insert(pair<string,double>("CS030HBA0", 28.));
  tmpMap.insert(pair<string,double>("CS030HBA1", 28.));
  tmpMap.insert(pair<string,double>("CS031HBA0", 34.));
  tmpMap.insert(pair<string,double>("CS031HBA1", 34.));
  tmpMap.insert(pair<string,double>("CS032HBA0", 80.));
  tmpMap.insert(pair<string,double>("CS032HBA1", 80.));
  tmpMap.insert(pair<string,double>("CS101HBA0", 12.));
  tmpMap.insert(pair<string,double>("CS101HBA1", 12.));
  tmpMap.insert(pair<string,double>("CS201HBA0", 42.));
  tmpMap.insert(pair<string,double>("CS201HBA1", 42.));
  tmpMap.insert(pair<string,double>("CS301HBA0", 58.));
  tmpMap.insert(pair<string,double>("CS301HBA1", 58.));
  tmpMap.insert(pair<string,double>("CS401HBA0", 72.));
  tmpMap.insert(pair<string,double>("CS401HBA1", 72.));
  tmpMap.insert(pair<string,double>("CS501HBA0", 88.));
  tmpMap.insert(pair<string,double>("CS501HBA1", 88.));
  tmpMap.insert(pair<string,double>("CS302HBA0", 48.));
  tmpMap.insert(pair<string,double>("CS302HBA1", 48.));
  tmpMap.insert(pair<string,double>("CS103HBA0",  2.));
  tmpMap.insert(pair<string,double>("CS103HBA1",  2.));
  tmpMap.insert(pair<string,double>("RS104HBA",  22.));
  tmpMap.insert(pair<string,double>("RS106HBA",  36.));
  tmpMap.insert(pair<string,double>("RS205HBA",  32.));
  tmpMap.insert(pair<string,double>("RS208HBA",  18.));
  tmpMap.insert(pair<string,double>("RS210HBA",  56.));
  tmpMap.insert(pair<string,double>("RS305HBA",  22.));
  tmpMap.insert(pair<string,double>("RS306HBA",  70.));
  tmpMap.insert(pair<string,double>("RS307HBA",  10.));
  tmpMap.insert(pair<string,double>("RS310HBA",  74.));
  tmpMap.insert(pair<string,double>("RS404HBA",  68.));
  tmpMap.insert(pair<string,double>("RS406HBA",  82.));
  tmpMap.insert(pair<string,double>("RS407HBA",   6.));
  tmpMap.insert(pair<string,double>("RS409HBA",  14.));
  tmpMap.insert(pair<string,double>("RS410HBA",  44.));
  tmpMap.insert(pair<string,double>("RS503HBA",  78.));
  tmpMap.insert(pair<string,double>("RS508HBA",  26.));
  tmpMap.insert(pair<string,double>("RS509HBA",  40.));
  tmpMap.insert(pair<string,double>("DE601HBA",  16.));
  tmpMap.insert(pair<string,double>("DE602HBA",  60.));
  tmpMap.insert(pair<string,double>("DE603HBA", 120.));
  tmpMap.insert(pair<string,double>("DE604HBA", 180.));
  tmpMap.insert(pair<string,double>("DE605HBA",  46.));
  tmpMap.insert(pair<string,double>("DE609HBA",  44.));
  tmpMap.insert(pair<string,double>("FR606HBA",  80.));
  tmpMap.insert(pair<string,double>("SE607HBA",  20.));
  tmpMap.insert(pair<string,double>("UK608HBA",  50.));
  tmpMap.insert(pair<string,double>("FI609HBA",  31.));


  return tmpMap;
}

casa::CountedPtr<wcsprm> AntennaFieldHBA::readWCS(casa::CountedPtr<fitsfile> fitsPtr)
{
  int status=0, nkeyrec, nreject, nwcs, stat[NWCSFIX];
  char *header;
  struct wcsprm *wcs;

  /* Read the primary header. */
  if ((status = fits_hdr2str(fitsPtr.get(), 1, NULL, 0, &header, &nkeyrec,
                             &status))) {
    fits_report_error(stderr, status);
    THROW (Exception, "Error reading primary header of FITS file");
  }

  /* Parse the primary header of the FITS file. */
  if ((status = wcspih(header, nkeyrec, WCSHDR_all, 2, &nreject, &nwcs,
                       &wcs))) {
    fprintf(stderr, "wcspih ERROR %d: %s.\n", status,wcshdr_errmsg[status]);
    THROW (Exception, "Error parsing primary header in FITS file");
  }

  /* Read coordinate arrays from the binary table extension. */
  if ((status = fits_read_wcstab(fitsPtr.get(), wcs->nwtb, (wtbarr *)wcs->wtb,
                                 &status))) {
    fits_report_error(stderr, status);
    THROW (Exception, "Error reading coordinate arrays from FITS file");
  }

  /* Translate non-standard WCS keyvalues. */
  if ((status = wcsfix(7, 0, wcs, stat))) {
    for (uint i = 0; i < NWCSFIX; i++) {
      if (stat[i] > 0) {
        fprintf(stderr, "wcsfix ERROR %d: %s.\n", status,
                wcsfix_errmsg[stat[i]]);
      }
    }

    THROW (Exception, "Error translating non-standard WCS keyvalues");
  }
  return casa::CountedPtr<wcsprm>(wcs);
}


casa::CountedPtr<fitsfile> AntennaFieldHBA::theirFitsFile_p =
    AntennaFieldHBA::readFITS("cs002hba1mode5.fits");
map<string,double> AntennaFieldHBA::theirRotationMap =
    AntennaFieldHBA::readRotationMap();
casa::CountedPtr<wcsprm> AntennaFieldHBA::theirWCS_p =
    AntennaFieldHBA::readWCS(theirFitsFile_p);

} //# namespace StationResponse
} //# namespace LOFAR
