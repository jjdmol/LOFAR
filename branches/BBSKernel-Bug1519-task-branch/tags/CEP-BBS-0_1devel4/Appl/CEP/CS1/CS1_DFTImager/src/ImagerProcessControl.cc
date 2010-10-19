/***************************************************************************
 *   Copyright (C) 2007 by ASTRON, Adriaan Renting                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <casa/Inputs/Input.h>
#include <ms/MeasurementSets.h>

#include <CS1_DFTImager/ImagerProcessControl.h>
#include "MS_File.h"
#include "Image_File.h"
#include "DFTImager.h"

#define IMAGER_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    //===============>>> ImagerProcessControl::ImagerProcessControl  <<<===============
    ImagerProcessControl::ImagerProcessControl()
    : ProcessControl()
    {
      itsMS     = NULL;
      itsImage  = NULL;
      itsImager = NULL;
    }

    //===============>>> ImagerProcessControl::~ImagerProcessControl  <<<==============
    ImagerProcessControl::~ImagerProcessControl()
    {
      if (itsMS)
      { delete itsMS;
      }
      if (itsImage)
      { delete itsImage;
      }
      if (itsImager)
      { delete itsImager;
      }
    }

    //===============>>> ImagerProcessControl::define  <<<==============================
    tribool ImagerProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsMSName     = ParamSet->getString("ms");
      itsImageName  = ParamSet->getString("image");
      itsResolution = ParamSet->getInt32("resolution");
      return true;
    }

    //===============>>> ImagerProcessControl::run  <<<=================================
    tribool ImagerProcessControl::run()
    {
      try{
      std::cout << "Runnning Imager please wait..." << std::endl;
      itsImager->MakeImage(itsResolution);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> ImagerProcessControl::init  <<<================================
    tribool ImagerProcessControl::init()
    {
      try {
      using std::cout;
      using std::cerr;
      using std::endl;

      cout  << string(IMAGER_VERSION) + string(" DFT Imager by Ronald Nijboer and Adriaan Renting\n") +
              string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
              string("Documentation can be found at: www.astron.nl/~renting\n");
      cout << itsMS << endl;
      itsMS     = new MS_File(itsMSName);
      itsImage  = new Image_File(itsImageName);
      itsImager = new DFTImager (itsMS, itsImage);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> ImagerProcessControl::pause  <<<===============================
    tribool ImagerProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> ImagerProcessControl::quit  <<<================================
    tribool ImagerProcessControl::quit()
    {
      if (itsMS)
      {
        delete itsMS;
        itsMS = NULL;
      }
      if (itsImager)
      {
        delete itsImager;
        itsImager = NULL;
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
