//# ATermLofar.cc: Compute the LOFAR beam response on the sky.
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
#include <LofarFT/ATermLofar.h>

#include <Common/LofarLogger.h>
#include <Common/Exception.h>

#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCPosition.h>
#include <synthesis/TransformMachines/SynthesisError.h>

#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixIter.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>



#include "helper_functions.tcc"

using namespace casa;
using namespace LOFAR::StationResponse;

namespace LOFAR {
namespace LofarFT {

namespace
{
  bool dummy = LOFAR::LofarFT::ATermFactory::instance().registerClass<LOFAR::LofarFT::ATermLofar>("ATermLofar");
  
  /*!
    *  \brief Convert an ITRF position given as a StationResponse::vector3r_t
    *  instance to a casa::MPosition.
    */
  MPosition toMPositionITRF(const vector3r_t &position);

  /*!
    *  \brief Convert a casa::MPosition instance to a
    #  StationResponse::vector3r_t instance.
    */
  vector3r_t fromMPosition(const MPosition &position);

  /*!
    *  \brief Convert a casa::MDirection instance to a
    *  StationResponse::vector3r_t instance.
    */
  vector3r_t fromMDirection(const MDirection &direction);

  /*!
  *  \brief Check if the specified column exists as a column of the specified
  *  table.
  *
  *  \param table The Table instance to check.
  *  \param column The name of the column.
  */
  bool hasColumn(const Table &table, const string &column);

  /*!
  *  \brief Check if the specified sub-table exists as a sub-table of the
  *  specified table.
  *
  *  \param table The Table instance to check.
  *  \param name The name of the sub-table.
  */
  bool hasSubTable(const Table &table, const string &name);

  /*!
  *  \brief Provide access to a sub-table by name.
  *
  *  \param table The Table instance to which the sub-table is associated.
  *  \param name The name of the sub-table.
  */
  Table getSubTable(const Table &table, const string &name);

  /*!
  *  \brief Attempt to read the position of the observatory. If the
  *  observatory position is unknown, the specified default position is
  *  returned.
  *
  *  \param ms MeasurementSet to read the observatory position from.
  *  \param idObservation Identifier that determines of which observation the
  *  observatory position should be read.
  *  \param defaultPosition The position that will be returned if the
  *  observatory position is unknown.
  */
  MPosition readObservatoryPosition(const MeasurementSet &ms,
    unsigned int idObservation, const MPosition &defaultPosition);

  /*!
  *  \brief Read the station beam former reference direction.
  *
  *  \param ms MeasurementSet to read the station beam former reference
  *  direction from.
  *  \param idField Identifier of the field of which the station beam former
  *  reference direction should be read.
  */
  MDirection readDelayReference(const MeasurementSet &ms,
    unsigned int idField);

  /*!
  *  \brief Read the station beam former reference direction.
  *
  *  \param ms MeasurementSet to read the tile beam former reference direction
  *  from.
  *  \param idField Identifier of the field of which the tile beam former
  *  reference direction should be read.
  */
  MDirection readTileReference(const MeasurementSet &ms,
    unsigned int idField);

  /*!
  *  \brief Convert from Array<DComplex> to vector<Cube<Complex> >. The
  *  conversion is applied to the fourth axis, which is the frequency axis (by
  *  convention).
  *
  *  This is a compatibility function that can be removed as soon as the
  *  response of a station is stored as an Array everywhere.
  *
  *  Note that this function also type casts from DComplex to Complex. This is
  *  because outside of LofarATerm, single precision math is used exclusively.
  */
  vector<Cube<Complex> > asVector(Array<DComplex> &response);

  /*!
  *  \brief Convert from Cube<DComplex> to vector<Cube<Complex> >. The
  *  conversion is applied to the third axis, which is the frequency axis (by
  *  convention).
  *
  *  This is a compatibility function that can be removed as soon as the
  *  scalar response of a station is stored as a Cube everywhere.
  *
  *  Note that this function also type casts from DComplex to Complex. This is
  *  because outside of LofarATerm, single precision math is used exclusively.
  */
  vector<Matrix<Complex> > asVector(Cube<DComplex> &response);

  /*!
  *  \brief Normalize the station response such that it is the identity at the
  *  centre.
  */
  void rescale(Array<DComplex> &response);

  /*!
  *  \brief Normalize the scalar station response such that it is 1.0 for the
  *  at the centre.
  */
  void rescale(Cube<DComplex> &response);
} //# unnamed namespace
  
ATermLofar::ATermLofar(const MeasurementSet& ms, const casa::Record& parameters) :
  itsVerbose(parameters.asInt("verbose"))
{
  // Read station information.
  readStations(ms, std::back_inserter(itsStations));

  // Read reference position and directions.
  itsPosition0 = readObservatoryPosition(ms, 0,
    toMPositionITRF(itsStations.front()->position()));
  itsStation0 = readDelayReference(ms, 0);
  itsTile0 = readTileReference(ms, 0);

  itsDirectionCoordinates = 0;

  itsApplyBeam = True;
  if(parameters.fieldNumber("applyBeam") > -1)
  {
    itsApplyBeam = parameters.asBool("applyBeam");
  }

  itsApplyIonosphere = False;
  if (parameters.fieldNumber("applyIonosphere") > -1)
  {
    itsApplyIonosphere = parameters.asBool("applyIonosphere");
  }
  if (itsApplyIonosphere) 
  {
    String parmdbname = ms.tableName() + "/instrument";
    if (itsParameters.fieldNumber("parmdbname") > -1) 
    {
      parmdbname = ms.tableName() + "/" + itsParameters.asString("parmdbname");
    }
    if (itsVerbose) cout << parmdbname << endl;
    initParmDB(parmdbname);
    if (itsVerbose) cout << itsCal_pp_names << endl;
  }
}

void ATermLofar::initParmDB(const casa::String  &parmdbname)
{
  itsPDB = new LOFAR::BBS::ParmFacade (parmdbname);
  std::string prefix = "Piercepoint:X:";
  std::vector<std::string> v = itsPDB->getNames(prefix + "*");
  cout << "Nparm: " << v.size() << endl;
  itsCal_pp_names = Vector<String>(v.size());
  itsCal_pp = Matrix<Double>(3,v.size());
  itsTec_white = Vector<Double>(v.size());
  
  //strip cal_pp_names from prefix
  for (uint i=0; i<v.size(); i++) 
  {
    itsCal_pp_names[i] = v[i].substr(prefix.length());
  }
  
}

void ATermLofar::setDirection(
  const casa::DirectionCoordinate &coordinates, 
  const IPosition &shape) 
{
  itsDirectionCoordinates = &coordinates;
  itsShape = &shape;
}


void ATermLofar::setEpoch( const MEpoch &epoch )
{
  if (itsDirectionCoordinates)
  {
    itsITRFDirectionMap = makeDirectionMap(*itsDirectionCoordinates, 
                                           *itsShape, epoch);    
  }
  if (itsApplyIonosphere) 
  {
    itsTime = epoch.get(casa::Unit("s")).getValue();
    Record parms = itsPDB->getValuesGrid ("*", 0, 1e9, itsTime, itsTime + 0.01);
    itsR0 = get_parmvalue(parms, "r_0");
    itsBeta = get_parmvalue(parms, "beta");
    itsHeight = get_parmvalue(parms, "height");
    for(uint i = 0; i < itsCal_pp_names.size(); ++i) {
      itsCal_pp(0, i) = get_parmvalue(parms, "Piercepoint:X:" + itsCal_pp_names(i));
      itsCal_pp(1, i) = get_parmvalue(parms, "Piercepoint:Y:" + itsCal_pp_names(i));
      itsCal_pp(2, i) = get_parmvalue(parms, "Piercepoint:Z:" + itsCal_pp_names(i));
      itsTec_white(i) = get_parmvalue(parms, "TECfit_white:0:" + itsCal_pp_names(i));

    }
  }
}

double ATermLofar::get_parmvalue( const Record &parms, const std::string &parmname ) 
{
  double r = 0.0;
  if (parms.isDefined(parmname)) {
    casa::Array<double> parmvalues;
    parms.subRecord(parmname).get("values", parmvalues);
    r = parmvalues(IPosition(2,0,0));
  }  
  return r;
}


ATermLofar::ITRFDirectionMap
ATermLofar::makeDirectionMap(
  const DirectionCoordinate &coordinates,
  const IPosition &shape, 
  const MEpoch &epoch) const
{
  ITRFDirectionMap map;

  // Convert from MEpoch to a time in MJD(UTC) in seconds.
  MEpoch mEpochUTC = MEpoch::Convert(epoch, MEpoch::Ref(MEpoch::UTC))();
  MVEpoch mvEpochUTC = mEpochUTC.getValue();
  Quantity qEpochUTC = mvEpochUTC.getTime();
  map.time0 = qEpochUTC.getValue("s");

  // Create conversion engine J2000 => ITRF at the specified epoch.
  MDirection::Convert convertor = MDirection::Convert(MDirection::J2000,
    MDirection::Ref(MDirection::ITRF, MeasFrame(epoch, itsPosition0)));

  // Compute station and tile beam former reference directions in ITRF at the
  // specified epoch.
  map.station0 = fromMDirection(convertor(itsStation0));
  map.tile0 = fromMDirection(convertor(itsTile0));

  // Pre-allocate space for the grid of ITRF directions.
  map.directions.resize(shape);

  // Compute ITRF directions.
  MDirection world;
  Vector<Double> pixel = coordinates.referencePixel();
  for(pixel(1) = 0.0; pixel(1) < shape(1); ++pixel(1))
  {
    for(pixel(0) = 0.0; pixel(0) < shape(0); ++pixel(0))
    {
      // CoordinateSystem::toWorld(): RA range [-pi,pi], DEC range
      // [-pi/2,pi/2].
      if(coordinates.toWorld(world, pixel))
      {
        map.directions(pixel(0), pixel(1)) = fromMDirection(convertor(world));
      }
    }
  }

  return map;
}

vector<Cube<Complex> > ATermLofar::evaluate(
  uint idStation,
  const Vector<Double> &freq, 
  const Vector<Double> &freq0, 
  bool normalize)
  const
{
  AlwaysAssert(idStation < itsStations.size(), SynthesisError);
  Station::ConstPtr station = itsStations[idStation];

  const ITRFDirectionMap &map = itsITRFDirectionMap;
  const uint nX = map.directions.shape()[0];
  const uint nY = map.directions.shape()[1];
  const uint nFreq = freq.size();

  Array<DComplex> result(IPosition(4, nX, nY, 4, nFreq));
  for(ArrayIterator<DComplex> it(result, 3); !it.pastEnd(); it.next())
  {
    const uint idx = it.pos()(3);
    Cube<DComplex> slice(it.array());

    for(uint y = 0; y < nY; ++y)
    {
      for(uint x = 0; x < nX; ++x)
      {
        matrix22c_t response = station->response(map.time0, freq(idx),
          map.directions(x, y), freq0(idx), map.station0, map.tile0);

        slice(x, y, 0) = response[0][0];
        slice(x, y, 1) = response[0][1];
        slice(x, y, 2) = response[1][0];
        slice(x, y, 3) = response[1][1];
      }
    }
  }

  if(normalize)
  {
    rescale(result);
  }

  return asVector(result);
}

Cube<DComplex> ATermLofar::evaluateStationScalarFactor(
  const uint idStation,
  const uint idPolarization, 
  const Vector<Double> &freq,
  const Vector<Double> &freq0, 
  bool normalize) const
{
  AlwaysAssert(idStation < itsStations.size(), SynthesisError);
  AlwaysAssert(idPolarization < 2, SynthesisError);

  const ITRFDirectionMap &map = itsITRFDirectionMap;
  const uint nX = map.directions.shape()[0];
  const uint nY = map.directions.shape()[1];
  const uint nFreq = freq.size();

  // Allocate space for the result and initialize to 1.0. Initialization is
  // important when itsApplyBeam == itsApplyIonosphere == false.
  Cube<DComplex> result(nX, nY, nFreq, DComplex(1.0, 0.0));

  if (itsApplyBeam)
  {
    Station::ConstPtr station = itsStations[idStation];
    for(MatrixIterator<DComplex> it(result); !it.pastEnd(); it.next())
    {
      const uint idx = it.pos()(2);
      Matrix<DComplex> slice(it.matrix());

      for(uint y = 0; y < nY; ++y)
      {
        for(uint x = 0; x < nX; ++x)
        {
          diag22c_t AF = station->arrayFactor(map.time0, freq(idx),
            map.directions(x, y), freq0(idx), map.station0, map.tile0);

          slice(x, y) *= AF[idPolarization];
        }
      }
    }
  }

  if (itsApplyIonosphere)
  {
    Cube<DComplex> IF = evaluateIonosphere(idStation, freq);
    result *= conj(IF);
  }

  if(normalize)
  {
    rescale(result);
  }

  return result;
}

vector<Matrix<Complex> > ATermLofar::evaluateArrayFactor(
  uint idStation,
  uint idPolarization, 
  const Vector<Double> &freq,
  const Vector<Double> &freq0, 
  bool normalize) const
{
  AlwaysAssert(idStation < itsStations.size(), SynthesisError);
  AlwaysAssert(idPolarization < 2, SynthesisError);

  const ITRFDirectionMap &map = itsITRFDirectionMap;
  const uint nX = map.directions.shape()[0];
  const uint nY = map.directions.shape()[1];
  const uint nFreq = freq.size();
  Station::ConstPtr station = itsStations[idStation];

  // Compute the array factor for the requested station and polarization.
  Cube<DComplex> result(nX, nY, nFreq);
  for(MatrixIterator<DComplex> it(result); !it.pastEnd(); it.next())
  {
    Matrix<DComplex> slice(it.matrix());
    const uint idx = it.pos()(2);

    for(uint y = 0; y < nY; ++y)
    {
      for(uint x = 0; x < nX; ++x)
      {
        diag22c_t AF = station->arrayFactor(map.time0, freq(idx),
          map.directions(x, y), freq0(idx), map.station0, map.tile0);

        slice(x, y) = AF[idPolarization];
      }
    }
  }

  if(normalize)
  {
    rescale(result);
  }

  return asVector(result);
}

vector<Cube<Complex> > ATermLofar::evaluateElementResponse(
  uint idStation,
  uint idField, 
  const Vector<Double> &freq, 
  bool normalize) const
{
  AlwaysAssert(idStation < itsStations.size(), SynthesisError);
  Station::ConstPtr station = itsStations[idStation];

  AlwaysAssert(idField < station->nFields(), SynthesisError);
  AntennaField::ConstPtr field = station->field(idField);

  const ITRFDirectionMap &map = itsITRFDirectionMap;
  const uint nX = map.directions.shape()[0];
  const uint nY = map.directions.shape()[1];
  const uint nFreq = freq.size();

  // Compute the element response for the requested antenna field.
  Array<DComplex> result(IPosition(4, nX, nY, 4, nFreq));
  for(ArrayIterator<DComplex> it(result, 3); !it.pastEnd(); it.next())
  {
    const uint idx = it.pos()(3);
    Cube<DComplex> slice(it.array());
    for(uint y = 0; y < nY; ++y)
    {
      for(uint x = 0; x < nX; ++x)
      {
        matrix22c_t response = field->elementResponse(map.time0,
          freq(idx), map.directions(x, y));

        slice(x, y, 0) = response[0][0];
        slice(x, y, 1) = response[0][1];
        slice(x, y, 2) = response[1][0];
        slice(x, y, 3) = response[1][1];
      }
    }
  }

  if(normalize)
  {
    rescale(result);
  }

  return asVector(result);
}

Cube<DComplex> ATermLofar::evaluateIonosphere(
  uint idStation, 
  const Vector<Double> &freq) const
{
  AlwaysAssert(idStation < itsStations.size(), SynthesisError);
  Station::ConstPtr station = itsStations[idStation];
  
  const ITRFDirectionMap &map = itsITRFDirectionMap;

  const uint nX = map.directions.shape()[1];
  const uint nY = map.directions.shape()[2];
  const uint nFreq = freq.size();
  
  const MVPosition p = toMPositionITRF(station->position()).getValue();
  
  const double earth_ellipsoid_a = 6378137.0;
  const double earth_ellipsoid_a2 = earth_ellipsoid_a*earth_ellipsoid_a;
  const double earth_ellipsoid_b = 6356752.3142;
  const double earth_ellipsoid_b2 = earth_ellipsoid_b*earth_ellipsoid_b;
  const double earth_ellipsoid_e2 = (earth_ellipsoid_a2 - earth_ellipsoid_b2) / earth_ellipsoid_a2;

  const double ion_ellipsoid_a = earth_ellipsoid_a + itsHeight;
  const double ion_ellipsoid_a2_inv = 1.0 / (ion_ellipsoid_a * ion_ellipsoid_a);
  const double ion_ellipsoid_b = earth_ellipsoid_b + itsHeight;
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
      double dx = map.directions(i,j)[0] / ion_ellipsoid_a;
      double dy = map.directions(i,j)[1] / ion_ellipsoid_a;
      double dz = map.directions(i,j)[2] / ion_ellipsoid_b;
      double a = dx*dx + dy*dy + dz*dz;
      double b = x*dx + y*dy  + z*dz;
      double alpha = (-b + std::sqrt(b*b - a*c))/a;
      piercepoints(0, i, j) = p(0) + alpha*map.directions(i,j)[0];
      piercepoints(1, i, j) = p(1) + alpha*map.directions(i,j)[1];
      piercepoints(2, i, j) = p(2) + alpha*map.directions(i,j)[2];
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

      double local_ion_ellipsoid_e2 = (M-N) / ((M+itsHeight)*sin_lat2 - N - itsHeight);
      double local_ion_ellipsoid_a = (N+itsHeight) * std::sqrt(1.0 - local_ion_ellipsoid_e2*sin_lat2);
      double local_ion_ellipsoid_b = local_ion_ellipsoid_a*std::sqrt(1.0 - local_ion_ellipsoid_e2);

      double z_offset = ((1.0-earth_ellipsoid_e2)*N + itsHeight - (1.0-local_ion_ellipsoid_e2)*(N + itsHeight)) * std::sqrt(sin_lat2);

      double x1 = p(0)/local_ion_ellipsoid_a;
      double y1 = p(1)/local_ion_ellipsoid_a;
      double z1 = (p(2)-z_offset)/local_ion_ellipsoid_b;
      double c1 = x1*x1 + y1*y1 + z1*z1 - 1.0;

      dx = map.directions(i,j)[0] / local_ion_ellipsoid_a;
      dy = map.directions(i,j)[1] / local_ion_ellipsoid_a;
      dz = map.directions(i,j)[2] / local_ion_ellipsoid_b;
      a = dx*dx + dy*dy + dz*dz;
      b = x1*dx + y1*dy  + z1*dz;
      alpha = (-b + std::sqrt(b*b - a*c1))/a;

      piercepoints(0, i, j) = p(0) + alpha*map.directions(i,j)[0];
      piercepoints(1, i, j) = p(1) + alpha*map.directions(i,j)[1];
      piercepoints(2, i, j) = p(2) + alpha*map.directions(i,j)[2];
      normal_x = piercepoints(0, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
      normal_y = piercepoints(1, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
      normal_z = (piercepoints(2, i, j)-z_offset) / (local_ion_ellipsoid_b * local_ion_ellipsoid_b);
      norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
      norm_normal = std::sqrt(norm_normal2);
      double cos_za_rec = norm_normal / 
        (map.directions(i,j)[0]*normal_x + map.directions(i,j)[1]*normal_y + map.directions(i,j)[2]*normal_z);
      piercepoints(3, i, j) = cos_za_rec;

    }
  }  
  Matrix<Double> tec(nX, nY, 0.0);
  
  Double r0sqr = itsR0 * itsR0;
  Double beta_2 = 0.5 * itsBeta;
  for(uint i = 0 ; i < nX; ++i) 
  {
    for(uint j = 0 ; j < nY; ++j) 
    {
      for(uint k = 0 ; k < itsCal_pp_names.size(); ++k) 
      {
        Double dx = itsCal_pp(0, k) - piercepoints(0,i,j);
        Double dy = itsCal_pp(1, k) - piercepoints(1,i,j);
        Double dz = itsCal_pp(2, k) - piercepoints(2,i,j);
        Double weight = pow((dx * dx + dy * dy + dz * dz) / r0sqr, beta_2);
        tec(i,j) += weight * itsTec_white(k);
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
        Double phase = -tec(j,k) * a; 
        IF(j,k,i) = DComplex(cos(phase), sin(phase));
      }
    }
  }
  return IF;
}


namespace
{
  MPosition toMPositionITRF(const vector3r_t &position)
  {
    MVPosition mvITRF(position[0], position[1], position[2]);
    return MPosition(mvITRF, MPosition::ITRF);
  }

  vector3r_t fromMPosition(const MPosition &position)
  {
    MVPosition mvPosition = position.getValue();
    vector3r_t result = {{mvPosition(0), mvPosition(1), mvPosition(2)}};
    return result;
  }

  vector3r_t fromMDirection(const MDirection &direction)
  {
    MVDirection mvDirection = direction.getValue();
    vector3r_t result = {{mvDirection(0), mvDirection(1), mvDirection(2)}};
    return result;
  }

  bool hasColumn(const Table &table, const string &column)
  {
    return table.tableDesc().isColumn(column);
  }

  bool hasSubTable(const Table &table, const string &name)
  {
    return table.keywordSet().isDefined(name);
  }

  Table getSubTable(const Table &table, const string &name)
  {
    return table.keywordSet().asTable(name);
  }

  MPosition readObservatoryPosition(const MeasurementSet &ms,
    unsigned int idObservation, const MPosition &defaultPosition)
  {
    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    ROMSObservationColumns observation(ms.observation());
    ASSERT(observation.nrow() > idObservation);
    ASSERT(!observation.flagRow()(idObservation));

    // Read observatory name and try to look-up its position.
    const string observatory = observation.telescopeName()(idObservation);

    // Look-up observatory position, default to specified default position.
    MPosition position(defaultPosition);
    MeasTable::Observatory(position, observatory);
    return position;
  }

  MDirection readDelayReference(const MeasurementSet &ms,
    unsigned int idField)
  {
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));

    return field.delayDirMeas(idField);
  }

  MDirection readTileReference(const MeasurementSet &ms,
    unsigned int idField)
  {
    // The MeasurementSet class does not support LOFAR specific columns, so we
    // use ROArrayMeasColumn to read the tile beam reference direction.
    Table tab_field = getSubTable(ms, "FIELD");

    static const String columnName = "LOFAR_TILE_BEAM_DIR";
    if(hasColumn(tab_field, columnName))
    {
      ROArrayMeasColumn<MDirection> c_direction(tab_field, columnName);
      if(c_direction.isDefined(idField))
      {
        return c_direction(idField)(IPosition(1, 0));
      }
    }

    // By default, the tile beam reference direction is assumed to be equal
    // to the station beam reference direction (for backward compatibility,
    // and for non-HBA measurements).
    return readDelayReference(ms, idField);
  }

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

      // Apply the inverse of the Jones matrix at the central pixel to all
      // Jones matrices.
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
} //# unnamed namespace

} // end namespace LofarFT
} // end namespace LOFAR
