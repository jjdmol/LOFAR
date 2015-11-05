//# makebeamimage.cc: Generate images of the beam response of multiple stations
//# for a given MS.
//#
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>
#include <LofarFT/Exceptions.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/Package__Version.h>
#include <Common/InputParSet.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <images/Images/PagedImage.h>
#include <images/Images/HDF5Image.h>
#include <images/Images/ImageFITSConverter.h>

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

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Timer.h>
#include <casa/OS/PrecTimer.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

using namespace casa;
using namespace LOFAR;
using LOFAR::operator<<;

Quantity readQuantity (const String& in)
{
  Quantity res;
  if (!Quantity::read(res, in)) {
    THROW(AWImagerException, in + " is an illegal quantity");
  }
  return res;
}

int main(int argc, char *argv[])
{
    TEST_SHOW_VERSION(argc, argv, LofarFT);
    INIT_LOGGER(basename(string(argv[0])));
    Version::show<LofarFTVersion>(cout);

    LOFAR::InputParSet inputs;
    inputs.create("ms", "", "Name of input MeasurementSet", "string");
    inputs.create("stations", "[0]", "IDs of stations to process",
        "int vector");
    inputs.create("cellsize", "60arcsec", "Angular pixel size",
        "quantity string");
    inputs.create("size", "256", "Number of pixels along each axis", "int");
    inputs.create("offset", "0s", "Time offset from the start of the MS",
        "quantity string");
    inputs.create("frames", "0", "Number of images that will be generated for"
        " each station (equally spaced over the duration of the MS)", "int");
    inputs.create("abs", "false", "If set to true, store the absolute value of"
        " the beam response instead of the complex value (intended for use with"
        " older versions of casaviewer)", "bool");
    inputs.readArguments(argc, argv);

    String msName = inputs.getString("ms");
    if(msName.empty())
    {
        THROW(AWImagerException, "An MS name must be provided, for example:"
            " ms=test.ms");
    }

    vector<int> stationID(inputs.getIntVector("stations"));
    Quantity cellsize = readQuantity(inputs.getString("cellsize"));
    size_t size = max(inputs.getInt("size"), 1);
    Quantity offset = readQuantity(inputs.getString("offset"));
    size_t nFrames = max(inputs.getInt("frames"), 1);
    Bool abs = inputs.getBool("abs");

    // ---------------------------------------------------------------------- //

    MeasurementSet ms(msName);

    uInt idObservation = 0;
    uInt idField = 0;
    uInt idDataDescription = 0;

    // Read number of stations.
    ROMSAntennaColumns antenna(ms.antenna());
    uInt nStation = antenna.nrow();

    // Filter invalid station IDs.
    vector<unsigned int> filteredID;
    for(vector<int>::const_iterator it = stationID.begin(),
        end = stationID.end(); it != end; ++it)
    {
        if(*it >= 0 && static_cast<size_t>(*it) < nStation)
        {
            filteredID.push_back(*it);
        }
    }
    
    // Read phase reference direction.
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));
    MDirection refDir = field.phaseDirMeas(idField);

    // Read reference frequency.
    ROMSDataDescColumns desc(ms.dataDescription());
    ASSERT(desc.nrow() > idDataDescription);
    ASSERT(!desc.flagRow()(idDataDescription));
    uInt idWindow = desc.spectralWindowId()(idDataDescription);

    ROMSSpWindowColumns window(ms.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    double refFreq = window.refFrequency()(idWindow);

    // Read reference time.
    Table msView =
        ms(ms.col("OBSERVATION_ID") == static_cast<Int>(idObservation)
        && ms.col("FIELD_ID") == static_cast<Int>(idField)
        && ms.col("DATA_DESC_ID") == static_cast<Int>(idDataDescription));

    Table tab_sorted = msView.sort("TIME", Sort::Ascending,
        Sort::HeapSort | Sort::NoDuplicates);

    ROScalarColumn<Double> c_time(tab_sorted, "TIME");
    Vector<Double> time = c_time.getColumn();

    // ---------------------------------------------------------------------- //

    MDirection refDirJ2000(MDirection::Convert(refDir, MDirection::J2000)());
    Quantum<Vector<Double> > angles = refDirJ2000.getAngle();

    double ra = angles.getBaseValue()(0);
    double dec = angles.getBaseValue()(1);
    double delta = cellsize.getValue("rad");

    // Construct DirectionCoordinate instance.
    Matrix<Double> xform(2,2);
    xform = 0.0; xform.diagonal() = 1.0;
    DirectionCoordinate coordinates(MDirection::J2000,
                            Projection(Projection::SIN),
                            ra, dec,
                            -delta, delta,
                            xform,
                            size / 2, size / 2);

    // ---------------------------------------------------------------------- //

    LofarATerm aTerm(ms, Record());
      
    IPosition shape(2, size, size);
    aTerm.setDirection(coordinates, shape);
      
    Vector<Double> freq(1, refFreq);
 
    Quantity refTime(time(0), "s");
    refTime = refTime + offset;

    Quantity deltaTime((time(time.size() - 1) - time(0) - offset.getValue("s"))
        / (nFrames - 1), "s");

    cout << "computing..." << flush;
    for(size_t j = 0; j < nFrames; ++j)
    {
        MEpoch refEpoch;
        refEpoch.set(refTime);
        aTerm.setEpoch(refEpoch);
        refTime = refTime + deltaTime;

        for(vector<unsigned int>::const_iterator it = filteredID.begin(),
            end = filteredID.end(); it != end; ++it)
        {
            vector<Cube<Complex> > response = aTerm.evaluate(*it, freq, freq);

            std::ostringstream oss;
            oss << "beam-id-" << *it;
            if(nFrames > 1)
            {
                oss << "-frame-" << j;
            }
            oss << ".img";

            if(abs)
            {
                Cube<Float> ampl(amplitude(response[0]));
                store(coordinates, ampl, oss.str());
            }
            else
            {
                store(coordinates, response[0], oss.str());
            }
        }

        cout << "." << flush;
    }
    cout << " done." << endl;

    return 0;
}
