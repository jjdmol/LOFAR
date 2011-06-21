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

// \file reads in MODEL_DATA column(s) from (Casa) MS and adds these
// to a LOFAR MS with different names using the MS filename
// a third argument is the corresponding image
//
// If only one argument is passed that is assumed to be the image and
// the ft task is used internally

#include <lofar_config.h>

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumnFunc.h>
#include <casa/Arrays/IPosition.h>
#include <Vector.h>

#include <math.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <images/Images/PagedImage.h>                       // we need to open the image to determine patch centre direction

// casarest ft
#include <synthesis/MeasurementEquations/Imager.h>
//#include <casarest/synthesis/MeasurementEquations/Deconvolaver.h>

//casa::DirectionCoordinate getPatchDirection(const string &patchName);
casa::Vector<casa::Double> getPatchDirection(const string &patchName);
void addDirectionKeyword(casa::Table LofarMS, const string &patchName);
string createColumnName(const string &);
void usage(const char *);

//using namespace casa;
using namespace std;
using namespace casa;

int main(int argc, char *argv[])
{
    // Trying to use STL vectors
    casa::String MSfilename;
    //vector<string> patchNames;
    // Using casa::Vector
    Vector<String> MSfilenames;
    Vector<String> patchNames;
    string columnName;
    
    // Get command line parameters:
    //int opt=0;
    //int opterror=0;
    //int index=0;
    
    if(argc < 3)        // if not enough parameters given, display usage information
    {
       usage(argv[0]);
       exit(0);
    }
    /*
    else if(argc == 2)
    {
        MSfilenames.push_back(argv[1]);
        patchNames.push_back(argv[2]);
    } 
    */

    // DEBUG: check input parameters
    MSfilename=argv[1];
    cout << "MSfilename = " << MSfilename << endl;
    //cout << "patchName = " << patchName << endl;
    
    for(int i=2; i < argc; i++)
    {
        Int length;                          // Shape, i.e. length of Vector
        patchNames.shape(length);            // determine length of Vector

        patchNames.resize(length+1, True);  // resize and copy values
        patchNames[length]=argv[i];
//        patchNames.push_back(argv[i]);    // STL way to append patch    
    }
    
    
    // Check consistency of input parameters
    if(MSfilenames.size()==0)
    {
        casa::AbortError("No MS filename given");         // raise exception
        exit(0);
    }
    if(patchNames.size()==0)
    {
        casa::AbortError("No patch image filename given");
        exit(0);
    }
        
    MeasurementSet LofarMS(MSfilenames[0]);           // Open LOFAR MS
    
    // Casarest imager object which has ft method
    //Imager imager(LofarMS, casa::True, casa::True);     // create an Imager object needed for predict with ft
    Vector<String> models=patchNames;
    Bool incremental=False;                                   // create incremental UV data from models?
    
    for(int i=0; i < argc-1; i++)
    {
        columnName=createColumnName(patchNames[i]);
    
        // Do a predict with the casarest ft() function
        //imager.ft(models, MSfilename, incremental);
        
        // rename MODEL_DATA column to MODEL_DATA_patchname column
        casa::Table LofarTable(MSfilenames[i], casa::Table::Update);  
        LofarTable.renameColumn (columnName, "MODEL_DATA");    
        //addDirectionKeyword(LofarMS, columnName);           
    
        // recreate MODEL_DATA column (must be present)
        ColumnDesc ModelColumn(ArrayColumnDesc<Complex>("MODEL_DATA"));
        LofarTable.addColumn(ModelColumn);
    }
    
    // Cleanup    
    //LofarMS.flush();
    //LofarMS.closeSubTables();                            // close Lofar MS

}


// Get the patch direction, i.e. RA/Dec of the central image pixel
//
//casa::DirectionCoordinate::DirectionCoordinate getPatchDirection(const string &patchName)
casa::Vector<casa::Double> getPatchDirection(const string &patchName)
{
    casa::IPosition imageShape;                             // shape of image
    casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
    casa::Vector<casa::Double> World(2);                    // world coords vector of image centre
    casa::PagedImage<casa::Float> image(patchName);         // open image
    
    imageShape=image.shape();                               // get centre pixel
    Pixel[0]=floor(imageShape[0]/2);
    Pixel[1]=floor(imageShape[1]/2);

    // Determine DirectionCoordinate
    casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
    dir.toWorld(World, Pixel);

    return World;
}


// Add keyword "DIRECTION" containing a 2-valued vector of type double with J2000 RA and DEC of patch center in radians.
// read patch center from model MS/FIELD column PHASE_DIR
//casa::Table ModelMSField(MSfilename + "/FIELD");
//
void addDirectionKeyword(casa::Table LofarTable, const string &patchName)
{  
    casa::Vector<casa::Double> direction;
    string columnName=createColumnName(patchName);

    cout << "patchName = " << patchName << endl;    // DEBUG       
    cout << "columnName = " << columnName << endl;  // DEBUG
    cout.flush();                                   // DEBUG

    
    // write it to the columnDesc
    //casa::TableRecord &LofarModel_Data_Record=LofarMS.rwKeywordSet(); 
    //casa::ArrayColumn<casa::Complex> LofarModelColumn(LofarMS, columnName);
    casa::TableColumn LofarModelColumn(LofarTable, columnName);
    casa::TableRecord &Model_keywords = LofarModelColumn.rwKeywordSet();
    
    direction=getPatchDirection(patchName);
   
    Model_keywords.define("Ra", direction[0]);
    Model_keywords.define("Dec", direction[1]);
}


// Create a column name based on the Modelfilename of the form
// MODEL_DATA_modelfilename (minus any file extension)
//
string createColumnName(const string &ModelFilename)
{
    string columnName="MODEL_DATA_";
    string patchName;
    unsigned long pos=ModelFilename.find(".");       // remove .image or .img extension from Patchname

    if(pos!=string::npos)                       // if we have a file suffix
    {
        patchName=ModelFilename.substr(0, pos); // remove it
    }
    columnName+=patchName.substr(0,pos);        // create complete column name according to scheme

    return columnName;
}


// Display usage info
//
void usage(const char *programname)
{
    cout << "Usage: " << programname << ": LofarMS <patchname[s]>" << endl;
    cout << "LofarMS        - MS to add model data to" << endl;
    cout << "<patchname[s]> - list of patchname[s] of image[s], these filenames are used to name the column and should be" << endl;
    cout << "                 referred to in the skymodel file with .MS extension removed" << endl;
}
