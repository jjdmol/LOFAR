//# LofarATermOld.h: Compute the LOFAR beam response on the sky.
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

#ifndef LOFAR_LOFARFT_LOFARATERMOLD_H
#define LOFAR_LOFARFT_LOFARATERMOLD_H

#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

namespace casa
{
  class DirectionCoordinate;
  class MEpoch;
  class MeasurementSet;
  class Path;
}

namespace LOFAR
{
  struct Vector3
  {
    const double &operator[](uint i) const
    { return __data[i]; }

    double &operator[](uint i)
    { return __data[i]; }

    double  __data[3];
  };

  class AntennaField
  {
  public:
    enum Axis
    {
      P,
      Q,
      R,
      N_Axis
    };

    struct Element
    {
      Vector3 offset;
      bool    flag[2];
    };

    AntennaField()
    {
    }

    AntennaField(const casa::String &name, const Vector3 &position,
                 const Vector3 &p,
                 const Vector3 &q, const Vector3 &r);

    const casa::String &name() const;
    const Vector3 &position() const;
    const Vector3 &axis(Axis axis) const;

    bool isHBA() const;

    void appendTileElement(const Vector3 &offset);
    inline uint nTileElement() const;
    inline const Vector3 &tileElement(uint i) const;

    void appendElement(const Element &element);
    inline uint nElement() const;
    inline const Element &element(uint i) const;

  private:
    casa::String    m_name;
    Vector3         m_position;
    Vector3         m_axes[N_Axis];
    vector<Vector3> m_tileElements;
    vector<Element> m_elements;
  };

  class Station
  {
  public:
    Station()
    {
    }

    Station(const casa::String &name, const casa::MPosition &position);
    Station(const casa::String &name, const casa::MPosition &position,
            const AntennaField &field0);
    Station(const casa::String &name, const casa::MPosition &position,
            const AntennaField &field0, const AntennaField &field1);

    const casa::String &name() const;
    const casa::MPosition &position() const;

    bool isPhasedArray() const;
    uint nField() const;
    const AntennaField &field(uint i) const;

  private:
    casa::String            m_name;
    casa::MPosition         m_position;
    vector<AntennaField>    m_fields;
  };

  class Instrument
  {
  public:
    Instrument()
    {
    }

    Instrument(const casa::String &name, const casa::MPosition &position);

    template <typename T>
    Instrument(const casa::String &name, const casa::MPosition &position,
               T first, T last);

    const casa::String &name() const;
    const casa::MPosition &position() const;

    uint nStations() const;
    const Station &station(uint i) const;
    const Station &station(const casa::String &name) const;

    void append(const Station &station);

  private:
    casa::String              m_name;
    casa::MPosition           m_position;
    map<casa::String, uint>   m_index;
    vector<Station>           m_stations;
  };

  inline uint AntennaField::nTileElement() const
  {
    return m_tileElements.size();
  }

  const Vector3 &AntennaField::tileElement(uint i) const
  {
    return m_tileElements[i];
  }

  inline uint AntennaField::nElement() const
  {
    return m_elements.size();
  }

  inline const AntennaField::Element &AntennaField::element(uint i) const
  {
    return m_elements[i];
  }

  template <typename T>
  Instrument::Instrument(const casa::String &name,
                         const casa::MPosition &position,
                         T first, T last)
    :   m_name(name),
        m_position(position),
        m_stations(first, last)
  {
  }

  class BeamCoeff
  {
  public:
    BeamCoeff();

    void load(const casa::Path &path);

    // Center frequency used to scale frequency to range [-1.0, 1.0].
    double center() const
    {
      return m_center;
    }

    // Width used to scale frequency to range [-1.0, 1.0].
    double width() const
    {
      return m_width;
    }

    uint nElements() const
    {
      return m_coeff.shape()(0);
    }

    uint nPowerFreq() const
    {
      return m_coeff.shape()(1);
    }

    uint nPowerTheta() const
    {
      return m_coeff.shape()(2);
    }

    uint nHarmonics() const
    {
      return m_coeff.shape()(3);
    }

    casa::DComplex operator()(uint i, uint freq, uint theta, uint harmonic) const
    {
      return m_coeff(casa::IPosition(4, i, freq, theta, harmonic));
    }

  private:
    double                      m_center, m_width;
    casa::Array<casa::DComplex> m_coeff;
  };

  class LofarATermOld
  {
  public:
    LofarATermOld(const casa::MeasurementSet &ms,
               const casa::String &beamElementPath);

    vector<casa::Cube<casa::Complex> > evaluate(const casa::IPosition &shape,
      const casa::DirectionCoordinate &coordinates,
      uint station,
      const casa::MEpoch &epoch,
      const casa::Vector<casa::Double> &freq,
      bool normalize = false) const;

  private:
    casa::Array<casa::DComplex>
    normalize(const casa::Array<casa::DComplex> &response) const;

    casa::Cube<casa::Double>
    computeITRFMap(const casa::DirectionCoordinate &coordinates,
      const casa::IPosition &shape,
      casa::MDirection::Convert convertor) const;

    casa::Array<casa::DComplex> evaluateStationBeam(const Station &station,
      const Vector3 &refDelay,
      const Vector3 &refTile,
      const casa::Cube<casa::Double> &map,
      const casa::Vector<casa::Double> &freq) const;

    casa::Cube<casa::DComplex> evaluateTileArrayFactor(const AntennaField &field,
      const Vector3 &reference,
      const casa::Cube<casa::Double> &map,
      const casa::Vector<casa::Double> &freq) const;

    casa::Array<casa::DComplex> evaluateElementBeam(const BeamCoeff &coeff,
      const AntennaField &field,
      const casa::Cube<casa::Double> &map,
      const casa::Vector<casa::Double> &freq) const;

    void initInstrument(const casa::MeasurementSet &ms);

    Station initStation(const casa::MeasurementSet &ms,
      uint id,
      const casa::String &name,
      const casa::MPosition &position) const;

    void initReferenceDirections(const casa::MeasurementSet &ms, uint idField);

    void initReferenceFreq(const casa::MeasurementSet &ms,
      uint idDataDescription);

    BeamCoeff        m_coeffLBA, m_coeffHBA;
    casa::MDirection m_refDelay, m_refTile;
    double           m_refFreq;
    Instrument       m_instrument;
  };
} // namespace LOFAR

#endif
