//#  ImagerProcessControl.cc: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#  @author Adriaan Renting, renting@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <iostream>
#include <cstdlib>
#include <ms/MeasurementSets.h>
#include <synthesis/MeasurementEquations/Imager.h>
#include <casa/version.h>

#include <CS1_Imager/ImagerProcessControl.h>

namespace LOFAR 
{
  namespace CS1
  {
    //===============>>> ImagerProcessControl::ImagerProcessControl  <<<===============
    ImagerProcessControl::ImagerProcessControl()
    : ProcessControl()
    {
      myMS     = NULL;
      myImager = NULL;
    }

    //===============>>> ImagerProcessControl::~ImagerProcessControl  <<<==============
    ImagerProcessControl::~ImagerProcessControl()
    {
      if (myMS)
      { delete myMS;
      }
      if (myImager)
      {
        myImager->close();
        delete myImager;
      }
    }

    //===============>>> ImagerProcessControl::define  <<<==============================
    tribool ImagerProcessControl::define(LOFAR::ACC::APS::ParameterSet ParamSet)
    { 
      cout << "AIPS++ version: ";
      casa::VersionInfo::report(cout);
      cout  << string(" Imager wrapper by Adriaan Renting for LOFAR CS1 data") << std::endl
            << string("This is experimental software, please report errors or requests to renting@astron.nl") << std::endl
            << string("Documentation can be found at: http://www.astron.nl/aips++/docs/aips++.html") << std::endl;
      itsMS            = ParamSet.getString("ms");
      itsCompress      = ParamSet.getBool("compress"); 
      itsMode          = ParamSet.getString("mode");
      itsNChannel      = ParamSet.getInt32("nchan");
      itsStart         = ParamSet.getInt32("start");
      itsStep          = ParamSet.getInt32("step");
      itsNX            = ParamSet.getInt32("nx");
      itsNY            = ParamSet.getInt32("ny");
      itsCellX         = ParamSet.getInt32("cellx");
      itsCellY         = ParamSet.getInt32("celly");
      itsStokes        = ParamSet.getString("stokes");
      itsWeightType    = ParamSet.getString("weighttype");
      itsWeightNPixels = ParamSet.getInt32("weightnpixels");
      itsTile          = ParamSet.getInt32("tile");
      itsPadding       = ParamSet.getDouble("padding");
      itsGridFunction  = ParamSet.getString("gridfunction");
      itsImageType     = ParamSet.getString("imagetype");
      itsImageName     = ParamSet.getString("imagename");
      return true;
    }

    //===============>>> ImagerProcessControl::run  <<<=================================
    tribool ImagerProcessControl::run()
    { 
      std::cout << "Runnning imager please wait..." << std::endl;
      myImager->makeimage(itsImageType, itsImageName);
      return true;
    }

    //===============>>> ImagerProcessControl::init  <<<================================
    tribool ImagerProcessControl::init()
    {
      myMS     = new casa::MeasurementSet(itsMS);
      myImager = new casa::Imager (*myMS, itsCompress);
      
      const casa::Vector<casa::Int> myNChannel(itsNChannel);
      const casa::Vector<casa::Int> myStart(itsStart);
      const casa::Vector<casa::Int> myStep(itsStep);
      const casa::String            myMode(itsMode);
      const casa::MRadialVelocity   itsMStart(casa::Quantity(0, "km/s"), casa::MRadialVelocity::DEFAULT); //? unknown what the default should be, AR
      const casa::MRadialVelocity   itsMStep(casa::Quantity(0, "km/s"), casa::MRadialVelocity::DEFAULT); //? unknown what the default should be, AR
      const casa::Vector<casa::Int> itsSpectralWindowIDs(0); //It's 1 in Glish, but propably 0 in C++
      int                           itsFieldID(0); //It's 1 in Glish, but propably 0 in C++
      const casa::Vector<casa::Int> myFieldID(itsFieldID);
      myImager->setdata(myMode, myNChannel, myStart, myStep, itsMStart, itsMStep, itsSpectralWindowIDs, myFieldID);
      
      const casa::Quantity          itsCellX(itsCellX, "arcsec");
      const casa::Quantity          itsCellY(itsCellY, "arcsec");
      const casa::String            myStokes(itsStokes);
      bool                          itsDoShift(false);
      const casa::MDirection        itsPhaseCenter(0.0); //? unknown what the default should be, AR
      const casa::Quantity          itsShiftX(0, "arcsec");
      const casa::Quantity          itsShiftY(0, "arcsec");
      int                           itsFacets(0); //It's 1 in Glish, but propably 0 in C++
      const casa::Quantity          itsDistance(0, "m");
      const casa::Float             itsPaStep(0.0); //? unknown what the default should be, AR
      const casa::Float             itsPbLimit(0.0); //? unknown what the default should be, AR
      myImager->setimage(itsNX, itsNY, itsCellX, itsCellY, myStokes, itsDoShift, itsPhaseCenter, itsShiftX, itsShiftY,
                         myMode, itsNChannel, itsStart, itsStep, itsMStart, itsMStep, itsSpectralWindowIDs,
                         itsFieldID, itsFacets, itsDistance, itsPaStep, itsPbLimit);

      const casa::String   itsWeightAlgorithm(itsWeightType);
      const casa::String   itsRmode("none");      
      const casa::Quantity itsNoise(0.0, "Jy");
      const casa::Double   itsRobust(0.0);
      const casa::Quantity itsFieldOfView(0, "rad");
      //const casa::Bool     itsMosaic(false); //?? It's there in Glish but not in C++
      if (itsWeightType == "uniform")
      { myImager->weight(itsWeightAlgorithm, itsRmode, itsNoise, itsRobust, itsFieldOfView, itsWeightNPixels);
      }
      else
      { myImager->weight(itsWeightAlgorithm, itsRmode, itsNoise, itsRobust, itsFieldOfView, 0);
      }
      //if (itsTile)
      //{
      //  myImager->setoptions(itsTile, itsPadding, itsGridFunction);
      //}
      return true;
    }

    //===============>>> ImagerProcessControl::pause  <<<===============================
    tribool ImagerProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> ImagerProcessControl::quit  <<<================================
    tribool ImagerProcessControl::quit()
    { 
      if (myMS)
      {
        delete myMS;
        myMS = NULL;
      }
      if (myImager)
      {
        myImager->close();
        delete myImager;
        myImager = NULL;
      }
      return true;
    }

    //===============>>> ImagerProcessControl::recover  <<<=============================
    tribool ImagerProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> ImagerProcessControl::reinit  <<<==============================
    tribool ImagerProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> ImagerProcessControl::askInfo  <<<=============================
    std::string ImagerProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> ImagerProcessControl::snapshot  <<<============================
    tribool ImagerProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
