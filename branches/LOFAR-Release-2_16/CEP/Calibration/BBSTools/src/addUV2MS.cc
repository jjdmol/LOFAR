//# addUV2MS: adds uv data from a (casa) MS file to a named column of another
//# (LOFAR) MS
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
//# $Id: addUV2MS.h xxxx 2011-05-16 12:59:37Z duscha $

// \file takes a LOFAR MS, using its UV-coverage and bandwidth coverage
// to predict uv-data from (CASA) input image(s)
// Internally casarest's ft() task is used to predict the uv-data. The uv
// data is a MODEL_DATA_<patchname> column (where the file extension has been
// stripped off). If multiple images are supplied, multiple columns are being
// created.
//
// Any existing MODEL_DATA column is copied over to a temporary column,
// MODEL_DATA_bak, and will be restored back to MODEL_DATA after the predictions.
//
// If MODEL_DATA_<patchname> columns are already present, overwrite or abort is
// offered.

#include <lofar_config.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/Exception.h>     // THROW macro for exceptions

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
#include <synthesis/MeasurementEquations/Imager.h>          // casarest ft()
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <BBSTools/addUV2MS.h>

//using namespace casa;
using namespace std;
using namespace casa;
using namespace LOFAR;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

/*
function declarations now in include/BBSTools/addUV2MS.h
*/

//----------------------------------------------------------------
// Main
//
int main(int argc, char *argv[])
{
  vector<string> arguments;       // vector to keep arguments for arg parsing
  casa::String MSfilename;        // Filename of LafarMS
  Vector<String> patchNames;      // vector with filenames of patches used as models
  unsigned int nwplanes=0;        // got to see  how to export this feature to the outside

  // Init logger
  string progName = LOFAR::basename(argv[0]);
  INIT_LOGGER(progName);
    
  if(argc < 3)        // if not enough parameters given, display usage information
  {
      usage(progName);
      exit(0);
  }
  else      // Handle file arguments: MS image image (and options, e.g. -w 512)
  {
      // Move arguments into a vector (this can be easier modified), skip program name
      for(int i=1; i < argc; i++)     
      {
        arguments.push_back(argv[i]);
      }
      // We need to take out options that are neither MSfilename or imageFilename
      // i.e. -w 512   
      parseOptions(arguments, MSfilename, patchNames, nwplanes);
  }
  
  if(MSfilename=="")
  {
      casa::AbortError("No MS filename given");         // raise exception
      exit(0);
  }
  if(patchNames.size()==0)
  {
      casa::AbortError("No patch image filename given");
      exit(0);
  }
    
  //-------------------------------------------------------------------------------
  // Do the work    
    
  // remove existing MODEL_DATA_<patch> columns (only for patches supplied as 
  // parameter, other columns will be left untouched)
  removeExistingColumns(MSfilename, patchNames);    
  MeasurementSet LofarMS(MSfilename, Table::Update);          // Open LOFAR MS read/write

  // In case a previous run failed, MODEL_DATA-bak may still exist
  // and should be renamed to MODEL_DATA. An existing MODEL_DATA column
  // is for a patch and should be removed.
  if(LofarMS.tableDesc().isColumn("MODEL_DATA_bak"))
  {
      if(LofarMS.tableDesc().isColumn("MODEL_DATA"))
      {
	      LofarMS.removeColumn("MODEL_DATA");
      }
      LofarMS.renameColumn("MODEL_DATA", "MODEL_DATA_bak");
  }

  // Make sure the imager columns exist for the CASA ft function.
  addImagerColumns (LofarMS);
  LofarMS.flush();

  // Rename the existing MODEL_DATA.
  if(LofarMS.tableDesc().isColumn("MODEL_DATA"))
  {
    LofarMS.renameColumn ("MODEL_DATA_bak", "MODEL_DATA");
  }
 
  map<string, double> refFrequencies;
  refFrequencies=patchFrequency(LofarMS, patchNames);
 
  for(unsigned int i=0; i < patchNames.size(); i++)   // Loop over patchNames
  {
    string error;                   // to hold error mesage from validModelImage
    string columnName;              // columnName for uv data of this patch in table
    Vector<String> model(1);        // we need a ft per model to write to each column

    model[0]=patchNames[i];

    if(!validModelImage(patchNames[i], error))      // validate model image (.model, Jy/pixel)
    {
      LOG_WARN_STR("Skipping image " << model[0] << " because it is invalid.");
      LOG_WARN_STR(error);
      break;
    }
    else
    {
      columnName=createColumnName(patchNames[i]);       
      LOG_INFO_STR("Adding column: " << columnName);
    }

    // Add the MODEL_DATA column for this patch.
    addModelColumn(LofarMS, columnName);
    LofarMS.flush();

    //showColumnNames(LofarMS);
    // Do a predict with the casarest ft() function, complist="", because we only use the model images
    // Casarest Imager object which has ft method
    // (last parameter casa::True "use MODEL_DATA column")
    // i.e. Imager imager(LofarMS, True, True);
       
    // Get image options
    unsigned imSizeX=0, imSizeY=0, nchan=1, npol=1;
    Quantity cellSizeX;
    Quantity cellSizeY;
    String stokes;
    getImageOptions(patchNames[i], imSizeX, imSizeY, cellSizeX, cellSizeY, nchan, npol, stokes);
    
    MDirection patchdir=getPatchDirection(patchNames[i]);
    String mode="mfs";
    Quantity qStep(getMSChanwidth(LofarMS), "Hz");
    Vector<Int> spwIds(1);
    spwIds[0]=0;
    // Display image info
    LOG_INFO_STR("imSizeX = " << imSizeX);
    LOG_INFO_STR("imSizeY = " << imSizeY);
    LOG_INFO_STR("stokes = " << stokes);

    // Values for setoptions were taken from Imager::setdefaults entries
    Imager imager(LofarMS, True, True);
    MPosition obsPosition;    
    imager.defineImage( imSizeX,  imSizeY,
                        cellSizeX, cellSizeY,
                        stokes,  patchdir, 0,
                        mode, nchan,
                			  0, 1,
			                  MFrequency(MVFrequency(getMSReffreq(LofarMS))),
			                  MRadialVelocity(Quantity(0, "km/s")), 
                	      qStep,
                	      spwIds);
    
    if(nwplanes != 0)     //  if we want to do a wprojection, i.e. nwplanes not zero
    {    
      LOG_INFO_STR("Using nwplanes = " << nwplanes);
      imager.setoptions( "wproject", (HostInfo::memoryTotal()/8)*1024, 16, "SF", obsPosition, 1.2, 
                          nwplanes);
    }
    imager.ft(model, "", false);       // perform fft
    
    // rename MODEL_DATA column to MODEL_DATA_patchname column
    LofarMS.renameColumn (columnName, "MODEL_DATA"); 
    addDirectionKeyword(LofarMS, patchNames[i]);               
    LofarMS.flush();
  }
    
  restoreFrequency(refFrequencies);
   
  if(LofarMS.canRenameColumn("MODEL_DATA_bak"))   // Rename MODEL_DATA_bak column back to MODEL_DATA
  {
    LofarMS.renameColumn ("MODEL_DATA", "MODEL_DATA_bak"); 
  }
    
  //-------------------------------------------------------------------------------
  // Cleanup
  //
  LofarMS.flush();
  LofarMS.closeSubTables();                       // close Lofar MS
}

// Parse command line options, e.g. -w 512
//
void parseOptions(const vector<string> &args, 
                  string &msName, 
                  Vector<String> &patchNames,
                  unsigned int &nwplanes)
{
  unsigned int nopts=0;               // number of options found
  vector<string> arguments=args;      // local copy to work on

  // First extract all options
  for(vector<string>::iterator argsIt=arguments.begin(); argsIt!=arguments.end(); ++argsIt)
  {
    vector<string>::iterator nextIt;

    if(*argsIt=="-w" || *argsIt=="--wplanes")  // -wprojPlanes parameter for ft
    {
      arguments.erase(argsIt);    
      nwplanes= boost::lexical_cast<unsigned int>(*argsIt);
      arguments.erase(argsIt);    
      nopts++;                        // increase counter of options found
    
      if(argsIt==arguments.end())
        break;
    }
    // Try to handle unknown options (works only for short-hand options, i.e. -x)
    else if(argsIt->size()==2 && (argsIt->find("-")!=string::npos))
    {
      cout << "Unknown option: " << *argsIt << endl;
      cout << "Exiting..." << endl;
      exit(0);
    }
  }
  
  msName=arguments[0];               // get the MS name (first remaining argument by defintion)
  vector<string>::iterator argIt=arguments.begin();
  arguments.erase(argIt);

  // Add patch names to casa vector (need to do this because there is no Vector.append!?
  uInt length=arguments.size();
  patchNames.resize(length, True);                     // reshape casa vector to hold all patch names
  for(unsigned int i=0; i<patchNames.size(); i++)      // exclude MS and get remaining Patchnames
  {
    patchNames[i]=arguments[i];
  }  
}

// Get the reference frequency, reffreq, from the MS
//
Double getMSReffreq(const MeasurementSet &ms)
{
  // Read MSfrequency from MS/SPECTRAL_WINDOW table
  Table SpectralWindowTable(ms.keywordSet().asTable("SPECTRAL_WINDOW"));
  ROScalarColumn<Double> LofarRefFreqColumn(SpectralWindowTable, "REF_FREQUENCY");
  
  Double MSrefFreq=LofarRefFreqColumn(0);
  return MSrefFreq;
}

// Get the channel width, chanWidth, from the MS
//
Double getMSChanwidth(const MeasurementSet &ms)
{
  // Read MSfrequency from MS/SPECTRAL_WINDOW table
  Table SpectralWindowTable(ms.keywordSet().asTable("SPECTRAL_WINDOW"));
  ROArrayColumn<Double> ChanWidthColumn(SpectralWindowTable, "CHAN_WIDTH");
  
  Vector<Double> chanWidthArray=ChanWidthColumn(0);
  Double MSchanWidth=chanWidthArray[0];  
  return MSchanWidth;
}

// Read frequency from Image, patch if different from MS frequency and keep original
// in map
//
map<string, double> patchFrequency(MeasurementSet &ms, const Vector<String> &patchNames)
{
  Double imageRestfreq=0;
  map<string, double> refFrequencies;

  // Read MSfrequency from MS/SPECTRAL_WINDOW table
  Table SpectralWindowTable(ms.keywordSet().asTable("SPECTRAL_WINDOW"));
  ROScalarColumn<Double> LofarRefFreqColumn(SpectralWindowTable, "REF_FREQUENCY");
  
  Double MSrefFreq=LofarRefFreqColumn(0);

  uInt nPatches=patchNames.size();
  for(uInt i=0; i<nPatches; i++)
  {
    LOG_INFO_STR("Patching image " << patchNames[i] << " with MS frequency: " << MSrefFreq);
    imageRestfreq=updateFrequency(patchNames[i], MSrefFreq);
    refFrequencies[patchNames[i]]=imageRestfreq;            // store image ref frequency in map
  }

  return refFrequencies;
}

// Update the reffrequency information in an image with a new frequency
//
double updateFrequency(const string &imageName, double reffreq)
{
    Double imageRestfreq=0;
    IPosition shape(1);                 // define shape for a one-dimensional array
    shape(0)=1;                   
    Array<Double> reffreqArray(shape, reffreq);  // one-dimensional array to store rest frequencies from image

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

// Restore the original frequencies in the images after the MODEL_DATA uv has been generated
// by the casacrest ft-machine
//
void restoreFrequency(const map<string, double> &refFrequencies)
{
  map<string, double>::const_iterator mapIt;
  for(mapIt=refFrequencies.begin(); mapIt!=refFrequencies.end(); ++mapIt)
  {
    LOG_INFO_STR("Restoring image: " << mapIt->first <<"\tFrequency: " << mapIt->second);
    updateFrequency(mapIt->first, mapIt->second);
  }
}

// Read options from necessary to be passed on to casarest imager
//
void getImageOptions( const string &patchName, 
                      unsigned int &imSizeX, unsigned int &imSizeY, 
                      Quantity &cellSizeX, Quantity &cellSizeY, 
                      unsigned int &nchan, unsigned int &npol, string &stokes)
{
  IPosition shape;                      // shape of image: x,y,npol,nchan
  Vector<Double> delta;                 // get delta world per pixel
  Vector<String> units;                 // world axes units
  Vector<Int> dirAxesNumbers(2);        // indices of direction axes
  Vector<String> worldNames(4);         // names for world axes (should be RA,Dec)
  Vector<Int> stokesEnum;               // stokes enum indices of stokes parms present in image
  Vector<String> stokesParm;            // stokes parameters present in image
  
  PagedImage<Float> image(patchName);   // get a page image interface for this patch
  shape=image.shape();                  // get image shape
  
  //-----------------------------------------------------------------------
  CoordinateSystem coordsys=image.coordinates();
  // Direction coordinate (x and y)
  if(coordsys.hasDirectionCoordinate())     // get Stokes array from image
  {
    dirAxesNumbers=coordsys.directionAxesNumbers();
    worldNames=coordsys.worldAxisNames();

    delta=coordsys.increment();
    units=coordsys.worldAxisUnits();
    
    // Depending on orientation if RA:x Dec:y or vice versa
    if(worldNames[0]=="Right Ascension" && worldNames[1]=="Declination")
    {
      imSizeX=shape[dirAxesNumbers[0]];
      imSizeY=shape[dirAxesNumbers[1]];        
      cellSizeX=Quantity(delta[0], units[0]);
      cellSizeY=Quantity(delta[1], units[1]);
    }
    else if(worldNames[0]=="Declination" && worldNames[1]=="Right Ascension")
    {
      imSizeX=shape[dirAxesNumbers[1]];
      imSizeY=shape[dirAxesNumbers[0]];    
      cellSizeX=Quantity(delta[0], units[1]);
      cellSizeY=Quantity(delta[1], units[0]);
    }
    else
    {
      THROW(Exception, "Image " << patchName << "invalid world coordinates, not 'RA' and 'Dec'.");
    }
  }  
  else
  {
    THROW(Exception, "Image " << patchName << " has no Direction coordinate.");  
  }
  // Stokes
  if(coordsys.hasPolarizationAxis())     // get Stokes array from image
  {
    npol=shape[coordsys.polarizationAxisNumber()];  // No. of polarizations

    // Get a vector with enum integers into stokes::StokesTypes
    stokesEnum=coordsys.stokesCoordinate(coordsys.polarizationCoordinateNumber()).stokes();
    stokesParm.resize(stokesEnum.size());
    for(uInt i=0; i<stokesEnum.size(); i++)   // convert enum entries to string Stokes parameters
    {
      stokesParm[i]=Stokes::name(Stokes::type(stokesEnum[i]));
      stokes.append(stokesParm[i]);
    }
  }
  else
  {
    THROW(Exception, "Image " << patchName << " has no Stokes coordinate.");  
  }
  // Spectral
  if(coordsys.hasSpectralAxis())
  {
    nchan=shape[coordsys.spectralAxisNumber()];  
  }
  else
  {
    THROW(Exception, "Image " << patchName << " has no Direction coordinate.");  
  }
}

// Get the patch direction, i.e. RA/Dec of the central image pixel
//
casa::MDirection getPatchDirection(const string &patchName)
{
  casa::IPosition imageShape;                             // shape of image
  casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
  casa::MDirection MDirWorld(casa::MDirection::J2000);   // astronomical direction in J2000
  casa::PagedImage<casa::Float> image(patchName);         // open image
    
  imageShape=image.shape();                               // get centre pixel
  Pixel[0]=floor(imageShape[0]/2);
  Pixel[1]=floor(imageShape[1]/2);

  // Determine DirectionCoordinate
  casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
  dir.toWorld(MDirWorld, Pixel);

  return MDirWorld;
}

// Add keyword "LOFAR_DIRECTION" containing a 2-valued vector of type double with J2000 RA and DEC of patch 
// center in radians.
//
void addDirectionKeyword(casa::Table LofarTable, const string &patchName)
{  
  casa::MDirection direction(casa::MDirection::J2000);
  string columnName=createColumnName(patchName);
  
  // write it to the columnDesc
  casa::TableColumn LofarModelColumn(LofarTable, columnName);
  casa::TableRecord &Model_keywords = LofarModelColumn.rwKeywordSet();
  casa::TableRecord MDirectionRecord;
  casa::String error="";

  direction=getPatchDirection(patchName);
  
  // Use MeasureHolder class to convert MDirection to a keyword-compatible format
  casa::MeasureHolder mHolder(direction);
  if(!mHolder.toRecord(error, MDirectionRecord))
  {
    casa::AbortError("addDirectionKeyword()");
  }
  else
  {
    Model_keywords.defineRecord("LOFAR_DIRECTION", MDirectionRecord);
  }
}

// Create a column name based on the Modelfilename of the form
// modelfilename (minus any file extension)
//
string createColumnName(const casa::String &ModelFilename)
{
  string columnName;
  string patchName;
  casa::String Filename;
  casa::Path Path(ModelFilename);             // casa Path object to allow basename stripping
    
  Filename=Path.baseName();                   // remove path from ModelFilename

  unsigned long pos=string::npos;             // remove .image or .img extension from Patchname  
  /*
  if((pos=Filename.find(".img")) != string::npos)
  {
  }
  else if((pos=Filename.find(".image")) != string::npos)
  {
  }
  else if((pos=Filename.find(".model")) != string::npos)
  {
  }
  */
  if((pos=Filename.find(".model")) != string::npos) // only accept .model extension anymore
  {
  }
  
  if(pos!=string::npos)                       // if we have a file suffix
  {
    patchName=Filename.substr(0, pos);        // remove it
  }
  columnName+=patchName;                      // create complete column name according to scheme

  return columnName;
}

// Check that input model image has Jy/pixel flux
//
bool validModelImage(const casa::String &imageName, string &error)
{
  size_t pos=string::npos;
  bool valid=false;
  bool validName=false, validUnit=false;

  if((pos=imageName.find(".model")) != string::npos)   // Check filename extension
  {
    validName=true;
  }
  else if((pos=imageName.find(".img")) != string::npos)
  {
    error="Image filename ";
    error.append(imageName).append(" must have .model extension.");
    validName=false;    
  }
  else if((pos=imageName.find(".image")) != string::npos)
  {
    error="Image filename ";
    error.append(imageName).append(" must have .model extension.");
    validName=false;
  }
  else    // no extension is acceptable
  {
    validName=true;     // can we accept no extension?
  }

  // Look for Jy/beam entry in file table keywords "unit"
  Table image(imageName);                                       // open image-table as read-only
  const TableRecord &imageKeywords(image.keywordSet());
  RecordFieldId unitsId("units");
  string units=imageKeywords.asString(unitsId);
  if(units=="Jy/pixel")
  {
    validUnit=true;
  }
  else
  {
    error="Image ";
    error.append(imageName).append(" must have flux unit Jy/pixel.");
    validUnit=false;
  }

  if(validName && validUnit)    // Determine final validity
  {
    valid=true;
  }
  else
  {
    valid=false;
  }

  return valid;
}

// Check table for existing patch names, if any of the existing patch names are already
// present in <patchname> columns, offer overwrite option
//
void removeExistingColumns(const string &MSfilename, const Vector<String> &patchNames)
{
  string columnName;
  casa::Table LofarTable(MSfilename, casa::Table::Update);     

  // Remove existing Patchnames, but only those that are specified in the list of current run
  // it preserves other patch columns
  //
  for(unsigned int i=0; i < patchNames.size(); i++)      
  {
    columnName=createColumnName(patchNames[i]);        
    if(LofarTable.tableDesc().isColumn(columnName))
    {
      LofarTable.removeColumn(columnName);
    }
  }
  LofarTable.flush();
  LofarTable.closeSubTables();
}

// Display usage info
//
void usage(const string &programname)
{
  cout << "Usage: " << programname << "<options> LofarMS <patchname[s]>" << endl;
  cout << "LofarMS            - MS to add model data to" << endl;
  cout << "-w <nprojplanes>   - number of w projection planes to use (default w=0, i.e. normal ft)" << endl;
  cout << "<patchname[s]>     - list of patchname[s] of image[s], these filenames are used to name the column and should be" << endl;
  cout << "                     referred to in the parset file with .image,.img,.model extension removed" << endl;
}

// From: CEP/MS/src/MSCreate::addImagerColumns
//
void addImagerColumns (MeasurementSet& ms)
{
  // Find data shape from DATA column.
  // Make tiles of appr. 1 MB.
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::DATA)).shapeColumn();
  IPosition dataTileShape;
  if (shape.empty()) 
  {
    dataTileShape = IPosition(3, 4, 64, (1024*1024)/(4*64*8));
  }
  else
  {
    dataTileShape = IPosition(3, shape[0], shape[1], (1024*1024)/(shape.product()*8));
  }
  String colName = MS::columnName(MS::CORRECTED_DATA);
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty()) 
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "corrected data"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "corrected data", shape,
                                             ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan("TiledCorrectedData", dataTileShape);
    ms.addColumn (td, stMan);
  }
  ////  addModelColumn (ms, "TiledModelData");
  colName = MS::columnName(MS::IMAGING_WEIGHT);
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty())
    {
      td.addColumn (ArrayColumnDesc<Float>(colName, "imaging weight"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Float>(colName, "imaging weight",
                                           IPosition(1, shape[1]),
                                           ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan("TiledImagingWeight", dataTileShape.getLast(2));
    ms.addColumn (td, stMan);
  }
}

void addModelColumn (MeasurementSet& ms, const String& dataManName)
{
  // Find data shape from DATA column.
  // Make tiles of appr. 1 MB.
  String colName (MS::columnName(MS::MODEL_DATA));
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::DATA)).shapeColumn();
  IPosition dataTileShape;
  if (shape.empty()) 
  {
    dataTileShape = IPosition(3, 4, 64, (1024*1024)/(4*64*8));
  } 
  else
  {
    dataTileShape = IPosition(3, shape[0], shape[1], (1024*1024)/(shape.product()*8));
  }
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty())
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "model data"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "model data", shape, ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan(dataManName, dataTileShape);
    ms.addColumn (td, stMan);
    // Set MODEL_DATA keyword for casa::VisSet.
    // Sort out the channel selection.
    MSSpWindowColumns msSpW(ms.spectralWindow());
    Matrix<Int> selection(2, msSpW.nrow());
    // Fill in default selection (all bands and channels).
    selection.row(0) = 0;    //start
    selection.row(1) = msSpW.numChan().getColumn(); 
    ArrayColumn<Complex> mcd(ms, colName);
    mcd.rwKeywordSet().define ("CHANNEL_SELECTION",selection);
  }
}

//------------------------------------------------------------------
// DEBUG functions

// Get the column names
//
Vector<String> getColumnNames(const Table &table)
{
  TableDesc td=table.tableDesc();
  return td.columnNames();
}

// Display the column Names of the table
//
void showColumnNames(Table &table)
{
  Vector<String> columnNames; 
  columnNames=getColumnNames(table);
    
  for(Vector<String>::iterator it=columnNames.begin(); it!=columnNames.end(); ++it)
  {
    cout << *it << "\t";
  }
  cout << endl;
}
