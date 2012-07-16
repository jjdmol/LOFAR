//  Small tool that patches the frequency in a casa image
//
//  File:         patchCasaFreq.cc
//  Author:       Sven Duscha (duscha@astron.nl)
//  Date:         2011-12-12
//  Last change:  2011-12-12

#include <lofar_config.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/Exception.h>

// STL/C++ includes
#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

// Boost
#include <boost/lexical_cast.hpp>             // convert string to number

// casacore includes
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumnFunc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TableRecord.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <images/Images/PagedImage.h>                       // we need to open the image to determine patch centre direction
#include <ms/MeasurementSets/MSSpWindowColumns.h>

//using namespace casa;
using namespace std;
using namespace casa;
using namespace LOFAR;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

double patchImageFreq(const string &imageName, double reffreq);
void usage(const char *programname);

int main(int argc, char **argv)
{
  string filename="";
  double originalfrequency=0, frequency=0;

  if(argc < 3)        // if not enough parameters given, display usage information
  {
      usage(argv[0]);
      return 0;
  }
  else      // Handle file arguments: MS image image (and options, e.g. -w 512)
  {
    filename=argv[1];
    frequency=boost::lexical_cast<double>(argv[2]);

    originalfrequency=patchImageFreq(filename, frequency);
    
    cout << "Patched " << filename << " with freq " << frequency << " (original freq: " <<
      originalfrequency << " Hz)" << endl;
  }

  return 0;
}


double patchImageFreq(const string &imageName, double reffreq)
{
    Double imageRestfreq=0;
    IPosition shape(1);                 // define shape for a one-dimensional array
    shape(0)=1;                   
    Array<Double> reffreqArray(shape, reffreq);  // one-dimensional array to store rest frequencies from image

//    LOG_INFO_STR("Writing frequency " << MSrefFreq << " to " << patchNames[i]);

    // Get image reference frequency and write it to the map
    //
    Table image(imageName, Table::Update);                // open image-table as rw
    RecordInterface &CoordsRec(image.rwKeywordSet().asrwRecord("coords"));
    RecordFieldId spectral2Id("spectral2");
    RecordInterface &spectral2(CoordsRec.asrwRecord(spectral2Id));

    RecordFieldId restFreqId("restfreq");
    imageRestfreq=spectral2.asDouble(restFreqId);     // read out original image reffreq
    RecordFieldId restFreqsId("restfreqs");
    Array<Double> imageRestfreqs=spectral2.asArrayDouble(restFreqsId);   // get all rest frequencies
    spectral2.define(restFreqId, reffreq);            // Update restfreq with new reffreq
    spectral2.define(restFreqsId, reffreqArray);      // Update restfreqs array with new reffreq

    // Update coords worldreplace2 field
    RecordFieldId worldreplace2Id("worldreplace2");
    CoordsRec.define(worldreplace2Id, reffreqArray);

    // Update wcs record: crval field
    RecordFieldId wcsId("wcs");
    RecordInterface &wcs(spectral2.asrwRecord(wcsId));
    RecordFieldId crvalId("crval");
    wcs.define(crvalId, reffreq);
    
    image.flush();
    return imageRestfreq;
}

// Display usage info
//
void usage(const char *programname)
{
  cout << "Usage: " << programname << "<imagename> <patchfrequency>" << endl;
}
