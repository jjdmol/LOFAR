//# LofarATerm.cc: Compute the LOFAR beam response on the sky.
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
//# $Id: LOFARATerm.cc 18046 2011-05-19 20:58:40Z diepen $

#include <lofar_config.h>
#include <LofarFT/LofarATerm.h>

#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <ElementResponse/ElementResponse.h>

#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>

using namespace casa;

namespace LOFAR
{

namespace
{
  Array<DComplex> computeFieldArrayFactor(const BBS::Station::ConstPtr &station,
    uint idField, const LofarATerm::ITRFDirectionMap &map,
    const Vector<Double> &freq, const Vector<Double> &reference);

  Cube<DComplex>
  computeTileArrayFactor(const BBS::AntennaField::ConstPtr &field,
    const LofarATerm::ITRFDirectionMap &map, const Vector<Double> &freq);

  struct ElementLBA;
  struct ElementHBA;

  template <typename T_ELEMENT>
  Array<DComplex>
  computeElementResponse(const BBS::AntennaField::ConstPtr &field,
    const LofarATerm::ITRFDirectionMap &map, const Vector<Double> &freq);

  void rescale(Array<DComplex> &response);
  void rescale(Cube<DComplex> &response);

  vector<Cube<Complex> > asVector(Array<DComplex> &response);
  vector<Matrix<Complex> > asVector(Cube<DComplex> &response);
}

  LofarATerm::LofarATerm(const MeasurementSet& ms, const casa::Record& parameters)
  {
    itsInstrument = BBS::readInstrument(ms);
    itsRefDelay = MDirection::Convert(BBS::readDelayReference(ms),
      MDirection::J2000)();
    itsRefTile = MDirection::Convert(BBS::readTileReference(ms),
      MDirection::J2000)();
     itsDirectionCoordinates = 0;
    itsParameters = parameters;
    itsApplyBeam = True;
    if (itsParameters.fieldNumber("applyBeam") > -1) itsApplyBeam = itsParameters.asBool("applyBeam");
    itsApplyIonosphere = False;
    if (itsParameters.fieldNumber("applyIonosphere") > -1) itsApplyIonosphere = itsParameters.asBool("applyIonosphere");
    if (itsApplyIonosphere)
    {
      String parmdbname = ms.tableName() + "/instrument";
      cout << parmdbname << endl;
      initParmDB(parmdbname);
      cout << cal_pp_names << endl;
    }
  }

  void LofarATerm::initParmDB(const casa::String  &parmdbname)
  {
    this->pdb = new LOFAR::BBS::ParmFacade (parmdbname);
    std::string prefix = "Piercepoint:X:";
    std::vector<std::string> v = this->pdb->getNames(prefix + "*");
    this->cal_pp_names = Vector<String>(v.size());
    this->cal_pp = Matrix<Double>(3,v.size());
    this->tec_white = Vector<Double>(v.size());

    //strip cal_pp_names from prefix
    for (uint i=0; i<v.size(); i++)
    {
      this->cal_pp_names[i] = v[i].substr(prefix.length());
    }

  }

  void LofarATerm::setDirection(const casa::DirectionCoordinate &coordinates, const IPosition &shape) {
    itsDirectionCoordinates = &coordinates;
    itsShape = &shape;
  }


  void LofarATerm::setEpoch( const MEpoch &epoch )
  {
    if (this->itsDirectionCoordinates) itsITRFDirectionMap = makeDirectionMap(*itsDirectionCoordinates, *itsShape, epoch);
    if (this->itsApplyIonosphere)
    {
      this->time = epoch.get(casa::Unit("s")).getValue();
      this->r0 = get_parmvalue("r_0");
      this->beta = get_parmvalue("beta");
      this->height = get_parmvalue("height");
      for(uint i = 0; i < this->cal_pp_names.size(); ++i) {
        this->cal_pp(0, i) = get_parmvalue("Piercepoint:X:" + this->cal_pp_names(i));
        this->cal_pp(1, i) = get_parmvalue("Piercepoint:Y:" + this->cal_pp_names(i));
        this->cal_pp(2, i) = get_parmvalue("Piercepoint:Z:" + this->cal_pp_names(i));
        this->tec_white(i) = get_parmvalue("TECfit_white:0:" + this->cal_pp_names(i));
      }
    }
  }

  double LofarATerm::get_parmvalue( std::string parmname )
  {
    double freq = 50e6; // the ionospheric parameters are not frequency dependent, so just pick an arbitrary frequency
                         // this frequency should be within the range specified in the parmdbname
                         // this range is again quite arbitrary
    casa::Record result = this->pdb->getValues (parmname, freq, freq+0.5, 1.0, this->time, this->time + 0.5, 1.0 );
    casa::Array<double> parmvalues;
    result.subRecord(parmname).get("values", parmvalues);
    return parmvalues(IPosition(2,0,0));
  }


  LofarATerm::ITRFDirectionMap
  LofarATerm::makeDirectionMap(const DirectionCoordinate &coordinates,
    const IPosition &shape, const MEpoch &epoch) const
  {
    AlwaysAssert(shape[0] > 0 && shape[1] > 0, SynthesisError);

    ITRFDirectionMap map;
    map.epoch = epoch;

    // Create conversion engine J2000 -> ITRF at epoch.
    MDirection::Convert convertor = MDirection::Convert(MDirection::J2000,
      MDirection::Ref(MDirection::ITRF,
      MeasFrame(epoch, itsInstrument->position())));

    MVDirection mvRefDelay = convertor(itsRefDelay).getValue();
    map.refDelay[0] = mvRefDelay(0);
    map.refDelay[1] = mvRefDelay(1);
    map.refDelay[2] = mvRefDelay(2);

    MVDirection mvRefTile = convertor(itsRefTile).getValue();
    map.refTile[0] = mvRefTile(0);
    map.refTile[1] = mvRefTile(1);
    map.refTile[2] = mvRefTile(2);

    MDirection world;
    casa::Vector<Double> pixel = coordinates.referencePixel();

    Cube<Double> mapITRF(3, shape[0], shape[1], 0.0);
    for(pixel[1] = 0.0; pixel(1) < shape[1]; ++pixel[1])
    {
      for(pixel[0] = 0.0; pixel[0] < shape[0]; ++pixel[0])
      {
        // CoodinateSystem::toWorld()
        // DEC range [-pi/2,pi/2]
        // RA range [-pi,pi]
        if(coordinates.toWorld(world, pixel))
        {
          MVDirection mvITRF = convertor(world).getValue();
          mapITRF(0, pixel[0], pixel[1]) = mvITRF(0);
          mapITRF(1, pixel[0], pixel[1]) = mvITRF(1);
          mapITRF(2, pixel[0], pixel[1]) = mvITRF(2);
        }
      }
    }

    map.directions.reference(mapITRF);
    return map;
  }

  vector<Cube<Complex> > LofarATerm::evaluate(uint idStation,
    const Vector<Double> &freq,
    const Vector<Double> &reference, bool normalize) const
  {
    const ITRFDirectionMap &map = itsITRFDirectionMap;
    AlwaysAssert(idStation < itsInstrument->nStations(), SynthesisError);
    BBS::Station::ConstPtr station = itsInstrument->station(idStation);

    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    Array<DComplex> E(IPosition(4, nX, nY, 4, nFreq), DComplex(0.0, 0.0));
    for(uint i = 0; i < station->nField(); ++i)
    {
      BBS::AntennaField::ConstPtr field = station->field(i);

      // Compute element beam.
      Array<DComplex> element;
      if(field->isHBA())
      {
        element.reference(computeElementResponse<ElementHBA>(field, map, freq));
      }
      else
      {
        element.reference(computeElementResponse<ElementLBA>(field, map, freq));
      }

      // Compute antenna field array factor.
      Array<DComplex> fieldAF = computeFieldArrayFactor(station, i, map, freq,
        reference);

      // Multiply the element or tile response by the antenna field array
      // factor.
      for(uint j = 0; j < 2; ++j)
      {
        IPosition start(4, 0, 0, j, 0);
        IPosition end(4, nX - 1, nY - 1, j, nFreq - 1);
        Array<DComplex> slice = E(start, end);

        IPosition startAF(4, 0, 0, 0, 0);
        IPosition endAF(4, nX - 1, nY - 1, 0, nFreq - 1);
        slice += fieldAF(startAF, endAF) * element(start, end);
      }

      for(uint j = 2; j < 4; ++j)
      {
        IPosition startAF(4, 0, 0, 1, 0);
        IPosition endAF(4, nX - 1, nY - 1, 1, nFreq - 1);

        IPosition start(4, 0, 0, j, 0);
        IPosition end(4, nX - 1, nY - 1, j, nFreq - 1);
        Array<DComplex> slice = E(start, end);
        slice += fieldAF(startAF, endAF) * element(start, end);
      }
    }

    if(normalize)
    {
      rescale(E);
    }

    return asVector(E);
  }



  vector<Matrix<Complex> > LofarATerm::evaluateStationScalarFactor(const uint idStation,
    const Vector<Double> &freq, const Vector<Double> &reference, bool normalize) const
  {
    AlwaysAssert(idStation < itsInstrument->nStations(), SynthesisError);

    const uint idPolarization = 0;
    const ITRFDirectionMap &map = itsITRFDirectionMap;

    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    Cube<DComplex> SF(nX, nY, nFreq, DComplex(1.0, 0.0));

    if(itsApplyBeam)
    {
      BBS::Station::ConstPtr station = itsInstrument->station(idStation);

      if(station->nField() > 0)
      {
        Array<DComplex> AF = computeFieldArrayFactor(station, 0, map, freq,
          reference);
        for(uint i = 1; i < station->nField(); ++i)
        {
          AF += computeFieldArrayFactor(station, i, map, freq, reference);
        }

        IPosition start(4, 0, 0, idPolarization, 0);
        IPosition end(4, nX - 1, nY - 1, idPolarization, nFreq - 1);
        Cube<DComplex> AFslice =
          AF(start, end).reform(IPosition(3, nX, nY, nFreq));
        SF *= AFslice;
      }
    }

    if (itsApplyIonosphere)
    {
      Cube<DComplex> IF = evaluateIonosphere(idStation, freq);
      SF *= IF;
    }

    if(normalize)
    {
      rescale(SF);
    }

    return asVector(SF);
  }

  vector<Matrix<Complex> > LofarATerm::evaluateArrayFactor(uint idStation,
    uint idPolarization, const Vector<Double> &freq,
    const Vector<Double> &reference, bool normalize) const
  {
    AlwaysAssert(idStation < itsInstrument->nStations(), SynthesisError);
    AlwaysAssert(idPolarization < 2, SynthesisError);

    const ITRFDirectionMap &map = itsITRFDirectionMap;
    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    BBS::Station::ConstPtr station = itsInstrument->station(idStation);
    if(station->nField() == 0)
    {
      Cube<DComplex> slice(nX, nY, nFreq, DComplex(1.0, 0.0));
      return asVector(slice);
    }

    Array<DComplex> AF = computeFieldArrayFactor(station, 0, map, freq,
      reference);
    for(uint i = 1; i < station->nField(); ++i)
    {
      AF += computeFieldArrayFactor(station, i, map, freq, reference);
    }

    IPosition start(4, 0, 0, idPolarization, 0);
    IPosition end(4, nX - 1, nY - 1, idPolarization, nFreq - 1);
    Cube<DComplex> slice = AF(start, end).reform(IPosition(3, nX, nY, nFreq));

    if(normalize)
    {
      rescale(slice);
    }

    return asVector(slice);
  }

  vector<Cube<Complex> > LofarATerm::evaluateElementResponse(uint idStation,
    uint idField, const Vector<Double> &freq,
    bool normalize) const
  {
    AlwaysAssert(idStation < itsInstrument->nStations(), SynthesisError);

    const ITRFDirectionMap &map = itsITRFDirectionMap;
    BBS::AntennaField::ConstPtr field =
      itsInstrument->station(idStation)->field(idField);

    Array<DComplex> response;
    if(field->isHBA())
    {
      response.reference(computeElementResponse<ElementHBA>(field, map, freq));
    }
    else
    {
      response.reference(computeElementResponse<ElementLBA>(field, map, freq));
    }

    if(normalize)
    {
      rescale(response);
    }

    return asVector(response);
  }

  Cube<DComplex> LofarATerm::evaluateIonosphere(uint station, const Vector<Double> &freq) const
  {
    const ITRFDirectionMap &map = itsITRFDirectionMap;

    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];

    const uint nFreq = freq.size();

    const casa::MVPosition p = this->itsInstrument->station(station)->position().getValue();

    const double earth_ellipsoid_a = 6378137.0;
    const double earth_ellipsoid_a2 = earth_ellipsoid_a*earth_ellipsoid_a;
    const double earth_ellipsoid_b = 6356752.3142;
    const double earth_ellipsoid_b2 = earth_ellipsoid_b*earth_ellipsoid_b;
    const double earth_ellipsoid_e2 = (earth_ellipsoid_a2 - earth_ellipsoid_b2) / earth_ellipsoid_a2;

    const double ion_ellipsoid_a = earth_ellipsoid_a + height;
    const double ion_ellipsoid_a2_inv = 1.0 / (ion_ellipsoid_a * ion_ellipsoid_a);
    const double ion_ellipsoid_b = earth_ellipsoid_b + height;
    const double ion_ellipsoid_b2_inv = 1.0 / (ion_ellipsoid_b * ion_ellipsoid_b);


    double x = p(0)/ion_ellipsoid_a;
    double y = p(1)/ion_ellipsoid_a;
    double z = p(2)/ion_ellipsoid_b;
    double c = x*x + y*y + z*z - 1.0;

    casa::Cube<double> piercepoints(4, nX, nY, 0.0);

    for(uint i = 0 ; i < nX; ++i)
    {
      for(uint j = 0 ; j < nY; ++j)
      {
        double dx = map.directions(0,i,j) / ion_ellipsoid_a;
        double dy = map.directions(1,i,j) / ion_ellipsoid_a;
        double dz = map.directions(2,i,j) / ion_ellipsoid_b;
        double a = dx*dx + dy*dy + dz*dz;
        double b = x*dx + y*dy  + z*dz;
        double alpha = (-b + std::sqrt(b*b - a*c))/a;
        piercepoints(0, i, j) = p(0) + alpha*map.directions(0,i,j);
        piercepoints(1, i, j) = p(1) + alpha*map.directions(1,i,j);
        piercepoints(2, i, j) = p(2) + alpha*map.directions(2,i,j);
        double normal_x = piercepoints(0, i, j) * ion_ellipsoid_a2_inv;
        double normal_y = piercepoints(1, i, j) * ion_ellipsoid_a2_inv;
        double normal_z = piercepoints(2, i, j) * ion_ellipsoid_b2_inv;
        double norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
        double norm_normal = std::sqrt(norm_normal2);
        double sin_lat2 = normal_z*normal_z / norm_normal2;

        double g = 1.0 - earth_ellipsoid_e2*sin_lat2;
        double sqrt_g = std::sqrt(g);

        double M = earth_ellipsoid_b2 / ( earth_ellipsoid_a * g * sqrt_g );
        double N = earth_ellipsoid_a / sqrt_g;

        double local_ion_ellipsoid_e2 = (M-N) / ((M+height)*sin_lat2 - N - height);
        double local_ion_ellipsoid_a = (N+height) * std::sqrt(1.0 - local_ion_ellipsoid_e2*sin_lat2);
        double local_ion_ellipsoid_b = local_ion_ellipsoid_a*std::sqrt(1.0 - local_ion_ellipsoid_e2);

        double z_offset = ((1.0-earth_ellipsoid_e2)*N + height - (1.0-local_ion_ellipsoid_e2)*(N+height)) * std::sqrt(sin_lat2);

        double x1 = p(0)/local_ion_ellipsoid_a;
        double y1 = p(1)/local_ion_ellipsoid_a;
        double z1 = (p(2)-z_offset)/local_ion_ellipsoid_b;
        double c1 = x1*x1 + y1*y1 + z1*z1 - 1.0;

        dx = map.directions(0,i,j) / local_ion_ellipsoid_a;
        dy = map.directions(1,i,j) / local_ion_ellipsoid_a;
        dz = map.directions(2,i,j) / local_ion_ellipsoid_b;
        a = dx*dx + dy*dy + dz*dz;
        b = x1*dx + y1*dy  + z1*dz;
        alpha = (-b + std::sqrt(b*b - a*c1))/a;

        piercepoints(0, i, j) = p(0) + alpha*map.directions(0,i,j);
        piercepoints(1, i, j) = p(1) + alpha*map.directions(1,i,j);
        piercepoints(2, i, j) = p(2) + alpha*map.directions(2,i,j);
        normal_x = piercepoints(0, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
        normal_y = piercepoints(1, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
        normal_z = (piercepoints(2, i, j)-z_offset) / (local_ion_ellipsoid_b * local_ion_ellipsoid_b);
        norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
        norm_normal = std::sqrt(norm_normal2);
        double cos_za_rec = norm_normal / (map.directions(0,i,j)*normal_x + map.directions(1,i,j)*normal_y + map.directions(2,i,j)*normal_z);
        piercepoints(3, i, j) = cos_za_rec;

      }
    }
    Matrix<Double> tec(nX, nY, 0.0);

    Double r0sqr = r0 * r0;
    Double beta_2 = 0.5 * beta;
    for(uint i = 0 ; i < nX; ++i)
    {
      for(uint j = 0 ; j < nY; ++j)
      {
        for(uint k = 0 ; k < cal_pp_names.size(); ++k)
        {
          Double dx = cal_pp(0, k) - piercepoints(0,i,j);
          Double dy = cal_pp(1, k) - piercepoints(1,i,j);
          Double dz = cal_pp(2, k) - piercepoints(2,i,j);
          Double weight = pow((dx * dx + dy * dy + dz * dz) / r0sqr, beta_2);
          tec(i,j) += weight * tec_white(k);
        }
        tec(i,j) *= (-0.5 * piercepoints(3,i,j));
      }
    }

    Cube<DComplex> IF(nX, nY, nFreq, DComplex(0.0, 0.0));
    for (uint i = 0; i < freq.size(); ++i)
    {
      Double a = (8.44797245e9 / freq[i]);
      for(uint j = 0 ; j < nX; ++j)
      {
        for(uint k = 0 ; k < nY; ++k)
        {
          Double phase = -tec(j,k) * a; // SvdT : removed minus sign, but still believe it should be there
                                        // put it back again
          IF(j,k,i) = DComplex(cos(phase), sin(phase));
        }
      }
    }
    return IF;
  }


namespace
{
  vector<Cube<Complex> > asVector(Array<DComplex> &response)
  {
    vector<Cube<Complex> > result;
    for(ArrayIterator<DComplex> it(response, 3); !it.pastEnd(); it.next())
    {
      Cube<Complex> slice(it.array().shape());
      convertArray(slice, it.array());
      result.push_back(slice);
    }

    return result;
  }

  vector<Matrix<Complex> > asVector(Cube<DComplex> &response)
  {
    vector<Matrix<Complex> > result;
    for(ArrayIterator<DComplex> it(response, 2); !it.pastEnd(); it.next())
    {
      Matrix<Complex> slice(it.array().shape());
      convertArray(slice, it.array());
      result.push_back(slice);
    }

    return result;
  }

  void rescale(Array<DComplex> &response)
  {
    AlwaysAssert(response.ndim() == 4, SynthesisError);
    AlwaysAssert(response.shape()[2] == 4, SynthesisError);

    const uint nX = response.shape()[0];
    const uint nY = response.shape()[1];
    const uint centerX = nX / 2;
    const uint centerY = nY / 2;

    DComplex invXX, invXY, invYX, invYY;
    for(ArrayIterator<DComplex> it(response, 3); !it.pastEnd(); it.next())
    {
      Cube<DComplex> slice(it.array());

      // Compute the inverse of the Jones matrix at the central pixel.
      DComplex det = slice(centerX, centerY, 0) * slice(centerX, centerY, 3)
        - slice(centerX, centerY, 1) * slice(centerX, centerY, 2);
      invXX = slice(centerX, centerY, 3) / det;
      invXY = -slice(centerX, centerY, 1) / det;
      invYX = -slice(centerX, centerY, 2) / det;
      invYY = slice(centerX, centerY, 0) / det;

      // Apply the inverse of the Jones matrix at the central pixel to all Jones
      // matrices.
      Matrix<DComplex> XX = slice(IPosition(3, 0, 0, 0),
        IPosition(3, nX - 1, nY - 1, 0)).nonDegenerate();
      Matrix<DComplex> XY = slice(IPosition(3, 0, 0, 1),
        IPosition(3, nX - 1, nY - 1, 1)).nonDegenerate();
      Matrix<DComplex> YX = slice(IPosition(3, 0, 0, 2),
        IPosition(3, nX - 1, nY - 1, 2)).nonDegenerate();
      Matrix<DComplex> YY = slice(IPosition(3, 0, 0, 3),
        IPosition(3, nX - 1, nY - 1, 3)).nonDegenerate();

      DComplex normXX, normXY, normYX, normYY;
      for(uint j = 0; j < nY; ++j)
      {
        for(uint i = 0; i < nX; ++i)
        {
          normXX = invXX * XX(i, j) + invXY * YX(i, j);
          normXY = invXX * XY(i, j) + invXY * YY(i, j);
          normYX = invYX * XX(i, j) + invYY * YX(i, j);
          normYY = invYX * XY(i, j) + invYY * YY(i, j);

          XX(i, j) = normXX;
          XY(i, j) = normXY;
          YX(i, j) = normYX;
          YY(i, j) = normYY;
        }
      }
    }
  }

  void rescale(Cube<DComplex> &response)
  {
    const uint centerX = response.shape()[0] / 2;
    const uint centerY = response.shape()[1] / 2;
    for(ArrayIterator<DComplex> it(response, 2); !it.pastEnd(); it.next())
    {
      Matrix<DComplex> slice(it.array());
      slice /= slice(centerX, centerY);
    }
  }

  Array<DComplex> computeFieldArrayFactor(const BBS::Station::ConstPtr &station,
    uint idField, const LofarATerm::ITRFDirectionMap &map,
    const Vector<Double> &freq, const Vector<Double> &reference)
  {
    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    // Account for the case where the delay reference position is not equal to
    // the field center (only applies to core HBA fields).
    BBS::AntennaField::ConstPtr field = station->field(idField);
    const BBS::Vector3 &fieldCenter = field->position();

    MVPosition delayCenter = station->position().getValue();
    BBS::Vector3 offsetShift = {{fieldCenter[0] - delayCenter(0),
      fieldCenter[1] - delayCenter(1),
      fieldCenter[2] - delayCenter(2)}};

    // Compute field array factors.
    Array<DComplex> AF(IPosition(4, nX, nY, 2, nFreq), DComplex(0.0, 0.0));
    Cube<DComplex> weight(nX, nY, nFreq);

    for(uint i = 0; i < field->nElement(); ++i)
    {
      const BBS::AntennaField::Element &element = field->element(i);
      if(element.flag[0] && element.flag[1])
      {
        continue;
      }

      // Compute the offset relative to the delay center.
      BBS::Vector3 offset = {{element.offset[0] + offsetShift[0],
        element.offset[1] + offsetShift[1],
        element.offset[2] + offsetShift[2]}};

      // Compute the delay for a plane wave approaching from the delay
      // reference direction with respect to the element position.
      double delay0 = (map.refDelay[0] * offset[0]
        + map.refDelay[1] * offset[1]
        + map.refDelay[2] * offset[2]) / C::c;

      for(uint k = 0; k < nY; ++k)
      {
        for(uint j = 0; j < nX; ++j)
        {
          // Compute the delay for a plane wave approaching from the direction
          // of interest with respect to the element position.
          double delay = (map.directions(0, j, k) * offset[0]
            + map.directions(1, j, k) * offset[1]
            + map.directions(2, j, k) * offset[2]) / C::c;

          for(uint l = 0; l < nFreq; ++l)
          {
            double shift = C::_2pi * (freq[l] * delay - reference[l] * delay0);
            weight(j, k, l) = DComplex(cos(shift), sin(shift));
          }
        }
      }

      if(!element.flag[0])
      {
        IPosition start(4, 0, 0, 0, 0);
        IPosition end(4, nX - 1, nY - 1, 0, nFreq - 1);
        Array<DComplex> slice = AF(start, end).reform(weight.shape());
        slice += weight;
      }

      if(!element.flag[1])
      {
        IPosition start(4, 0, 0, 1, 0);
        IPosition end(4, nX - 1, nY - 1, 1, nFreq - 1);
        Array<DComplex> slice = AF(start, end).reform(weight.shape());
        slice += weight;
      }
    }

    // Normalize.
    if(station->nActiveElement() > 0)
    {
      AF /= static_cast<Double>(station->nActiveElement());
    }

    if(field->isHBA())
    {
      // Compute tile array factor.
      Cube<DComplex> tileAF = computeTileArrayFactor(field, map, freq);

      // Multiply the station array factor by the tile array factor.
      for(uint i = 0; i < 2; ++i)
      {
        IPosition start(4, 0, 0, i, 0);
        IPosition end(4, nX - 1, nY - 1, i, nFreq - 1);
        Array<DComplex> slice = AF(start, end).reform(tileAF.shape());
        slice *= tileAF;
      }
    }

    return AF;
  }

  Cube<DComplex>
  computeTileArrayFactor(const BBS::AntennaField::ConstPtr &field,
    const LofarATerm::ITRFDirectionMap &map, const Vector<Double> &freq)
  {
    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    Cube<DComplex> AF(nX, nY, nFreq, DComplex(0.0, 0.0));
    for(uint j = 0; j < nY; ++j)
    {
      for(uint i = 0; i < nX; ++i)
      {
        // Instead of computing a phase shift for the pointing direction and a
        // phase shift for the direction of interest and then computing the
        // difference, compute the resultant phase shift in one go. Here we make
        // use of the relation a . b + a . c = a . (b + c). The sign of k is
        // related to the sign of the phase shift.
        double k[3];
        k[0] = map.directions(0, i, j) - map.refTile[0];
        k[1] = map.directions(1, i, j) - map.refTile[1];
        k[2] = map.directions(2, i, j) - map.refTile[2];

        for(uint l = 0; l < field->nTileElement(); ++l)
        {
          // Compute the effective delay for a plane wave approaching from the
          // direction of interest with respect to the position of element i
          // when beam forming in the reference direction using time delays.
          const BBS::Vector3 &offset = field->tileElement(l);
          double delay = (k[0] * offset[0] + k[1] * offset[1] + k[2]
            * offset[2]) / C::c;

          // Turn the delay into a phase shift.
          for(uint m = 0; m < nFreq; ++m)
          {
            double shift = C::_2pi * freq[m] * delay;
            AF(i, j, m) += DComplex(cos(shift), sin(shift));
          }
        }
      }
    }

    // Normalize.
    if(field->nTileElement() > 0)
    {
      AF /= static_cast<Double>(field->nTileElement());
    }

    return AF;
  }

  struct ElementLBA
  {
    static void response(double freq, double theta, double phi,
      DComplex (&response)[2][2])
    {
      element_response_lba(freq, theta, phi, response);
    }
  };

  struct ElementHBA
  {
    static void response(double freq, double theta, double phi,
      DComplex (&response)[2][2])
    {
      element_response_hba(freq, theta, phi, response);
    }
  };

  template <typename T_ELEMENT>
  Array<DComplex>
  computeElementResponse(const BBS::AntennaField::ConstPtr &field,
    const LofarATerm::ITRFDirectionMap &map, const Vector<Double> &freq)
  {
    const BBS::Vector3 &p = field->axis(BBS::AntennaField::P);
    const BBS::Vector3 &q = field->axis(BBS::AntennaField::Q);
    const BBS::Vector3 &r = field->axis(BBS::AntennaField::R);

    const uint nX = map.directions.shape()[1];
    const uint nY = map.directions.shape()[2];
    const uint nFreq = freq.size();

    DComplex J[2][2];
    Array<DComplex> E(IPosition(4, nX, nY, 4, nFreq), DComplex(0.0, 0.0));
    for(uint j = 0; j < nY; ++j)
    {
      for(uint i = 0; i < nX; ++i)
      {
        BBS::Vector3 target = {{map.directions(0, i, j),
          map.directions(1, i, j), map.directions(2, i, j)}};

        // Check for non-physical directions (the image is square while the
        // projected sky is circular, therefore some image pixels may map to
        // invalid directions.
        if(target[0] == 0.0 && target[1] == 0.0 && target[2] == 0.0)
        {
          continue;
        }

        // Compute the cross product of the NCP and the target direction. This
        // yields a vector tangent to the celestial sphere at the target
        // direction, pointing towards the East (the direction of +Y in the IAU
        // definition, or positive right ascension).
        BBS::Vector3 v1 = {{-target[1], target[0], 0.0}};
        double normv1 = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
        v1[0] /= normv1;
        v1[1] /= normv1;
        v1[2] /= normv1;

        // Compute the cross product of the antenna field normal (R) and the
        // target direction. This yields a vector tangent to the topocentric
        // spherical coordinate system at the target direction, pointing towards
        // the direction of positive phi (which runs East over North around the
        // pseudo zenith).
        BBS::Vector3 v2 = {{r[1] * target[2] - r[2] * target[1],
          r[2] * target[0] - r[0] * target[2],
          r[0] * target[1] - r[1] * target[0]}};
        double normv2 = sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);
        v2[0] /= normv2;
        v2[1] /= normv2;
        v2[2] /= normv2;

        // Compute the cosine and sine of the parallactic angle, i.e. the angle
        // between v1 and v2, both tangent to a latitude circle of their
        // respective spherical coordinate systems.
        double coschi = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
        double sinchi = (v1[1] * v2[2] - v1[2] * v2[1]) * target[0]
          + (v1[2] * v2[0] - v1[0] * v2[2]) * target[1]
          + (v1[0] * v2[1] - v1[1] * v2[0]) * target[2];

        // The input coordinate system is a right handed system with its third
        // axis along the direction of propagation (IAU +Z). The output
        // coordinate system is right handed as well, but its third axis points
        // in the direction of arrival (i.e. exactly opposite).
        //
        // Because the electromagnetic field is always perpendicular to the
        // direction of propagation, we only need to relate the (X, Y) axes of
        // the input system to the corresponding (theta, phi) axes of the output
        // system.
        //
        // To this end, we first rotate the input system around its third axis
        // to align the Y axis with the phi axis. The X and theta axis are
        // parallel after this rotation, but point in opposite directions. To
        // align the X axis with the theta axis, we flip it.
        //
        // The Jones matrix to align the Y axis with the phi axis when these are
        // separated by an angle phi (measured counter-clockwise around the
        // direction of propagation, looking towards the origin), is given by:
        //
        // [ cos(phi)  sin(phi)]
        // [-sin(phi)  cos(phi)]
        //
        // Here, cos(phi) and sin(phi) can be computed directly, without having
        // to compute phi first (see the computation of coschi and sinchi
        // above).
        //
        // Now, sinchi as computed above is opposite to sin(phi), because the
        // direction used in the computation is the direction of arrival instead
        // of the direction of propagation. Therefore, the sign of sinchi needs
        // to be reversed. Furthermore, as explained above, the X axis has to be
        // flipped to align with the theta axis. The Jones matrix returned from
        // this function is therefore given by:
        //
        // [-coschi  sinchi]
        // [ sinchi  coschi]

        // Compute the P and Q coordinate of the direction vector by projecting
        // onto the positive P and Q axis.
        double projectionP = target[0] * p[0] + target[1] * p[1] + target[2]
          * p[2];
        double projectionQ = target[0] * q[0] + target[1] * q[1] + target[2]
          * q[2];

        // Compute the inner product between the antenna field normal (R) and
        // the direction vector to get the cosine of the zenith angle.
        double projectionR = target[0] * r[0] + target[1] * r[1] + target[2]
          * r[2];

        double theta = acos(projectionR);
        double phi = atan2(projectionQ, projectionP);

        // The positive X dipole direction is SW of the reference orientation,
        // which translates to a phi coordinate of 5/4*pi in the topocentric
        // spherical coordinate system. The phi coordinate is corrected for this
        // offset before evaluating the antenna model.
        phi -= 5.0 * C::pi_4;

        for(uint k = 0; k < nFreq; ++k)
        {
          T_ELEMENT::response(freq[k], theta, phi, J);

          E(IPosition(4, i, j, 0, k)) = J[0][0] * -coschi + J[0][1] * sinchi;
          E(IPosition(4, i, j, 1, k)) = J[0][0] *  sinchi + J[0][1] * coschi;
          E(IPosition(4, i, j, 2, k)) = J[1][0] * -coschi + J[1][1] * sinchi;
          E(IPosition(4, i, j, 3, k)) = J[1][0] *  sinchi + J[1][1] * coschi;
        }
      }
    }

    return E;
  }

} // unnamed namespace

} // namespace LOFAR
