//# LofarATermIonosphere.cc: Compute the LOFAR beam response on the sky.
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
#include <LofarFT/LofarATermIonosphere.h>

#include <math.h>

#include <Common/LofarLogger.h>
#include <Common/Exception.h>

#include <ParmDB/ParmDB.h> 
#include <ParmDB/ParmMap.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/Package__Version.h>


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
  LofarATermIonosphere::LofarATermIonosphere(const MeasurementSet& ms, const casa::String& parmdbname)
  {
    initInstrument(ms);
    initParmDB(parmdbname);
    initReferenceFreq(ms, 0);
//    initReferenceDirections(ms, 0);
  }

  vector<Cube<Complex> > LofarATermIonosphere::evaluate(const IPosition &shape,
    const DirectionCoordinate &coordinates,
    uint station,
    const MEpoch &epoch,
    const Vector<Double> &freq,
    bool normalize)
  {
//    AlwaysAssert(station < this->instrument.nStations(), SynthesisError);
//    AlwaysAssert(shape[0] > 0 && shape[1] > 0, SynthesisError);
//    AlwaysAssert(freq.size() > 0, SynthesisError);

    setEpoch(epoch);

    // Create conversion engine (from J2000 -> ITRF).
    MDirection::Convert convertor = MDirection::Convert(MDirection::J2000,
      MDirection::Ref(MDirection::ITRF, MeasFrame(epoch, this->instrument.position())));

    MVDirection mvRefDelay = convertor(this->refDelay).getValue();
    Vector3 refDelay = {{mvRefDelay(0), mvRefDelay(1), mvRefDelay(2)}};

    MVDirection mvRefTile = convertor(this->refTile).getValue();
    Vector3 refTile = {{mvRefTile(0), mvRefTile(1), mvRefTile(2)}};

    // Compute ITRF map.
    LOG_INFO("LofarATermIonosphere::evaluate(): Computing ITRF map...");
    Cube<double> mapITRF = computeITRFMap(coordinates, shape, convertor);
    LOG_INFO("LofarATermIonosphere::evaluate(): Computing ITRF map... done.");
    
    const uint nX = shape[0];
    const uint nY = shape[1];
    const uint nFreq = freq.size();
    
    const casa::MVPosition p = this->instrument.station(station).position().getValue();
    
    const double earth_ellipsoid_a = 6378137.0;
    const double earth_ellipsoid_a2 = earth_ellipsoid_a*earth_ellipsoid_a;
    const double earth_ellipsoid_b = 6356752.3142;
    const double earth_ellipsoid_b2 = earth_ellipsoid_b*earth_ellipsoid_b;
    const double earth_ellipsoid_e2 = (earth_ellipsoid_a2 - earth_ellipsoid_b2) / earth_ellipsoid_a2;
 
    const double ion_ellipsoid_a = earth_ellipsoid_a + this->height;
    const double ion_ellipsoid_a2_inv = 1.0 / (ion_ellipsoid_a * ion_ellipsoid_a);
    const double ion_ellipsoid_b = earth_ellipsoid_b + this->height;
    const double ion_ellipsoid_b2_inv = 1.0 / (ion_ellipsoid_b * ion_ellipsoid_b);


    double x = p(0)/ion_ellipsoid_a;
    double y = p(1)/ion_ellipsoid_a;
    double z = p(2)/ion_ellipsoid_b;
    double c = x*x + y*y + z*z - 1.0;
    
    Cube<double> piercepoints(4, nX, nY, 0.0);

    for(uint i = 0 ; i < nX; ++i) 
    {
      for(uint j = 0 ; j < nY; ++j) 
      {
        double dx = mapITRF(0,i,j) / ion_ellipsoid_a;
        double dy = mapITRF(1,i,j) / ion_ellipsoid_a;
        double dz = mapITRF(2,i,j) / ion_ellipsoid_b;
        double a = dx*dx + dy*dy + dz*dz;
        double b = x*dx + y*dy  + z*dz;
        double alpha = (-b + std::sqrt(b*b - a*c))/a;
        piercepoints(0, i, j) = p(0) + alpha*mapITRF(0,i,j);
        piercepoints(1, i, j) = p(1) + alpha*mapITRF(1,i,j);
        piercepoints(2, i, j) = p(2) + alpha*mapITRF(2,i,j);
        double normal_x = piercepoints(0, i, j) * ion_ellipsoid_a2_inv;
        double normal_y = piercepoints(1, i, j) * ion_ellipsoid_a2_inv;
        double normal_z = piercepoints(2, i, j) * ion_ellipsoid_b2_inv;
        double norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
        double norm_normal = std::sqrt(norm_normal2);
        double sin_lat2 = normal_z*normal_z / norm_normal2;
//         double cos_za = (mapITRF(0,i,j)*normal_x + mapITRF(1,i,j)*normal_y + mapITRF(2,i,j)*normal_z) / norm_normal;
//         piercepoints(3, i, j) = cos_za;

//        casa::MPosition p1(casa::MVPosition(piercepoints(0, i, j), piercepoints(1, i, j), piercepoints(2, i, j)),casa::MPosition::ITRF);
//        positionWGS84 = casa::MPosition::Convert(p1, casa::MPosition::WGS84)();
//        piercepoints(2, i, j) = positionWGS84.getValue().getLength().getValue()-height;

        double g = 1.0 - earth_ellipsoid_e2*sin_lat2;
        double sqrt_g = std::sqrt(g);

        double M = earth_ellipsoid_b2 / ( earth_ellipsoid_a * g * sqrt_g );
        double N = earth_ellipsoid_a / sqrt_g;

        double local_ion_ellipsoid_e2 = (M-N) / ((M+this->height)*sin_lat2 - N - this->height);
        double local_ion_ellipsoid_a = (N+this->height) * std::sqrt(1.0 - local_ion_ellipsoid_e2*sin_lat2);
        double local_ion_ellipsoid_b = local_ion_ellipsoid_a*std::sqrt(1.0 - local_ion_ellipsoid_e2);

        double z_offset = ((1.0-earth_ellipsoid_e2)*N + this->height - (1.0-local_ion_ellipsoid_e2)*(N+this->height)) * std::sqrt(sin_lat2);

        double x1 = p(0)/local_ion_ellipsoid_a;
        double y1 = p(1)/local_ion_ellipsoid_a;
        double z1 = (p(2)-z_offset)/local_ion_ellipsoid_b;
        double c1 = x1*x1 + y1*y1 + z1*z1 - 1.0;

        dx = mapITRF(0,i,j) / local_ion_ellipsoid_a;
        dy = mapITRF(1,i,j) / local_ion_ellipsoid_a;
        dz = mapITRF(2,i,j) / local_ion_ellipsoid_b;
        a = dx*dx + dy*dy + dz*dz;
        b = x1*dx + y1*dy  + z1*dz;
        alpha = (-b + std::sqrt(b*b - a*c1))/a;

        piercepoints(0, i, j) = p(0) + alpha*mapITRF(0,i,j);
        piercepoints(1, i, j) = p(1) + alpha*mapITRF(1,i,j);
        piercepoints(2, i, j) = p(2) + alpha*mapITRF(2,i,j);
        normal_x = piercepoints(0, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
        normal_y = piercepoints(1, i, j) / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
        normal_z = (piercepoints(2, i, j)-z_offset) / (local_ion_ellipsoid_b * local_ion_ellipsoid_b);
        norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
        norm_normal = std::sqrt(norm_normal2);
        double cos_za_rec = norm_normal / (mapITRF(0,i,j)*normal_x + mapITRF(1,i,j)*normal_y + mapITRF(2,i,j)*normal_z);
        piercepoints(3, i, j) = cos_za_rec;

//         p1 = casa::MPosition(casa::MVPosition(piercepoints(0, i, j), piercepoints(1, i, j), piercepoints(2, i, j)),casa::MPosition::ITRF);
//        positionWGS84 = casa::MPosition::Convert(p1, casa::MPosition::WGS84)();
//        piercepoints(0, i, j) = positionWGS84.getValue().getLength().getValue()-height;
//        cout << positionWGS84.getValue().getLength().getValue()-height << endl;

      }
    }
      
    Matrix<Double> tec(nX, nY, 0.0);
    
    Double r0sqr = this->r0 * this->r0;
    Double beta_2 = 0.5 * this->beta;
    
    for(uint i = 0 ; i < nX; ++i) 
    {
      for(uint j = 0 ; j < nY; ++j) 
      {
        for(uint k = 0 ; k < this->cal_pp_names.size(); ++k) 
        {
          Double dx = cal_pp(0, k) - piercepoints(0,i,j);
          Double dy = cal_pp(1, k) - piercepoints(1,i,j);
          Double dz = cal_pp(2, k) - piercepoints(2,i,j);
          Double weight = pow((dx * dx + dy * dy + dz * dz) / r0sqr, beta_2);
          tec(i,j) += weight * this->tec_white(k);
        }
        tec(i,j) *= (-0.5 * piercepoints(3,i,j));
      }
    }
    
    cout << "TEC center pixel: " << tec(nX / 2, nY/ 2 ) << endl;

//    cout << tec << endl;
    

/*    Station s = this->instrument.station(station);
    casa::String parmname = "Piercepoint:X:" + s.name();*/
//    cout << parmname << " " << epoch.get(casa::Unit("s")).getValue() << " " << freq[0] << endl;
//     double time = epoch.get(casa::Unit("s")).getValue();
//      LOFAR::BBS::Box domain(freq[0], 1e6, time+1e4, 100);
//      LOFAR::BBS::ParmMap result;
/*    casa::Record result = this->pdb->getValues (parmname, freq[0], freq[0]+0.5, 1.0, time, time + 0.5, 1.0 );
    casa::Record result1;
    casa::Array<double> parmvalues;
    result.subRecord(parmname).get("values", parmvalues);*/
    
//    cout << a[0] << endl;
    
/*     cout << result.size() << endl;
 
     LOFAR::BBS::ParmValueSet pvs = result[parmname];
     cout << pvs.size() << endl;
     LOFAR::BBS::ParmValue pv = pvs.getParmValue (0);
     cout << pv.nx() << ", " << pv.ny() << endl;*/
/*     casa::Array<double> a = pv.getValues();
     for (uint i=0; i<pv.nx(); i++) 
     {
       for (uint j=0; j<pv.ny(); j++) 
       {
         cout << a(IPosition(2,i,j)) << endl;
       }
     }*/
	 


    //Array<DComplex> response(IPosition(4, nX, nY, 4, nFreq), DComplex(0.0, 0.0));


    // Convert an Array<DComplex> to a vector<Cube<Complex> >.

    vector<Cube<Complex> > tmp;
//    cout << "freq: " << freq[0] << endl;
    tmp.reserve(nFreq);
    for (uint i = 0; i < freq.size(); ++i)
    {
      Double a = (8.44797245e9 / freq[i]);
      Cube<Complex> planef(IPosition(3, nX, nY, 4), Complex(0.0, 0.0));
      for(uint j = 0 ; j < nX; ++j) 
      {
        for(uint k = 0 ; k < nY; ++k) 
        {
          Double phase = -tec(j,k) * a;
          Complex phasor(cos(phase), sin(phase));
          planef(j,k, 0) = phasor;
          planef(j,k, 3) = phasor;
        }
      }
//      cout << planef << endl;
      tmp.push_back(planef);
    }
    return tmp;
  }

  double LofarATermIonosphere::resolution()
  {
    Double station_diam = 700.;                                           // station diameter in meters: To be adapted to the individual station size.
    return ((casa::C::c/this->refFreq)/station_diam)/2.;
  }

  Array<DComplex> LofarATermIonosphere::normalize(const Array<DComplex> &response)
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



  Cube<double> LofarATermIonosphere::computeITRFMap(const DirectionCoordinate &coordinates,
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

  void LofarATermIonosphere::initParmDB(const casa::String  &parmdbname)
  {
//    LOFAR::BBS::ParmDBMeta meta ("casa", parmdbname);
    this->pdb = new LOFAR::BBS::ParmFacade (parmdbname);
    std::string prefix = "Piercepoint:X:";
    std::vector<std::string> v = this->pdb->getNames(prefix + "*");
    this->cal_pp_names = Vector<String>(v.size());
    this->cal_pp = Matrix<Double>(3,v.size());
    this->tec_white = Vector<Double>(v.size());
    for (uint i=0; i<v.size(); i++) 
    {
      this->cal_pp_names[i] = v[i].substr(prefix.length());
    }
    
  }

  void LofarATermIonosphere::initInstrument(const MeasurementSet &ms)
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

//        ASSERT(stations[i].nField() > 0);

        // Update ITRF centroid.
        centroid += position.getValue();
      }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition position;
/*    if(MeasTable::Observatory(position, name))
      {
        position = MPosition::Convert(position, MPosition::ITRF)();
      }
    else*/
      {
        LOG_INFO("LofarATermBeam initInstrument "
                 "Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        position = MPosition(centroid, MPosition::ITRF);
      }

    this->instrument = Instrument(name, position, stations.begin(), stations.end());
  }

  Station LofarATermIonosphere::initStation(const MeasurementSet &ms,
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

    return Station(name, position);
  }

  void LofarATermIonosphere::initReferenceFreq(const MeasurementSet &ms,
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

    this->refFreq = window.refFrequency()(idWindow);
  }
  
  void LofarATermIonosphere::setEpoch( const MEpoch &epoch )
  {
    double time = epoch.get(casa::Unit("s")).getValue();
    if (this->time != time) {
      cout << (time - this->time) << endl;
      this->time = time;
      this->r0 = get_parmvalue("r_0");
      this->beta = get_parmvalue("beta");
      this->height = get_parmvalue("height");
      for(uint i = 0; i < this->cal_pp_names.size(); ++i) {
        this->cal_pp(0, i) = get_parmvalue("Piercepoint:X:" + cal_pp_names(i));
        this->cal_pp(1, i) = get_parmvalue("Piercepoint:Y:" + cal_pp_names(i));
        this->cal_pp(2, i) = get_parmvalue("Piercepoint:Z:" + cal_pp_names(i));
        this->tec_white(i) = get_parmvalue("TECfit_white:0:" + cal_pp_names(i));
      }
    }
  }
  
  double LofarATermIonosphere::get_parmvalue( std::string parmname ) 
  {
    casa::Record result = this->pdb->getValues (parmname, this->refFreq, this->refFreq+0.5, 1.0, this->time, this->time + 0.5, 1.0 );
    casa::Array<double> parmvalues;
    result.subRecord(parmname).get("values", parmvalues);
    return parmvalues(IPosition(2,0,0));
  }

} // namespace LOFAR
