//# LofarATermOld.cc: Compute the LOFAR beam response on the sky.
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

#include <lofar_config.h>
#include <LofarFT/LofarATermOld.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>

#include <casa/OS/Path.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MeasurementSet.h>
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
#include <synthesis/MeasurementComponents/SynthesisError.h>

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
#include <iomanip>
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

using namespace casa;

namespace LOFAR
{
  LofarATermOld::LofarATermOld(const MeasurementSet& ms,
                         const String& beamElementPath)
  {
    if (beamElementPath.empty()) {
      m_coeffLBA.load(Path("element_beam_HAMAKER_LBA.coeff"));
      m_coeffHBA.load(Path("element_beam_HAMAKER_HBA.coeff"));
    } else {
      m_coeffLBA.load(Path(beamElementPath + "/element_beam_HAMAKER_LBA.coeff"));
      m_coeffHBA.load(Path(beamElementPath + "/element_beam_HAMAKER_HBA.coeff"));
    }
     //    m_coeffLBA.load(Path("element_beam_LBA.coeff"));
     //    m_coeffHBA.load(Path("element_beam_HBA.coeff"));

    initInstrument(ms);
    initReferenceFreq(ms, 0);
    initReferenceDirections(ms, 0);
  }

  vector<Cube<Complex> > LofarATermOld::evaluate(const IPosition &shape,
    const DirectionCoordinate &coordinates,
    uint station,
    const MEpoch &epoch,
    const Vector<Double> &freq,
    bool normalize) const
  {
    AlwaysAssert(station < m_instrument.nStations(), SynthesisError);
    AlwaysAssert(shape[0] > 0 && shape[1] > 0, SynthesisError);
    AlwaysAssert(freq.size() > 0, SynthesisError);

    // Create conversion engine (from J2000 -> ITRF).
    MDirection::Convert convertor = MDirection::Convert(MDirection::J2000,
      MDirection::Ref(MDirection::ITRF,
      MeasFrame(epoch, m_instrument.position())));

    MVDirection mvRefDelay = convertor(m_refDelay).getValue();
    Vector3 refDelay = {{mvRefDelay(0), mvRefDelay(1), mvRefDelay(2)}};

    MVDirection mvRefTile = convertor(m_refTile).getValue();
    Vector3 refTile = {{mvRefTile(0), mvRefTile(1), mvRefTile(2)}};

    // Compute ITRF map.
    LOG_INFO("LofarATermOld::evaluate(): Computing ITRF map...");
    Cube<double> mapITRF = computeITRFMap(coordinates, shape, convertor);
    LOG_INFO("LofarATermOld::evaluate(): Computing ITRF map... done.");

    // Compute element beam response.
    LOG_INFO("LofarATermOld::evaluate(): Computing station response...");
    Array<DComplex> response =
      evaluateStationBeam(m_instrument.station(station), refDelay, refTile,
        mapITRF, freq);

    // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
//    MDirection world;
//    Vector<double> refPixel = coordinates.referencePixel();

//    cout << "shape: " << shape << " ref. pixel: " << refPixel << endl;
//    coordinates.toWorld(world, refPixel);

//    casa::Quantum<casa::Vector<casa::Double> > refAngles = world.getAngle();
//    double ra = refAngles.getBaseValue()(0);
//    double dec = refAngles.getBaseValue()(1);
//    cout << "ref. world: " << std::setprecision(17) << ra << " " << dec << endl;

//    cout << "station: " << station << endl;
//    cout << "freq: " << std::setprecision(17) << freq << endl;
//    cout << "time: " << std::setprecision(17) << epoch.getValue().getTime("s") << endl;
//    IPosition st(4, refPixel(0), refPixel(1), 0, 0);
//    IPosition en(4, refPixel(0), refPixel(1), 3, freq.size() - 1);
//    Array<DComplex> tmpResponse = response(st, en).nonDegenerate();
//    cout << "response shape: " << tmpResponse.shape() << endl;
//    cout << "response: " << endl << tmpResponse << endl;

//    refPixel = 0.0;
//    coordinates.toWorld(world, refPixel);
//    refAngles = world.getAngle();
//    ra = refAngles.getBaseValue()(0);
//    dec = refAngles.getBaseValue()(1);
//    cout << "0 world: " << std::setprecision(17) << ra << " " << dec << endl;

//    st = IPosition(4, 0, 0, 0, 0);
//    en = IPosition(4, 0, 0, 3, freq.size() - 1);
//    Array<DComplex> tmpResponse2 = response(st, en).nonDegenerate();
//    cout << "response shape: " << tmpResponse2.shape() << endl;
//    cout << "response: " << endl << tmpResponse2 << endl;

//    AlwaysAssert(false, SynthesisError);
    // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG



    //Cyril
    if(normalize)
    {
     response = this->normalize(response);
    }
    LOG_INFO("LofarATermOld::evaluate(): Computing station response... done.");

    // Convert an Array<DComplex> to a vector<Cube<Complex> >.
    vector<Cube<Complex> > tmp;
    tmp.reserve(freq.size());
    for (ArrayIterator<DComplex> iter(response, 3);
         !iter.pastEnd(); iter.next())
    {
      Cube<Complex> planef(iter.array().shape());
      convertArray (planef, iter.array());
      tmp.push_back(planef);
    }

    // if(normalize)
    //   MDirection::Convert convertor = MDirection::Convert(MDirection::J2000, MDirection::Ref(MDirection::ITRF, MeasFrame(epoch, m_instrument.position())));
    // mapITRF = computeITRFMap(coordinates, shape, convertor);
    // Cube<double> mapITRF_center;
    // DirectionCoordinate coordinates_center(coordinates);
    //   Vector<Double> Refpix(2,0.);
    //   coordinates_center.setReferencePixel(Refpix);
    //   mapITRF_center = computeITRFMap(coordinates_center, IPosition(2,1,1), convertor);
    //   for(uInt i = 0; i < freq.size(); ++i){
    // 	{
    //   evaluateStationBeam(m_instrument.station(station), refDelay, refTile,
    //     mapITRF, freq);
    // 	  Cube<Complex> gain(evaluateStationBeam(mapITRF_center, convertor(m_phaseReference), m_instrument.station(station), freq[i]));
    // 	  Matrix<Complex> central_gain(gain.xzPlane(0));
    // 	  // central_gain.resize(2,2,true); //resize does not work:
    // 	  // Central gain  Axis Lengths: [1, 4]  (NB: Matrix in Row/Column order)
    // 	  //   [(-0.0235668,-0.000796029), (-0.0345345,-0.000373378), (0.030112,0.000938836), (-0.0268743,-0.000258621)]
    // 	  // Central gain  Axis Lengths: [2, 2]  (NB: Matrix in Row/Column order)
    // 	  //   [(-0.0235668,-0.000796029), (-0.0345345,-0.000373378)
    // 	  //    (0,0), (0,0)]
    // 	  Matrix<Complex> central_gain_reform(central_gain.reform(IPosition(2,2,2)));
    // 	  Matrix<Complex> central_gain_invert(invert(central_gain_reform));

    // 	  //Cube<Complex> IM=beams[i];
    // 	  for(uInt ii=0;ii<shape[0];++ii)
    // 	    {
    // 	      for(uInt jj=0;jj<shape[1];++jj)
    // 	  	{
    // 	  	  Cube<Complex> pixel(tmp[i](IPosition(3,ii,jj,0),IPosition(3,ii,jj,3)).copy());
    // 		  // cout<<"================="<<pixel<<endl;
    // 		  // cout<<"pixel"<<pixel<<endl;
    // 	  	  Matrix<Complex> pixel_reform(pixel.reform(IPosition(2,2,2)));
    // 		  // cout<<"pixel_reform"<<pixel_reform<<endl;
    // 	  	  Matrix<Complex> pixel_product=product(central_gain_invert,pixel_reform);
    // 		  // cout<<"pixel_product"<<pixel_product<<endl;
    // 		  Matrix<Complex> pixel_product_reform(pixel_product.reform(IPosition(2,1,4)));
    // 		  // cout<<"pixel_product_reform"<<pixel_product_reform<<endl;

    // 		  for(uInt ind=0;ind<4;++ind){tmp[i](ii,jj,ind)=pixel_product_reform(0,ind);};
    // 		    //beams[i](IPosition(3,ii,jj,0),IPosition(3,ii,jj,3))=pixel_product;
    // 					//IM(ii,jj)=pixel_product;
    // 	  	}
    // 	    }
    // 	};

    return tmp;
  }

  Array<DComplex> LofarATermOld::normalize(const Array<DComplex> &response)
    const
  {
    const uint nX = response.shape()[0];
    const uint nY = response.shape()[1];
    const uint nFreq = response.shape()[3];
    AlwaysAssert(response.shape()[2] == 4, SynthesisError);
    AlwaysAssert(nX > 0 && nY > 0 && nFreq > 0, SynthesisError);

    // Cast away const, to be able to use Array<T>::operator(IPosition,
    // IPosition) to extract a slice (for reading).
    Array<DComplex> &__response = const_cast<Array<DComplex>&>(response);

    // Extract beam response for the central pixel at the central frequency.
    IPosition start(4, floor(nX / 2.), floor(nY / 2.), 0, floor(nFreq / 2.));
    IPosition end(4, floor(nX / 2.), floor(nY / 2.), 3, floor(nFreq / 2.));

    // Use assignment operator to force a copy.
    Vector<DComplex> factor;
    factor = __response(start, end).nonDegenerate();

    // Compute the inverse of the reponse.
    Vector<DComplex> inverse(4);
    DComplex determinant = factor(0) * factor(3) - factor(1) * factor(2);
    inverse(0) = factor(3) / determinant;
    inverse(1) = -factor(1) / determinant;
    inverse(2) = -factor(2) / determinant;
    inverse(3) = factor(0) / determinant;

    // Multiply the beam response for all pixels, at all frequencies, by the
    // computed inverse.
    Array<DComplex> XX = __response(IPosition(4, 0, 0, 0, 0),
      IPosition(4, nX - 1, nY - 1, 0, nFreq - 1));
    Array<DComplex> XY = __response(IPosition(4, 0, 0, 1, 0),
      IPosition(4, nX - 1, nY - 1, 1, nFreq - 1));
    Array<DComplex> YX = __response(IPosition(4, 0, 0, 2, 0),
      IPosition(4, nX - 1, nY - 1, 2, nFreq - 1));
    Array<DComplex> YY = __response(IPosition(4, 0, 0, 3, 0),
      IPosition(4, nX - 1, nY - 1, 3, nFreq - 1));

    Array<DComplex> normal(response.shape());
    Array<DComplex> nXX = normal(IPosition(4, 0, 0, 0, 0),
      IPosition(4, nX - 1, nY - 1, 0, nFreq - 1));
    Array<DComplex> nXY = normal(IPosition(4, 0, 0, 1, 0),
      IPosition(4, nX - 1, nY - 1, 1, nFreq - 1));
    Array<DComplex> nYX = normal(IPosition(4, 0, 0, 2, 0),
      IPosition(4, nX - 1, nY - 1, 2, nFreq - 1));
    Array<DComplex> nYY = normal(IPosition(4, 0, 0, 3, 0),
      IPosition(4, nX - 1, nY - 1, 3, nFreq - 1));

    nXX = inverse(0) * XX + inverse(1) * YX;
    nXY = inverse(0) * XY + inverse(1) * YY;
    nYX = inverse(2) * XX + inverse(3) * YX;
    nYY = inverse(2) * XY + inverse(3) * YY;

    return normal;
  }

  Array<DComplex> LofarATermOld::evaluateElementBeam(const BeamCoeff &coeff,
    const AntennaField &field,
    const Cube<double> &map,
    const Vector<double> &freq) const
  {
    const Vector3 &p = field.axis(AntennaField::P);
    const Vector3 &q = field.axis(AntennaField::Q);
    const Vector3 &r = field.axis(AntennaField::R);

    const uint nX = map.shape()[1];
    const uint nY = map.shape()[2];
    const uint nFreq = freq.shape()[0];

    Array<DComplex> beam(IPosition(4, nX, nY, 4, nFreq), DComplex(0.0, 0.0));
    for(uint j = 0; j < nY; ++j)
    {
      for(uint i = 0; i < nX; ++i)
      {
        if(map(0, i, j) == 0.0 && map(1, i, j) == 0.0 && map(2, i, j) == 0.0)
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
        // Correct azimuth for dipole orientation.
        const double phi = az - 3.0 * C::pi_4;

        // NB: The model is parameterized in terms of zenith angle. The
        // appropriate conversion is taken care of below.
        const double theta = C::pi_2 - el;

        // Only compute the beam response for directions above the horizon.
        if(theta < C::pi_2)
        {
          for(uint k = 0; k < nFreq; ++k)
          {
            // J-jones matrix (2x2 complex matrix)
            DComplex J[2][2] = {{0.0, 0.0}, {0.0, 0.0}};

            // NB: The model is parameterized in terms of a normalized
            // frequency in the range [-1, 1]. The appropriate conversion is
            // taken care of below.
            const double normFreq = (freq[k] - coeff.center()) / coeff.width();

            for(uint l = 0; l < coeff.nHarmonics(); ++l)
            {
              // Compute diagonal projection matrix P for the current
              // harmonic.
              DComplex P[2] = {0.0, 0.0};

              DComplex inner[2];
              for(int m = coeff.nPowerTheta() - 1; m >= 0; --m)
              {
                inner[0] = coeff(0, coeff.nPowerFreq() - 1, m, l);
                inner[1] = coeff(1, coeff.nPowerFreq() - 1, m, l);

                for(int n = coeff.nPowerFreq() - 2; n >= 0; --n)
                {
                  inner[0] = inner[0] * normFreq + coeff(0, n, m, l);
                  inner[1] = inner[1] * normFreq + coeff(1, n, m, l);
                }

                P[0] = P[0] * theta + inner[0];
                P[1] = P[1] * theta + inner[1];
              }

              // Compute Jones matrix for this harmonic by rotating P over
              // kappa * phi and add it to the result.
              const double kappa = ((l & 1) == 0 ? 1.0 : -1.0) * (2.0 * l + 1.0);
              const double cphi = cos(kappa * phi);
              const double sphi = sin(kappa * phi);

              J[0][0] += cphi * P[0];
              J[0][1] += -sphi * P[1];
              J[1][0] += sphi * P[0];
              J[1][1] += cphi * P[1];
            }

            beam(IPosition(4, i, j, 0, k)) = J[0][0];
            beam(IPosition(4, i, j, 1, k)) = J[0][1];
            beam(IPosition(4, i, j, 2, k)) = J[1][0];
            beam(IPosition(4, i, j, 3, k)) = J[1][1];
          }
        }
      }
    }

    return beam;
  }

  Array<DComplex> LofarATermOld::evaluateStationBeam(const Station &station,
    const Vector3 &refDelay,
    const Vector3 &refTile,
    const Cube<Double> &map,
    const Vector<Double> &freq) const
  {
    const uint nX = map.shape()[1];
    const uint nY = map.shape()[2];
    const uint nFreq = freq.shape()[0];

    uint countX = 0, countY = 0;
    Array<DComplex> E(IPosition(4, nX, nY, 4, nFreq), DComplex(0.0, 0.0));
    for(uint i = 0; i < station.nField(); ++i)
    {
      const AntennaField &field = station.field(i);

      // Compute element beam.
      LOG_INFO("LofarATermOld::computeStationBeam: Computing element beam...");
      Array<DComplex> beam;
      if(field.isHBA())
      {
        beam = evaluateElementBeam(m_coeffHBA, field, map, freq);
      }
      else
      {
        beam = evaluateElementBeam(m_coeffLBA, field, map, freq);
      }
      LOG_INFO("LofarATermOld::computeStationBeam: Computing element beam... done.");

      if(field.isHBA())
      {
        // Compute tile array factor.
        LOG_INFO("LofarATermOld::computeStationBeam: Computing tile array factor...");
        Cube<DComplex> tileAF = evaluateTileArrayFactor(field, refTile, map,
          freq);
        LOG_INFO("LofarATermOld::computeStationBeam: Computing tile array factor... done.");

        Array<DComplex> tileAF4 = tileAF.reform(IPosition(4, nX, nY, 1, nFreq));

        // Multiply the element beam by the tile array factor.
        for(uint j = 0; j < 4; ++j)
        {
          IPosition start(4, 0, 0, j, 0);
          IPosition end(4, nX - 1, nY - 1, j, nFreq - 1);

          Array<DComplex> plane = beam(start, end);
          plane *= tileAF4;
        }
      }

      LOG_INFO("LofarATermOld::computeStationBeam: Computing station array factor...");

      // Account for the case where the delay reference position is not equal to
      // the field center (only applies to core HBA fields).
      const Vector3 &fieldCenter = field.position();
      MVPosition delayCenter = station.position().getValue();
      Vector3 offsetShift = {{fieldCenter[0] - delayCenter(0),
                              fieldCenter[1] - delayCenter(1),
                              fieldCenter[2] - delayCenter(2)}};

      // Compute field array factors.
      Cube<DComplex> fieldAFX(nX, nY, nFreq, DComplex(0.0, 0.0));
      Cube<DComplex> fieldAFY(nX, nY, nFreq, DComplex(0.0, 0.0));
      Cube<DComplex> phase(nX, nY, nFreq, DComplex(0.0, 0.0));

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

        // Compute the delay for a plane wave approaching from the delay
        // reference direction with respect to the element position.
        double delay0 = (refDelay[0] * offset[0] + refDelay[1] * offset[1]
          + refDelay[2] * offset[2]) / casa::C::c;
        double shift0 = C::_2pi * m_refFreq * delay0;

        for(uint y = 0; y < nY; ++y)
        {
          for(uint x = 0; x < nX; ++x)
          {
            // Compute the delay for a plane wave approaching from the direction
            // of interest with respect to the element position.
            double delay = (map(0, x, y) * offset[0]
                            + map(1, x, y) * offset[1]
                            + map(2, x, y) * offset[2]) / casa::C::c;

            for(uint k = 0; k < nFreq; ++k)
            {
              double shift = C::_2pi * freq[k] * delay - shift0;
              phase(x, y, k) = DComplex(cos(shift), sin(shift));
            }
          }
        }

        if(!element.flag[0])
        {
          fieldAFX += phase;
          ++countX;
        }

        if(!element.flag[1])
        {
          fieldAFY += phase;
          ++countY;
        }
      }

      LOG_INFO("LofarATermOld::computeStationBeam: Computing station array factor... done.");
      Array<DComplex> fieldAFX4 = fieldAFX.reform(IPosition(4, nX, nY, 1, nFreq));
      for(uint k = 0; k < 2; ++k)
      {
        IPosition start(4, 0, 0, k, 0);
        IPosition end(4, nX - 1, nY - 1, k, nFreq - 1);
        Array<DComplex> plane = E(start, end);
        plane += fieldAFX4 * beam(start, end);
      }

      Array<DComplex> fieldAFY4 = fieldAFY.reform(IPosition(4, nX, nY, 1, nFreq));
      for(uint k = 2; k < 4; ++k)
      {
        IPosition start(4, 0, 0, k, 0);
        IPosition end(4, nX - 1, nY - 1, k, nFreq - 1);
        Array<DComplex> plane = E(start, end);
        plane += fieldAFY4 * beam(start, end);
      }
    } // fields

    // Normalize.
    if(countX > 0)
    {
      IPosition start(4, 0, 0, 0, 0);
      IPosition end(4, nX - 1, nY - 1, 1, nFreq - 1);
      Array<DComplex> plane = E(start, end);
      plane /= static_cast<Double>(countX);
    }

    if(countY > 0)
    {
      IPosition start(4, 0, 0, 2, 0);
      IPosition end(4, nX - 1, nY - 1, 3, nFreq - 1);
      Array<DComplex> plane = E(start, end);
      plane /= static_cast<Double>(countY);
    }

    return E;
  }

  Cube<DComplex> LofarATermOld::evaluateTileArrayFactor(const AntennaField &field,
    const Vector3 &reference,
    const Cube<Double> &map,
    const Vector<Double> &freq) const
  {
    const uint nX = map.shape()[1];
    const uint nY = map.shape()[2];
    const uint nFreq = freq.shape()[0];

    Cube<DComplex> factor(nX, nY, nFreq, DComplex(0.0, 0.0));
    for(uint y = 0; y < nY; ++y)
    {
      for(uint x = 0; x < nX; ++x)
      {
        // Instead of computing a phase shift for the pointing direction and a
        // phase shift for the direction of interest and then computing the
        // difference, compute the resultant phase shift in one go. Here we make
        // use of the relation a . b + a . c = a . (b + c). The sign of k is
        // related to the sign of the phase shift.
        double k[3];
        k[0] = map(0, x, y) - reference[0];
        k[1] = map(1, x, y) - reference[1];
        k[2] = map(2, x, y) - reference[2];

        for(uint j = 0; j < field.nTileElement(); ++j)
        {
          // Compute the effective delay for a plane wave approaching from the
          // direction of interest with respect to the position of element i
          // when beam forming in the reference direction using time delays.
          const Vector3 &offset = field.tileElement(j);
          double delay = (k[0] * offset[0] + k[1] * offset[1] + k[2] * offset[2])
            / C::c;

          // Turn the delay into a phase shift.
          for(uint k = 0; k < nFreq; ++k)
          {
            double shift = C::_2pi * freq[k] * delay;
            factor(x, y, k) += DComplex(cos(shift), sin(shift));
          }
        }
      }
    }

    // Normalize.
    if(field.nTileElement() > 0)
    {
      factor /= static_cast<Double>(field.nTileElement());
    }

    return factor;
  }

  Cube<double> LofarATermOld::computeITRFMap(const DirectionCoordinate &coordinates,
    const IPosition &shape,
    MDirection::Convert convertor) const
  {
    MDirection world;
    Vector<double> pixel = coordinates.referencePixel();

    Cube<double> map(3, shape[0], shape[1], 0.0);
    for(pixel[1] = 0.0; pixel(1) < shape[1]; ++pixel[1])
      {
        for(pixel[0] = 0.0; pixel[0] < shape[0]; ++pixel[0])
          {
            // CoodinateSystem::toWorld()
            // DEC range [-pi/2,pi/2]
            // RA range [-pi,pi]
            if(coordinates.toWorld(world, pixel))
              {
                MVDirection mvITRF(convertor(world).getValue());
                map(0, pixel[0], pixel[1]) = mvITRF(0);
                map(1, pixel[0], pixel[1]) = mvITRF(1);
                map(2, pixel[0], pixel[1]) = mvITRF(2);
              }
          }
      }

    return map;
  }

  void LofarATermOld::initInstrument(const MeasurementSet &ms)
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
        LOG_INFO("LofarATermOld initInstrument "
                 "Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        position = MPosition(centroid, MPosition::ITRF);
      }

    m_instrument = Instrument(name, position, stations.begin(), stations.end());
  }

  Station LofarATermOld::initStation(const MeasurementSet &ms,
    uint id,
    const String &name,
    const MPosition &position) const
  {
    AlwaysAssert(ms.keywordSet().isDefined("LOFAR_ANTENNA_FIELD"), SynthesisError);

    Table tab_field(ms.keywordSet().asTable("LOFAR_ANTENNA_FIELD"));
    tab_field = tab_field(tab_field.col("ANTENNA_ID") == static_cast<Int>(id));

    const uLong nFields = tab_field.nrow();
    AlwaysAssert(nFields == 1 || nFields == 2, SynthesisError);

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

        Vector3 position = {{aips_position[0].getValue(),
                             aips_position[1].getValue(),
                             aips_position[2].getValue()}};

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

  void LofarATermOld::initReferenceDirections(const MeasurementSet &ms,
    uint idField)
  {
    // Get phase center as RA and DEC (J2000).
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));

    m_refDelay = MDirection::Convert(field.delayDirMeas(idField),
      MDirection::J2000)();

    // By default, the tile beam reference direction is assumed to be equal
    // to the station beam reference direction (for backward compatibility,
    // and for non-HBA measurements).
    m_refTile = m_refDelay;

    // The MeasurementSet class does not support LOFAR specific columns, so we
    // use ROArrayMeasColumn to read the tile beam reference direction.
    Table tab_field(ms.keywordSet().asTable("FIELD"));
    static const String columnName = "LOFAR_TILE_BEAM_DIR";
    if(tab_field.tableDesc().isColumn(columnName))
    {
      ROArrayMeasColumn<MDirection> c_direction(tab_field, columnName);
      if(c_direction.isDefined(idField))
      {
        m_refTile = MDirection::Convert(c_direction(idField)(IPosition(1, 0)),
          MDirection::J2000)();
      }
    }
  }

  void LofarATermOld::initReferenceFreq(const MeasurementSet &ms,
    uint idDataDescription)
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

    m_refFreq = window.refFrequency()(idWindow);
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
    cout<<"Reading "<<expandedPath<<endl;
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

  AntennaField::AntennaField(const String &name,
    const Vector3 &position,
    const Vector3 &p,
    const Vector3 &q,
    const Vector3 &r)
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

  Station::Station(const String &name,
    const MPosition &position)
    :   m_name(name),
        m_position(position)
  {
  }

  Station::Station(const String &name,
    const MPosition &position,
    const AntennaField &field0)
    :   m_name(name),
        m_position(position)
  {
    m_fields.push_back(field0);
  }

  Station::Station(const String &name,
    const MPosition &position,
    const AntennaField &field0,
    const AntennaField &field1)
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

  Instrument::Instrument(const String &name,
    const MPosition &position)
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
