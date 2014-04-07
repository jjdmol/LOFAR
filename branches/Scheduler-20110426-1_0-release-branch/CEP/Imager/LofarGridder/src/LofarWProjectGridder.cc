//# LofarWProjectGridder.cc: WProjection gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2010
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

#include <lofar_config.h>
#include <ParmDB/Grid.h>

//# ASKAP includes
#include <LofarGridder/LofarWProjectGridder.h>
#include <gridding/VisGridderFactory.h>
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/TableConstDataIterator.h>

//# casacore includes
#include <tables/Tables/TableRecord.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <measures/Measures/MeasTable.h>
#include <casa/OS/Path.h>

using namespace LOFAR::BBS;
using namespace casa;
using namespace askap;
using namespace askap::synthesis;

namespace LOFAR
{

  LofarWProjectGridder::LofarWProjectGridder (const ParameterSet& parset)
    : WProjectVisGridder (*(dynamic_cast<const WProjectVisGridder*>
                            (WProjectVisGridder::createGridder(parset).get()))),
      itsInitialized (false)
  {
    // Currently only beam can be corrected for.
    ///    itsTimeAvg = parset.getInt ("average.timestep", 1);
    ///    itsFreqAvg = parset.getInt ("average.freqstep", 1);
    ///    itsCorrect = parset.getStringVector ("correct");
    itsParmDBName = parset.getString ("ParmDB.Instrument", "");
    itsConfigName = parset.getString ("Beam.StationConfig.Name", "");
    itsConfigPath = parset.getString ("Beam.StationConfig.Path", "");
  }

  LofarWProjectGridder::~LofarWProjectGridder()
  {}

  // Clone a copy of this Gridder
  IVisGridder::ShPtr LofarWProjectGridder::clone()
  {
    return IVisGridder::ShPtr(new LofarWProjectGridder(*this));
  }

  IVisGridder::ShPtr LofarWProjectGridder::makeGridder
  (const ParameterSet& parset)
  {
    std::cout << "LofarWProjectGridder::makeGridder" << std::endl;
    return IVisGridder::ShPtr(new LofarWProjectGridder(parset));
  }

  const std::string& LofarWProjectGridder::gridderName()
  {
    static std::string name("LofarGridder.WProject");
    return name;
  }

  void LofarWProjectGridder::registerGridder()
  {
    VisGridderFactory::registerGridder (gridderName(), &makeGridder);
  }

  void LofarWProjectGridder::correctVisibilities (IDataAccessor& acc,
                                                  bool forward)
  {
    if (!itsInitialized) {
      initCorrections (acc);
    }
    // If changed, set the facet center as direction of interest.
    MVDirection facetCenter = getImageCentre();
    if (facetCenter != itsLastFacetCenter) {
      itsLastFacetCenter = facetCenter;
      itsResponse->setDirection (MDirection(facetCenter, MDirection::J2000));
    }
    // Get read/write access to the visibilities. It makes a copy if needed.
    Array<Complex>& visData = acc.rwVisibility();
    ASSERT (visData.contiguousStorage());
    Complex* data = visData.data();
    uInt nfreq = visData.shape()[1];
    double time = acc.time();
    // Set the axes for the BBS request.
    Axis::ShPtr freqAxis
      (new RegularAxis(itsStartFreq-itsFreqInterval/2, itsFreqInterval, nfreq));
    Axis::ShPtr timeAxis
      (new RegularAxis(time-itsTimeInterval/2, itsTimeInterval, 1));
    Grid grid(timeAxis, freqAxis);
    itsResponse->setEvalGrid (grid);
    const casa::Vector<uInt>& ant1 = acc.antenna1();
    const casa::Vector<uInt>& ant2 = acc.antenna2();
    for (uInt i=0; i<ant1.size(); ++i) {
      JonesMatrix::View j1 = itsResponse->evaluate (ant1[i]);
      JonesMatrix::View j2 = itsResponse->evaluate (ant2[i]);
      for (uInt j=0; j<nfreq; ++j) {
        dcomplex cl0 = j1(j,0).getDComplex(0,0);
        dcomplex cl1 = j1(j,0).getDComplex(1,0);
        dcomplex cl2 = j1(j,0).getDComplex(0,1);
        dcomplex cl3 = j1(j,0).getDComplex(1,1);
        dcomplex cr0 = conj (j2(j,0).getDComplex(0,0));
        dcomplex cr1 = conj (j2(j,0).getDComplex(1,0));
        dcomplex cr2 = conj (j2(j,0).getDComplex(0,1));
        dcomplex cr3 = conj (j2(j,0).getDComplex(1,1));
        dcomplex d0 = data[0];
        dcomplex d1 = data[1];
        dcomplex d2 = data[2];
        dcomplex d3 = data[3];
        if (forward) {
          // For gridding use inverse of responses.
          //   data = inv(res(i)) * data * inv(conjtranspose(res(j)))
          // Note that inv(conjtranspose(A)) == conjtranspose(inv(A))
          // inv(a b) == ( d -b)  / (ad-bc)
          //    (c d)    (-c  a)
          dcomplex tmp0 = cl3*d0 - cl1*d2;
          dcomplex tmp1 = cl3*d1 - cl1*d3;
          dcomplex tmp2 = cl0*d2 - cl2*d0;
          dcomplex tmp3 = cl0*d3 - cl2*d1;
          dcomplex factor = 1. / ((cl0*cl3 - cl1*cl2) * (cr0*cr3 - cr1*cr2));
          data[0] = factor * (tmp0*cr3 - tmp1*cr1);
          data[1] = factor * (tmp1*cr0 - tmp0*cr2);
          data[2] = factor * (tmp2*cr3 - tmp3*cr1);
          data[3] = factor * (tmp3*cr0 - tmp2*cr2);
        } else {
          // Degridding; data = res(i) * data * conjtranspose(res(j))
          dcomplex tmp0 = cl0*d0 + cl1*d2;
          dcomplex tmp1 = cl0*d1 + cl1*d3;
          dcomplex tmp2 = cl2*d0 + cl3*d2;
          dcomplex tmp3 = cl2*d1 + cl3*d3;
          data[0] = tmp0*cr0 + tmp1*cr1;
          data[1] = tmp0*cr2 + tmp1*cr3;
          data[2] = tmp2*cr0 + tmp3*cr1;
          data[3] = tmp2*cr2 + tmp3*cr3;
        }
        data += 4;
      }
    }
  }

  void LofarWProjectGridder::initCorrections (const IConstDataAccessor& acc)
  {
    // Ensure there are polarizations XX,XY,YX,YY.
    ASSERT (acc.stokes().size() == 4);
    ASSERT (acc.stokes()[0] == Stokes::XX);
    ASSERT (acc.stokes()[1] == Stokes::XY);
    ASSERT (acc.stokes()[2] == Stokes::YX);
    ASSERT (acc.stokes()[3] == Stokes::YY);
    const TableConstDataAccessor& tacc =
      dynamic_cast<const TableConstDataAccessor&>(acc);
    const TableConstDataIterator& titer = tacc.iterator();
    MeasurementSet ms(titer.table());
    // Get the time interval.
    ROMSColumns msCol(ms);
    // Get the reference frequency and width.
    // It assumes that the interval is constant for this band.
    ROMSSpWindowColumns spwCols(ms.spectralWindow());
    double refFreq = spwCols.refFrequency()(0);
    itsFreqInterval = spwCols.chanWidth()(0).data()[0];
    casa::Vector<double> freqs = spwCols.chanFreq()(0);
    itsStartFreq = freqs[0];
    for (uInt i=1; i<freqs.size(); ++i) {
      ASSERTSTR (casa::near(freqs[i], itsStartFreq + i*itsFreqInterval),
                 "Frequency channels are not regularly spaced");
    }
    // Form the StationResponse object.
    itsResponse = StationResponse::Ptr (new StationResponse
                                        (makeInstrument(ms),
                                         itsConfigName, Path(itsConfigPath),
                                         refFreq));
    // Set the pointing direction (for beamforming).
    // Use first value of MDirection array in first row in FIELD subtable.
    ROMSFieldColumns fieldCols(ms.field());
    itsResponse->setPointing (fieldCols.delayDirMeasCol()(0).data()[0]);
    itsTimeInterval = msCol.interval()(0);
    // Initialization is done.
    itsInitialized = true;
  }

  BBS::Instrument LofarWProjectGridder::makeInstrument
  (const MeasurementSet& ms)
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

} //# end namespace
