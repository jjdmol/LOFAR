//# LOFARATerm.cc: Compute the LOFAR beam response on the sky.
//#
//# Copyright (C) 2011
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

#include <LofarFT/LOFARATerm.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>

//#include <synthesis/MeasurementComponents/Utils.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MeasTable.h>

using namespace casa;

namespace LOFAR
{

  LOFARATerm::LOFARATerm(const MeasurementSet &ms)
  {
    m_coeffLBA.load(Path("element_beam_LBA.coeff"));
    m_coeffHBA.load(Path("element_beam_HBA.coeff"));

    initInstrument(ms);
    initReferenceFreq(ms, 0);
    initPhaseReference(ms, 0);
  }

  Cube<Complex> LOFARATerm::evaluate(const IPosition &shape, const DirectionCoordinate &coordinates,
                                     uint station, const MEpoch &epoch) const
  {
    ASSERT(station < m_instrument.nStations());

    LOG_INFO ("LOFARATerm evaluate shape: " << shape);

    // Create conversion engine (from J2000 -> ITRF).
    MDirection::Convert convertor = MDirection::Convert(MDirection::J2000,
                                                        MDirection::Ref(MDirection::ITRF, MeasFrame(epoch, m_instrument.position())));

    // Compute ITRF map.
    LOG_INFO ("LOFARATerm evaluate Computing ITRF map...");
    Cube<double> mapITRF = computeITRFMap(coordinates, shape, convertor);
    LOG_INFO ("LOFARATerm evaluate Computing ITRF map... done.");

    // Compute element beam response.
    LOG_INFO ("LOFARATerm evaluate Computing station beam...");
    Cube<Complex> beam = computeStationBeam(mapITRF, convertor(m_phaseReference),
                                            m_instrument.station(station));
    LOG_INFO ("LOFARATerm evaluate Computing station beam... done.");

    return beam;
  }

  Cube<Complex> LOFARATerm::computeElementBeam(const Cube<double> &map,
                                               const BeamCoeff &coeff,
                                               const AntennaField &field) const
  {
    const Vector3 &p = field.axis(AntennaField::P);
    const Vector3 &q = field.axis(AntennaField::Q);
    const Vector3 &r = field.axis(AntennaField::R);

    const uint nX = map.shape()(1);
    const uint nY = map.shape()(2);

    Cube<Complex> elementBeam(nX, nY, 4, Complex(0.0, 0.0));
    for(uint j = 0; j < nY; ++j)
      {
        for(uint i = 0; i < nX; ++i)
          {
            if(map(0, i, j) == 0 && map(1, i, j) == 0 && map(2, i, j) == 0)
              {
                // Non-physical pixel.
                continue;
              }

            // Compute the P and Q coordinate of the direction vector by projecting
            // onto the positive P and Q axis.
            double projectionP = map(0, i, j) * p[0] + map(1, i, j) * p[1] + map(2, i, j) * p[2];
            double projectionQ = map(0, i, j) * q[0] + map(1, i, j) * q[1] + map(2, i, j) * q[2];

            // Compute the inner product between the antenna field normal
            // (R) and the direction vector to get the sine of the elevation
            // (cosine of the zenith angle).
            double sinEl = map(0, i, j) * r[0] + map(1, i, j) * r[1] + map(2, i, j) * r[2];

            double az = atan2(projectionP, projectionQ);
            double el = asin(sinEl);

            // Evaluate beam.
            const double orientation = 3.0 * C::pi_4;

            // Correct azimuth for dipole orientation.
            const double phi = az - orientation;

            // NB: The model is parameterized in terms of zenith angle. The
            // appropriate conversion is taken care of below.
            const double theta = C::pi_2 - el;

            const double freq = 60e6;

            // J-jones matrix (2x2 complex matrix)
            DComplex J[2][2] = {{0.0, 0.0}, {0.0, 0.0}};

            // Only compute the beam response for directions above the horizon.
            if(theta < C::pi_2)
              {
                // NB: The model is parameterized in terms of a normalized
                // frequency in the range [-1, 1]. The appropriate conversion is
                // taken care of below.
                const double normFreq = (freq - coeff.center()) / coeff.width();

                for(uint k = 0; k < coeff.nHarmonics(); ++k)
                  {
                    // Compute diagonal projection matrix P for the current
                    // harmonic.
                    DComplex P[2] = {0.0, 0.0};

                    DComplex inner[2];
                    for(Int i = coeff.nPowerTheta() - 1; i >= 0; --i)
                      {
                        inner[0] = coeff(0, coeff.nPowerFreq() - 1, i, k);
                        inner[1] = coeff(1, coeff.nPowerFreq() - 1, i, k);

                        for(Int j = coeff.nPowerFreq() - 2; j >= 0; --j)
                          {
                            inner[0] = inner[0] * normFreq + coeff(0, j, i, k);
                            inner[1] = inner[1] * normFreq + coeff(1, j, i, k);
                          }
                        P[0] = P[0] * theta + inner[0];
                        P[1] = P[1] * theta + inner[1];
                      }

                    // Compute Jones matrix for this harmonic by rotating P over
                    // kappa * phi and add it to the result.
                    const double kappa = ((k & 1) == 0 ? 1.0 : -1.0) * (2.0 * k + 1.0);
                    const double cphi = cos(kappa * phi);
                    const double sphi = sin(kappa * phi);

                    J[0][0] += cphi * P[0];
                    J[0][1] += -sphi * P[1];
                    J[1][0] += sphi * P[0];
                    J[1][1] += cphi * P[1];
                  }

                elementBeam(i, j, 0) = J[0][0];
                elementBeam(i, j, 1) = J[0][1];
                elementBeam(i, j, 2) = J[1][0];
                elementBeam(i, j, 3) = J[1][1];
              }
          }
      }

    return elementBeam;
  }

  Cube<Complex> LOFARATerm::computeStationBeam(const Cube<double> &map,
                                               const MDirection &reference, const Station &station) const
  {
    const uint nX = map.shape()(1);
    const uint nY = map.shape()(2);

    Cube<Complex> E(IPosition(3, nX, nY, 4));

    MVDirection mvReference = reference.getValue();

    // Compute angular reference frequency.
    const double omega0 = C::_2pi * m_referenceFreq;

    const double freq = m_referenceFreq;

    LOG_INFO("LOFARATerm computeStationBeam "
             << "reference: "
             << mvReference(0) << " " << mvReference(1) << " " << mvReference(2)
             << " map(nx/2, ny/2): "
             << map(0, nX/2, nY/2) << " " << map(1, nX/2, nY/2)
             << " " << map(2, nX/2, nY/2));

    uLong countX = 0, countY = 0;
    for(uint i = 0; i < station.nField(); ++i)
      {
        const AntennaField &field = station.field(i);

        // Compute element beam.
        LOG_INFO("LOFARATerm computeStationBeam "
                 "Computing element beam...");
        Cube<Complex> elementBeam;
        if(field.isHBA())
          {
            elementBeam = computeElementBeam(map, m_coeffHBA, field);
          }
        else
          {
            elementBeam = computeElementBeam(map, m_coeffLBA, field);
          }
        LOG_INFO("LOFARATerm computeStationBeam "
                 "Computing element beam... done.");

        // Compute tile array factor.
        Matrix<Complex> AFTile(nX, nY, Complex(0.0, 0.0));
        if(field.isHBA())
          {
            LOG_INFO("LOFARATerm computeStationBeam "
                     "Computing tile array factor...");

            for(uint y = 0; y < nY; ++y)
              {
                for(uint x = 0; x < nX; ++x)
                  {
                    // Instead of computing a phase shift for the pointing direction and a phase
                    // shift for the direction of interest and then computing the difference,
                    // compute the resultant phase shift in one go. Here we make use of the
                    // relation a . b + a . c = a . (b + c). The sign of k is related to the
                    // sign of the phase shift.
                    double k[3];
                    k[0] = map(0, x, y) - mvReference(0);
                    k[1] = map(1, x, y) - mvReference(1);
                    k[2] = map(2, x, y) - mvReference(2);

                    for(uint j = 0; j < field.nTileElement(); ++j)
                      {
                        // Compute the effective delay for a plane wave approaching from the
                        // direction of interest with respect to the phase center of element i
                        // when beam forming in the reference direction using time delays.
                        const Vector3 &offset = field.tileElement(j);
                        double delay = (k[0] * offset[0] + k[1] * offset[1] + k[2] * offset[2])
                          / C::c;

                        // Turn the delay into a phase shift.
                        const double shift = C::_2pi * freq * delay;

                        AFTile(x, y) += Complex(cos(shift), sin(shift));
                      }
                  }
              }

            // Normalize.
            if(field.nTileElement() > 0)
              {
                AFTile /= field.nTileElement();
              }

            for(uint j = 0; j < 4; ++j)
              {
                Matrix<Complex> plane = elementBeam.xyPlane(j);
                plane *= AFTile;
              }

            LOG_INFO("LOFARATerm computeStationBeam "
                     "Computing tile array factor... done.");
          }

        LOG_INFO("LOFARATerm computeStationBeam "
                 "Computing field array factor...");

        // Account for the case where the delay center is not equal to the
        // field center (only applies to core HBA fields).
        const Vector3 &fieldCenter = field.position();
        MVPosition delayCenter = station.position().getValue();
        Vector3 offsetShift = {{fieldCenter[0] - delayCenter(0),
                                fieldCenter[1] - delayCenter(1),
                                fieldCenter[2] - delayCenter(2)}};

        // Compute array factors.
        Matrix<Complex> AFX(nX, nY, Complex(0.0, 0.0));
        Matrix<Complex> AFY(nX, nY, Complex(0.0, 0.0));
        Matrix<Complex> AF(nX, nY);

        for(uint j = 0; j < field.nElement(); ++j)
          {
            const AntennaField::Element &element = field.element(j);
            if(element.flag[0] && element.flag[1])
              {
                continue;
              }

            // Compute the offset relative to the delay center.
            Vector3 offset = {{element.offset[0] + offsetShift[0],
                               element.offset[1] + offsetShift[1],
                               element.offset[2] + offsetShift[2]}};

            // Compute the delay for a plane wave approaching from the phase
            // reference direction with respect to the phase center of the
            // element.
            double delay0 = (mvReference(0) * offset[0] + mvReference(1) * offset[1]
                             + mvReference(2) * offset[2]) / casa::C::c;
            double shift0 = omega0 * delay0;

            for(uint k = 0; k < nY; ++k)
              {
                for(uint l = 0; l < nX; ++l)
                  {
                    // Compute the delay for a plane wave approaching from the direction
                    // of interest with respect to the phase center of the element.
                    double delay = (map(0, l, k) * offset[0] + map(1, l, k) * offset[1]
                                    + map(2, l, k) * offset[2]) / casa::C::c;

                    double shift = C::_2pi * freq * delay - shift0;
                    AF(l, k) = DComplex(cos(shift), sin(shift));
                  }
              }

            if(!element.flag[0])
              {
                AFX += AF;
                ++countX;
              }

            if(!element.flag[1])
              {
                AFY += AF;
                ++countY;
              }
          }

        if(i == 0)
          {
            E.xyPlane(0) = AFX * elementBeam.xyPlane(0);
            E.xyPlane(1) = AFX * elementBeam.xyPlane(1);
            E.xyPlane(2) = AFY * elementBeam.xyPlane(2);
            E.xyPlane(3) = AFY * elementBeam.xyPlane(3);
          }
        else
          {
            Matrix<Complex> plane0 = E.xyPlane(0);
            plane0 += AFX * elementBeam.xyPlane(0);

            Matrix<Complex> plane1 = E.xyPlane(1);
            plane1 += AFX * elementBeam.xyPlane(1);

            Matrix<Complex> plane2 = E.xyPlane(2);
            plane2 += AFX * elementBeam.xyPlane(2);

            Matrix<Complex> plane3 = E.xyPlane(3);
            plane3 += AFX * elementBeam.xyPlane(3);
          }

        LOG_INFO("LOFARATerm computeStationBeam "
                 "Computing field array factor... done.");
      }

    // Normalize.
    if(countX > 0)
      {
        Matrix<Complex> plane0 = E.xyPlane(0);
        plane0 /= countX;
        Matrix<Complex> plane1 = E.xyPlane(1);
        plane1 /= countX;
      }

    if(countY > 0)
      {
        Matrix<Complex> plane2 = E.xyPlane(2);
        plane2 /= countY;
        Matrix<Complex> plane3 = E.xyPlane(3);
        plane3 /= countY;
      }

    return E;
  }

  Cube<double> LOFARATerm::computeITRFMap(const DirectionCoordinate &coordinates,
                                          const IPosition &shape, MDirection::Convert convertor) const
  {
    MDirection world;
    Vector<double> pixel = coordinates.referencePixel();

    Cube<double> map(3, shape(0), shape(1), 0.0);
    for(pixel(1) = 0.0; pixel(1) < shape(1); ++pixel(1))
      {
        for(pixel(0) = 0.0; pixel(0) < shape(0); ++pixel(0))
          {
            // CoodinateSystem::toWorld()
            // DEC range [-pi/2,pi/2]
            // RA range [-pi,pi]
            if(coordinates.toWorld(world, pixel))
              {
                MVDirection mvITRF(convertor(world).getValue());
                map(0, pixel(0), pixel(1)) = mvITRF(0);
                map(1, pixel(0), pixel(1)) = mvITRF(1);
                map(2, pixel(0), pixel(1)) = mvITRF(2);
              }
          }
      }

    return map;
  }

  void LOFARATerm::initInstrument(const MeasurementSet &ms)
  {
    // Get station names and positions in ITRF coordinates.
    ROMSAntennaColumns antenna(ms.antenna());
    ROMSObservationColumns observation(ms.observation());
    ASSERT(observation.nrow() > 0);
    ASSERT(!observation.flagRow()(0));

    // Get instrument name.
    String name = observation.telescopeName()(0);

    // Get station positions.
    MVPosition centroid;
    vector<Station> stations(antenna.nrow());
    for(uint i = 0; i < stations.size(); ++i)
      {
        // Get station name and ITRF position.
        MPosition position =
          MPosition::Convert(antenna.positionMeas()(i),
                             MPosition::ITRF)();

        // Store station information.
        stations[i] = initStation(ms, i, antenna.name()(i), position);

        ASSERT(stations[i].nField() > 0);

        // Update ITRF centroid.
        centroid += position.getValue();
      }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition position;
    if(MeasTable::Observatory(position, name))
      {
        position = MPosition::Convert(position, MPosition::ITRF)();
      }
    else
      {
        LOG_INFO("LOFARATerm initInstrument "
                 "Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        position = MPosition(centroid, MPosition::ITRF);
      }

    m_instrument = Instrument(name, position, stations.begin(), stations.end());
  }

  Station LOFARATerm::initStation(const MeasurementSet &ms,
                                  uint id, const String &name, const MPosition &position) const
  {
    if(!ms.keywordSet().isDefined("LOFAR_ANTENNA_FIELD"))
      {
        LOG_WARN("LOFARATerm initStation "
                 "Antenna " << name << ": no LOFAR_ANTENNA_FIELD!");
        return Station(name, position);
      }

    Table tab_field(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
    tab_field = tab_field(tab_field.col("ANTENNA_ID") == static_cast<Int>(id));

    const uLong nFields = tab_field.nrow();
    if(nFields < 1 || nFields > 2)
      {
        LOG_WARN("LOFARATerm initStation "
                 "Antenna " << name << " consists of an incompatible number"
                 " of antenna fields. Beam model simulation will not work for this"
                 " antenna.");
        return Station(name, position);
      }

    ROScalarColumn<String> c_name(tab_field, "NAME");
    ROArrayQuantColumn<double> c_position(tab_field, "POSITION",
                                          "m");
    ROArrayQuantColumn<double> c_axes(tab_field, "COORDINATE_AXES",
                                      "m");
    ROArrayQuantColumn<double> c_tile_offset(tab_field,
                                             "TILE_ELEMENT_OFFSET", "m");
    ROArrayQuantColumn<double> c_offset(tab_field, "ELEMENT_OFFSET",
                                        "m");
    ROArrayColumn<Bool> c_flag(tab_field, "ELEMENT_FLAG");

    AntennaField field[2];
    for(uLong i = 0; i < nFields; ++i)
      {
        // Read antenna field center.
        Vector<Quantum<double> > aips_position =
          c_position(i);
        ASSERT(aips_position.size() == 3);

        Vector3 position = {{aips_position(0).getValue(),
                             aips_position(1).getValue(), aips_position(2).getValue()}};

        // Read antenna field coordinate axes.
        Matrix<Quantum<double> > aips_axes = c_axes(i);
        ASSERT(aips_axes.shape().isEqual(IPosition(2, 3, 3)));

        Vector3 P = {{aips_axes(0, 0).getValue(), aips_axes(1, 0).getValue(),
                      aips_axes(2, 0).getValue()}};
        Vector3 Q = {{aips_axes(0, 1).getValue(), aips_axes(1, 1).getValue(),
                      aips_axes(2, 1).getValue()}};
        Vector3 R = {{aips_axes(0, 2).getValue(), aips_axes(1, 2).getValue(),
                      aips_axes(2, 2).getValue()}};

        // Store information as AntennaField.
        field[i] = AntennaField(c_name(i), position, P, Q, R);

        if(c_name(i) != "LBA")
          {
            // Read tile configuration for HBA antenna fields.
            Matrix<Quantum<double> > aips_offset =
              c_tile_offset(i);
            ASSERT(aips_offset.nrow() == 3);

            const uLong nElement = aips_offset.ncolumn();
            for(uLong j = 0; j < nElement; ++j)
              {
                Vector3 offset = {{aips_offset(0, j).getValue(),
                                   aips_offset(1, j).getValue(),
                                   aips_offset(2, j).getValue()}};

                field[i].appendTileElement(offset);
              }
          }

        // Read element position offsets and flags.
        Matrix<Quantum<double> > aips_offset = c_offset(i);
        Matrix<Bool> aips_flag = c_flag(i);

        const uLong nElement = aips_offset.ncolumn();
        ASSERT(aips_offset.shape().isEqual(IPosition(2, 3, nElement)));
        ASSERT(aips_flag.shape().isEqual(IPosition(2, 2, nElement)));

        for(uLong j = 0; j < nElement; ++j)
          {
            AntennaField::Element element;
            element.offset[0] = aips_offset(0, j).getValue();
            element.offset[1] = aips_offset(1, j).getValue();
            element.offset[2] = aips_offset(2, j).getValue();
            element.flag[0] = aips_flag(0, j);
            element.flag[1] = aips_flag(1, j);

            field[i].appendElement(element);
          }
      }

    return (nFields == 1 ? Station(name, position, field[0])
            : Station(name, position, field[0], field[1]));
  }

  void LOFARATerm::initReferenceFreq(const MeasurementSet &ms, uint idDataDescription)
  {
    // Read polarization id and spectral window id.
    ROMSDataDescColumns desc(ms.dataDescription());
    ASSERT(desc.nrow() > idDataDescription);
    ASSERT(!desc.flagRow()(idDataDescription));

    const uint idWindow = desc.spectralWindowId()(idDataDescription);

    // Get spectral information.
    ROMSSpWindowColumns window(ms.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    m_referenceFreq = window.refFrequency()(idWindow);
  }

  void LOFARATerm::initPhaseReference(const MeasurementSet &ms, uint idField)
  {
    // Get phase center as RA and DEC (J2000).
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));

    m_phaseReference = MDirection::Convert(field.phaseDirMeas(idField),
                                           MDirection::J2000)();
  }

  BeamCoeff::BeamCoeff()
    :   m_center(0.0),
        m_width(1.0)
  {
  }

  void BeamCoeff::load(const Path &path)
  {
    // Open file.
    String expandedPath = path.expandedName();
    ifstream in(expandedPath.c_str());
    if(!in)
      {
        THROW (Exception, "Unable to open beam coefficient file.");
      }

    // Read file header.
    String header, token0, token1, token2, token3, token4, token5;
    getline(in, header);

    uLong nElements, nHarmonics, nPowerTheta, nPowerFreq;
    double freqAvg, freqRange;

    istringstream iss(header);
    iss >> token0 >> nElements >> token1 >> nHarmonics >> token2 >> nPowerTheta
        >> token3 >> nPowerFreq >> token4 >> freqAvg >> token5 >> freqRange;

    if(!in || !iss || token0 != "d" || token1 != "k" || token2 != "pwrT"
       || token3 != "pwrF" || token4 != "freqAvg" || token5 != "freqRange")
      {
        THROW (Exception, "Unable to parse header");
      }

    if(nElements * nHarmonics * nPowerTheta * nPowerFreq == 0)
      {
        THROW (Exception, "The number of coefficients should be"
               " larger than zero.");
      }

    ASSERT(nElements == 2);
    ASSERT(in.good());

    // Allocate coefficient matrix.
    m_center = freqAvg;
    m_width = freqRange;
    m_coeff = Array<DComplex>(IPosition(4, 2, nPowerFreq, nPowerTheta, nHarmonics));

    uLong nCoeff = 0;
    while(in.good())
      {
        // Read line from file.
        String line;
        getline(in, line);

        // Skip lines that contain only whitespace.
        if(line.find_last_not_of(" ", String::npos) == String::npos)
          {
            continue;
          }

        // Parse line.
        uLong element, harmonic, powerTheta, powerFreq;
        double re, im;

        iss.clear();
        iss.str(line);
        iss >> element >> harmonic >> powerTheta >> powerFreq >> re >> im;

        if(!iss || element >= nElements || harmonic >= nHarmonics
           || powerTheta >= nPowerTheta || powerFreq >= nPowerFreq)
          {
            THROW (Exception, "Error reading beam coefficient file.");
          }

        // Store coefficient.
        m_coeff(IPosition(4, element, powerFreq, powerTheta, harmonic)) = DComplex(re, im);

        // Update coefficient counter.
        ++nCoeff;
      }

    if(!in.eof())
      {
        THROW (Exception, "Error reading beam coefficient"
               " file.");
      }

    if(nCoeff != nElements * nHarmonics * nPowerTheta * nPowerFreq)
      {
        THROW (Exception, "The number of coefficients"
               " specified in the header does not match the number of coefficients"
               " in the file.");
      }
  }

  AntennaField::AntennaField(const String &name, const Vector3 &position,
                             const Vector3 &p, const Vector3 &q, const Vector3 &r)
    :   m_name(name),
        m_position(position)
  {
    m_axes[P] = p;
    m_axes[Q] = q;
    m_axes[R] = r;
  }

  const String &AntennaField::name() const
  {
    return m_name;
  }

  const Vector3 &AntennaField::position() const
  {
    return m_position;
  }

  const Vector3 &AntennaField::axis(Axis axis) const
  {
    return m_axes[axis];
  }

  Bool AntennaField::isHBA() const
  {
    return m_name != "LBA";
  }

  void AntennaField::appendTileElement(const Vector3 &offset)
  {
    m_tileElements.push_back(offset);
  }

  void AntennaField::appendElement(const Element &element)
  {
    m_elements.push_back(element);
  }

  Station::Station(const String &name, const MPosition &position)
    :   m_name(name),
        m_position(position)
  {
  }

  Station::Station(const String &name, const MPosition &position,
                   const AntennaField &field0)
    :   m_name(name),
        m_position(position)
  {
    m_fields.push_back(field0);
  }

  Station::Station(const String &name, const MPosition &position,
                   const AntennaField &field0, const AntennaField &field1)
    :   m_name(name),
        m_position(position)
  {
    m_fields.push_back(field0);
    m_fields.push_back(field1);
  }

  const String &Station::name() const
  {
    return m_name;
  }

  const MPosition &Station::position() const
  {
    return m_position;
  }

  bool Station::isPhasedArray() const
  {
    return !m_fields.empty();
  }

  uint Station::nField() const
  {
    return m_fields.size();
  }

  const AntennaField &Station::field(uint i) const
  {
    return m_fields[i];
  }

  Instrument::Instrument(const String &name, const MPosition &position)
    :   m_name(name),
        m_position(position)
  {
  }

  const String &Instrument::name() const
  {
    return m_name;
  }

  const MPosition &Instrument::position() const
  {
    return m_position;
  }

  uint Instrument::nStations() const
  {
    return m_stations.size();
  }

  const Station &Instrument::station(uint i) const
  {
    return m_stations[i];
  }

  const Station &Instrument::station(const String &name) const
  {
    map<String, uint>::const_iterator it = m_index.find(name);
    if(it == m_index.end())
      {
        THROW (Exception, "Unknown station: " + name);
      }

    return m_stations[it->second];
  }

  void Instrument::append(const Station &station)
  {
    m_stations.push_back(station);
  }

} // namespace LOFAR
