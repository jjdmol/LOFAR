//# pystationresponse.cc: python module for StationResponse object.
//# Copyright (C) 2007
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
#include <BBSKernel/StationResponse.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <measures/Measures/MeasTable.h>
#include <casa/OS/Path.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "Package__Version.cc"

using namespace boost::python;
using namespace casa;
using namespace casa::pyrap;

namespace LOFAR { namespace BBS  {

  class PyStationResponse
  {
  public:
    PyStationResponse (const string& msName, const string& config,
                       const string& configPath, double ra, double dec,
                       double phaseCenterRa, double phaseCenterDec);

    // Get the software version.
    string version (const string& type) const
      { return Version::getInfo<pystationresponseVersion>
          ("stationresponse", type); }

    // Get the Jones matrices for the given time, station, and/or channel.
    Array<DComplex> getJones1 (double time);
    Array<DComplex> getJones2 (double time, int station);
    Array<DComplex> getJones3 (double time, int station, int channel);

    // Get the image in the beam direction with a given size and scale.
    /// Array<DComplex> getImage (int nRa, int nDec,
    /// double scaleRa, double scaleDec);

  private:
    // Fill the Jones matrices for itsTime and itsStation.
    void fillJones();

    // Initialize.
    void init (const MeasurementSet& ms, const string& configName,
               const string& configPath, double ra, double dec,
               double phaseCenterRa, double phaseCenterDec);

    // Create a BBS Instrument object from the MS.
    Instrument makeInstrument (const MeasurementSet&);

    //# Data members.
    StationResponse::Ptr itsResponse;
    double itsStartFreq;
    double itsFreqInterval;
    double itsTimeInterval;
    double itsTime;
    int    itsStation;
    int    itsNChan;
    //# Jones matrices for all channels, itsTime, and itsStation
    Array<DComplex> itsJones;
  };


  PyStationResponse::PyStationResponse (const string& msName,
                                        const string& configName,
                                        const string& configPath,
                                        double ra, double dec,
                                        double phaseCenterRa,
                                        double phaseCenterDec)
    : itsTime    (-1),
      itsStation (-1)
  {
    // Initialize.
    MeasurementSet ms(msName);
    init (ms, configName, configPath, ra, dec, phaseCenterRa, phaseCenterDec);
  }

  Array<DComplex> PyStationResponse::getJones3 (double time, int station,
                                                int channel)
  {
    ASSERTSTR (station>=0 && station<int(itsResponse->nStations()),
               "Stationnr " << station << " is invalid");
    ASSERTSTR (channel>=0 && channel<itsNChan,
               "Channel " << channel << " is invalid");
    Array<DComplex> result(IPosition(2,2,2));
    if (station != itsStation  ||  time != itsTime) {
      itsStation = station;
      itsTime    = time;
      fillJones();
    }
    memcpy (result.data(), itsJones.data()+4*channel, 4*sizeof(Complex));
    return result;
  }

  Array<DComplex> PyStationResponse::getJones2 (double time, int station)
  {
    ASSERTSTR (station>=0 && station<int(itsResponse->nStations()),
               "Stationnr " << station << " is invalid");
    if (station != itsStation  ||  time != itsTime) {
      itsStation = station;
      itsTime    = time;
      fillJones();
    }
    return itsJones;
  }

  Array<DComplex> PyStationResponse::getJones1 (double time)
  {
    int nstat = itsResponse->nStations();
    itsTime = time;
    Array<DComplex> result(IPosition(4,2,2,itsNChan,nstat));
    DComplex* resPtr = result.data();
    for (int i=0; i<nstat; ++i) {
      itsStation = i;
      fillJones();
      memcpy (resPtr, itsJones.data(), itsJones.size()*sizeof(DComplex));
      resPtr += itsJones.size();
    }
    return result;
  }

  void PyStationResponse::fillJones()
  {
    // Set the axes for the BBS request.
    Axis::ShPtr freqAxis
      (new RegularAxis(itsStartFreq-itsFreqInterval/2, itsFreqInterval,
                       itsNChan));
    Axis::ShPtr timeAxis
      (new RegularAxis(itsTime-itsTimeInterval/2, itsTimeInterval, 1));
    Grid grid(timeAxis, freqAxis);
    itsResponse->setEvalGrid (grid);
    JonesMatrix::View j1 = itsResponse->evaluate (itsStation);
    DComplex* resultPtr = itsJones.data();
    for (int j=0; j<itsNChan; ++j) {
      *resultPtr++ = j1(j,0).getDComplex(0,0);
      *resultPtr++ = j1(j,0).getDComplex(1,0);
      *resultPtr++ = j1(j,0).getDComplex(0,1);
      *resultPtr++ = j1(j,0).getDComplex(1,1);
    }
  }

  void PyStationResponse::init (const MeasurementSet& ms,
                                const string& configName,
                                const string& configPath,
                                double ra, double dec,
                                double phaseCenterRa, double phaseCenterDec)
  {
    // Get the time interval.
    ROMSColumns msCol(ms);
    itsTimeInterval = msCol.interval()(0);
    // Get the reference frequency and width.
    // It assumes that the interval is constant for this band.
    ROMSSpWindowColumns spwCols(ms.spectralWindow());
    double refFreq = spwCols.refFrequency()(0);
    itsFreqInterval = spwCols.chanWidth()(0).data()[0];
    casa::Vector<double> freqs = spwCols.chanFreq()(0);
    itsStartFreq = freqs[0];
    itsNChan     = freqs.size();
    for (uInt i=1; i<freqs.size(); ++i) {
      ASSERTSTR (casa::near(freqs[i], itsStartFreq + i*itsFreqInterval),
                 "Frequency channels are not regularly spaced");
    }
    // Form the StationResponse object.
    itsResponse = StationResponse::Ptr (new StationResponse
                                        (makeInstrument(ms),
                                         configName, Path(configPath),
                                         refFreq));
    // Set the pointing direction (for beamforming).
    // Use first value of MDirection array in first row in FIELD subtable.
    if (phaseCenterRa < -100  ||  phaseCenterDec < -100) {
      ROMSFieldColumns fieldCols(ms.field());
      itsResponse->setPointing (fieldCols.delayDirMeasCol()(0).data()[0]);
    } else {
      MVDirection radec (Quantity(phaseCenterRa,"rad"),
                         Quantity(phaseCenterDec,"rad"));
      itsResponse->setPointing (MDirection(radec, MDirection::J2000));
    }
    // Set the beam direction.
    MVDirection radec (Quantity(ra,"rad"), Quantity(dec,"rad"));
    itsResponse->setDirection (MDirection(radec, MDirection::J2000));
    // Size the result array.
    itsJones.resize (IPosition(2,2,itsNChan));
  }

  Instrument PyStationResponse::makeInstrument (const MeasurementSet& ms)
  {
    // Get all station positions.
    ROMSAntennaColumns antCols(ms.antenna());
    vector<Station> stations;
    stations.reserve (ms.antenna().nrow());
    for (uInt i=0; i<ms.antenna().nrow(); ++i) {
      stations.push_back (Station(antCols.name()(i),
                                  antCols.positionMeas()(i)));
    }
    // Find observatory position.
    // If not found, set it to the position of the middle station.
    MPosition arrayPos;
    Bool fndObs = False;
    if (ms.observation().nrow() > 0) {
      ROMSObservationColumns obsCols(ms.observation());
      String telescope = obsCols.telescopeName()(0);
      fndObs = MeasTable::Observatory (arrayPos, telescope);
    }
    if (!fndObs  &&  stations.size() > 0) {
      arrayPos = stations[stations.size()/2].position();
    }
    return BBS::Instrument ("LOFAR", arrayPos, stations);
  }

    // Now define the interface in Boost-Python.
  void pystationresponse()
  {
    class_<PyStationResponse> ("StationResponse",
                               init<std::string, std::string,
                               std::string, double, double, double, double>())

      .def ("_version", &PyStationResponse::version,
            (boost::python::arg("type")="other"))
      .def ("_getJones1", &PyStationResponse::getJones1,
 	    (boost::python::arg("time")))
      .def ("_getJones2", &PyStationResponse::getJones2,
 	    (boost::python::arg("time"),
             boost::python::arg("station")))
      .def ("_getJones3", &PyStationResponse::getJones3,
 	    (boost::python::arg("time"),
             boost::python::arg("station"),
             boost::python::arg("channel")))
      ;
  }
    
}}


// Define the python module itself.
BOOST_PYTHON_MODULE(_stationresponse)
{
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_std_vector<DComplex>();

  LOFAR::BBS::pystationresponse();
}
