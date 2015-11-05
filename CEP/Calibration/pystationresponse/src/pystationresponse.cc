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

#include <casa/OS/Path.h>
#include <casa/Containers/ValueHolder.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <measures/Measures/MeasTable.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>

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
    PyStationResponse(const string& msName, bool inverse = false,
        bool useElementBeam = true, bool useArrayFactor = true,
        bool useChannelFreq = false, bool conjugateAF = false);

    // Get the software version.
    string version(const string& type) const;

    // Set the delay reference direction in radians, J2000. The delay reference
    // direction is the direction used by the station beamformer.
    void setRefDelay(double ra, double dec);

    // Set the tile reference direction in radians, J2000. The tile reference
    // direction is the direction used by the analog tile beamformer and is
    // relevant only for HBA observations.
    void setRefTile(double ra, double dec);

    // Set the direction of interest in radians, J2000. Can and often will be
    // different than the delay and/or tile reference direction.
    void setDirection(double ra, double dec);

    // Compute the LOFAR beam Jones matrices for the given time, station, and/or
    // channel.
    ValueHolder evaluate0(double time);
    ValueHolder evaluate1(double time, int station);
    ValueHolder evaluate2(double time, int station, int channel);

    // Get the image in the beam direction with a given size and scale.
    /// Array<DComplex> getImage (int nRa, int nDec,
    /// double scaleRa, double scaleDec);

  private:
    // Fill the Jones matrices for itsTime and itsStation.
    void fillJones(unsigned int station, double time);

    // Initialize.
    void init(const MeasurementSet& ms, bool inverse, bool useElementBeam,
        bool useArrayFactor, bool useChannelFreq, bool conjugateAF);

    //# Data members.
    StationResponse::Ptr    itsResponse;
    double                  itsStartFreq;
    double                  itsFreqInterval;
    double                  itsTimeInterval;
    double                  itsTime;
    int                     itsNChan;
    //# Jones matrices for all channels, itsTime, and itsStation
    Array<DComplex>         itsJones;
  };


  PyStationResponse::PyStationResponse(const string& msName, bool inverse,
    bool useElementBeam, bool useArrayFactor, bool useChannelFreq,
    bool conjugateAF)
    : itsTime    (-1)
  {
    MeasurementSet ms(msName);
    init(ms, inverse, useElementBeam, useArrayFactor, useChannelFreq,
      conjugateAF);
  }

  string PyStationResponse::version(const string& type) const
  {
    return Version::getInfo<pystationresponseVersion>("stationresponse",
      type);
  }

  void PyStationResponse::setRefDelay(double ra, double dec)
  {
    MVDirection radec(Quantity(ra,"rad"), Quantity(dec,"rad"));
    itsResponse->setRefDelay(MDirection(radec, MDirection::J2000));
  }

  void PyStationResponse::setRefTile(double ra, double dec)
  {
    MVDirection radec(Quantity(ra,"rad"), Quantity(dec,"rad"));
    itsResponse->setRefTile(MDirection(radec, MDirection::J2000));
  }

  void PyStationResponse::setDirection(double ra, double dec)
  {
    MVDirection radec(Quantity(ra,"rad"), Quantity(dec,"rad"));
    itsResponse->setDirection(MDirection(radec, MDirection::J2000));
  }

  ValueHolder PyStationResponse::evaluate0(double time)
  {
    int nstat = itsResponse->nStations();

    Array<DComplex> result(IPosition(4,2,2,itsNChan,nstat));
    DComplex* resPtr = result.data();
    for (int i=0; i<nstat; ++i) {
      fillJones(i, time);
      memcpy (resPtr, itsJones.data(), itsJones.size()*sizeof(DComplex));
      resPtr += itsJones.size();
    }

    return ValueHolder(result);
  }

  ValueHolder PyStationResponse::evaluate1(double time, int station)
  {
    ASSERTSTR (station>=0 && station<int(itsResponse->nStations()),
               "Stationnr " << station << " is invalid");
    fillJones(station, time);

    return ValueHolder(itsJones);
  }

  ValueHolder PyStationResponse::evaluate2(double time, int station,
    int channel)
  {
    ASSERTSTR (station>=0 && station<int(itsResponse->nStations()),
               "Stationnr " << station << " is invalid");
    ASSERTSTR (channel>=0 && channel<itsNChan,
               "Channel " << channel << " is invalid");
    Array<DComplex> result(IPosition(2,2,2));
    fillJones(station, time);
    memcpy (result.data(), itsJones.data()+4*channel, 4*sizeof(DComplex));

    return ValueHolder(result);
  }

  void PyStationResponse::fillJones(unsigned int station, double time)
  {
    // Update the evaluation grid if necessary (this will clear the cache).
    if(time != itsTime)
    {
      itsTime = time;

      Axis::ShPtr freqAxis(new RegularAxis(itsStartFreq-itsFreqInterval/2,
        itsFreqInterval, itsNChan));
      Axis::ShPtr timeAxis(new RegularAxis(itsTime-itsTimeInterval/2,
        itsTimeInterval, 1));
      itsResponse->setEvalGrid (Grid(freqAxis, timeAxis));
    }

    JonesMatrix::View j1 = itsResponse->evaluate (station);
    const Matrix& xx = j1(0,0);
    const Matrix& xy = j1(0,1);
    const Matrix& yx = j1(1,0);
    const Matrix& yy = j1(1,1);

    DComplex* resultPtr = itsJones.data();
    for (int j=0; j<itsNChan; ++j) {
      *resultPtr++ = xx.getDComplex(j,0);
      *resultPtr++ = xy.getDComplex(j,0);
      *resultPtr++ = yx.getDComplex(j,0);
      *resultPtr++ = yy.getDComplex(j,0);
    }
  }

  void PyStationResponse::init(const MeasurementSet& ms, bool inverse,
    bool useElementBeam, bool useArrayFactor, bool useChannelFreq,
    bool conjugateAF)
  {
    // Get the time interval.
    ROMSColumns msCol(ms);
    itsTimeInterval = msCol.interval()(0);

    // Get the reference frequency and width.
    // It assumes that the interval is constant for this band.
    ROMSSpWindowColumns spwCols(ms.spectralWindow());
    itsFreqInterval = spwCols.chanWidth()(0).data()[0];
    casa::Vector<double> freqs = spwCols.chanFreq()(0);
    itsStartFreq = freqs[0];
    itsNChan     = freqs.size();
    for (uInt i=1; i<freqs.size(); ++i) {
      ASSERTSTR (casa::near(freqs[i], itsStartFreq + i*itsFreqInterval),
        "Frequency channels are not regularly spaced");
    }

    // Create the StationResponse object.
    itsResponse = StationResponse::Ptr(new StationResponse(ms, inverse,
      useElementBeam, useArrayFactor, useChannelFreq, conjugateAF));

    // Set the direction of interest equal to the phase center direction.
    ROMSFieldColumns fieldCols(ms.field());
    itsResponse->setDirection(fieldCols.phaseDirMeas(0));

    // Set the delay reference direction.
    // Use first value of MDirection array in first row in FIELD subtable.
    itsResponse->setRefDelay(fieldCols.delayDirMeas(0));

    // By default, the tile beam reference direction is assumed to be equal
    // to the station beam reference direction (for backward compatibility,
    // and for non-HBA measurements).
    itsResponse->setRefTile(fieldCols.delayDirMeas(0));

    // The MeasurementSet class does not support LOFAR specific columns, so we
    // use ROArrayMeasColumn to read the tile beam reference direction.
    Table tab_field = ms.keywordSet().asTable("FIELD");

    static const String columnName = "LOFAR_TILE_BEAM_DIR";
    if(tab_field.tableDesc().isColumn(columnName))
    {
        ROArrayMeasColumn<MDirection> c_direction(tab_field, columnName);
        if(c_direction.isDefined(0))
        {
            itsResponse->setRefTile(c_direction(0)(IPosition(1, 0)));
        }
    }

    // Size the result array.
    itsJones.resize (IPosition(3,2,2,itsNChan));
  }

  // Now define the interface in Boost-Python.
  void pystationresponse()
  {
    class_<PyStationResponse> ("StationResponse",
        init<std::string, bool, bool, bool, bool, bool>())
      .def ("_version", &PyStationResponse::version,
        (boost::python::arg("type")="other"))
      .def ("_setRefDelay", &PyStationResponse::setRefDelay,
        (boost::python::arg("ra"), boost::python::arg("dec")))
      .def ("_setRefTile", &PyStationResponse::setRefTile,
        (boost::python::arg("ra"), boost::python::arg("dec")))
      .def ("_setDirection", &PyStationResponse::setDirection,
        (boost::python::arg("ra"), boost::python::arg("dec")))
      .def ("_evaluate0", &PyStationResponse::evaluate0,
        (boost::python::arg("time")))
      .def ("_evaluate1", &PyStationResponse::evaluate1,
 	    (boost::python::arg("time"), boost::python::arg("station")))
      .def ("_evaluate2", &PyStationResponse::evaluate2,
 	    (boost::python::arg("time"), boost::python::arg("station"),
         boost::python::arg("channel")))
      ;
  }
}}

// Define the python module itself.
BOOST_PYTHON_MODULE(_stationresponse)
{
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_casa_valueholder();

  LOFAR::BBS::pystationresponse();
}
