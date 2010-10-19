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
    ImagerProcessControl::ImagerProcessControl(void)
    : ProcessControl()
    {
      myMS     = NULL;
      myImager = NULL;
    }

    //===============>>> ImagerProcessControl::~ImagerProcessControl  <<<==============
    ImagerProcessControl::~ImagerProcessControl(void)
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
    tribool ImagerProcessControl::define(void)
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      cout << "AIPS++ version: ";
      casa::VersionInfo::report(cout);
      cout  << string(" Imager wrapper by Adriaan Renting for LOFAR CS1 data") << std::endl
            << string("This is experimental software, please report errors or requests to renting@astron.nl") << std::endl
            << string("Documentation can be found at: http://www.astron.nl/aips++/docs/aips++.html") << std::endl;
      itsMS            = ParamSet->getString("ms");
      itsCompress      = ParamSet->getBool("compress");
      itsDataMode      = ParamSet->getString("datamode");
      itsImageMode     = ParamSet->getString("imagemode");
      itsNChannel      = ParamSet->getInt32("nchan");
      itsSpectralWindows = ParamSet->getInt32Vector("spwid");
      itsStart         = ParamSet->getInt32("start");
      itsStep          = ParamSet->getInt32("step");
      itsNX            = ParamSet->getInt32("nx");
      itsNY            = ParamSet->getInt32("ny");
      itsCellX         = ParamSet->getDouble("cellx");
      itsCellY         = ParamSet->getDouble("celly");
      itsStokes        = ParamSet->getString("stokes");
      itsWeightType    = ParamSet->getString("weighttype");
      itsWeightNPixels = ParamSet->getInt32("weightnpixels");
      itsTile          = ParamSet->getInt32("tile");
      itsPadding       = ParamSet->getDouble("padding");
      itsGridFunction  = ParamSet->getString("gridfunction");
      itsImageType     = ParamSet->getString("imagetype");
      itsImageName     = ParamSet->getString("imagename");
      return true;
    }

    //===============>>> ImagerProcessControl::run  <<<=================================
    tribool ImagerProcessControl::run(void)
    {
      std::cout << "Runnning imager please wait..." << std::endl;
      if ( myImager->makeimage(itsImageType, itsImageName))
      {
        std::cout << "Runnning imager finished." << std::endl;
        return true;
      }
      else
      { return false;
      }
    }

    //===============>>> ImagerProcessControl::init  <<<================================
    tribool ImagerProcessControl::init(void)
    {
      myMS     = new casa::MeasurementSet(itsMS, casa::Table::Update);
      myImager = new casa::Imager (*myMS, itsCompress);

      const casa::Vector<casa::Int> myNChannel(1, itsNChannel);
      const casa::Vector<casa::Int> myStart(1, itsStart);
      const casa::Vector<casa::Int> myStep(1, itsStep);
      const casa::String            myDataMode(itsDataMode);
      const casa::String            myImageMode(itsImageMode);
      const casa::MRadialVelocity   itsMStart(casa::Quantity(0, "km/s"), casa::MRadialVelocity::DEFAULT); //? unknown what the default should be, AR
      const casa::MRadialVelocity   itsMStep(casa::Quantity(0, "km/s"), casa::MRadialVelocity::DEFAULT); //? unknown what the default should be, AR
      const casa::Vector<casa::Int> mySpectralWindowIDs(itsSpectralWindows); //It's 1 in Glish, but propably 0 in C++
      int                           itsFieldID(0); //It's 1 in Glish, but propably 0 in C++
      const casa::Vector<casa::Int> myFieldID(1, itsFieldID);
      myImager->setdata(myDataMode, myNChannel, myStart, myStep, itsMStart, itsMStep, mySpectralWindowIDs, myFieldID);

      const casa::Quantity          myCellX(itsCellX, "arcsec");
      const casa::Quantity          myCellY(itsCellY, "arcsec");
      const casa::String            myStokes(itsStokes);
      bool                          itsDoShift(false);
      const casa::MDirection        itsPhaseCenter(0.0); //? unknown what the default should be, AR
      const casa::Quantity          itsShiftX(0, "arcsec");
      const casa::Quantity          itsShiftY(0, "arcsec");
      int                           itsFacets(1); //It's 1 in Glish, but propably 0 in C++, or maybe not ?
      const casa::Quantity          itsDistance(0, "m");
      const casa::Float             itsPaStep(5.0); //? unknown what the default should be, AR
      const casa::Float             itsPbLimit(0.05); //? unknown what the default should be, AR
      if (myImageMode == "mfs")
      { myImager->setimage(itsNX, itsNY, myCellX, myCellY, myStokes, itsDoShift, itsPhaseCenter, itsShiftX, itsShiftY,
                           myImageMode, 1, itsStart, itsStep, itsMStart, itsMStep, mySpectralWindowIDs,
                           itsFieldID, itsFacets, itsDistance, itsPaStep, itsPbLimit);
      }
      else
      { myImager->setimage(itsNX, itsNY, myCellX, myCellY, myStokes, itsDoShift, itsPhaseCenter, itsShiftX, itsShiftY,
                           myImageMode, itsNChannel, itsStart, itsStep, itsMStart, itsMStep, mySpectralWindowIDs,
                           itsFieldID, itsFacets, itsDistance, itsPaStep, itsPbLimit);
      }
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
    tribool ImagerProcessControl::quit(void)
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

    //===============>>> ImagerProcessControl::release  <<<=============================
    tribool ImagerProcessControl::release()
    { return false;
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
