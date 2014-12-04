//# Operation.h: Base class for awimager operations
//#
//# Copyright (C) 2014
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
//# $Id: $

#ifndef LOFAR_LOFARFT_OPERATION_H
#define LOFAR_LOFARFT_OPERATION_H

#include <LofarFT/Imager.h>

#include <Common/ObjectFactory.h>
#include <Common/Singleton.h>
#include <Common/lofar_string.h>

#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>


// Define color codes for help text

// #define COLOR_DEFAULT "\033[38;2;40;120;130m"
#define COLOR_DEFAULT "\033[36m"

// #define COLOR_OPERATION "\033[38;2;0;60;130m"
#define COLOR_OPERATION "\033[34m"

#define COLOR_PARAMETER "\033[35m"

#define COLOR_RESET "\033[0m"


namespace LOFAR { 
  
  namespace LofarFT {

    class Operation
    {
    public:
      Operation(ParameterSet& parset);
      
      virtual void init();

      void initData();

      void initImage();

      void initWeight();

      void initFTMachine();

      void makeEmptyImage(const casa::String& imgName, casa::Int fieldid);

      virtual void run() {}; // Only derived classes will do something useful in run()

      virtual void showHelp (ostream& os, const string& name);
      
      static casa::IPosition readIPosition (const casa::String& in);
      
      static casa::Quantity readQuantity (const casa::String& in);
      
      static casa::MDirection readDirection (const casa::String& in);
      
      static void readFilter (
        const casa::String& filter,
        casa::Quantity& bmajor, 
        casa::Quantity& bminor, 
        casa::Quantity& bpa);

      static void normalize(
        casa::String imagename_in, 
        casa::String avgpb_name, 
        casa::String imagename_out);
      
    protected:
      ParameterSet               &itsParset;
      casa::String               itsMSName;
      casa::MeasurementSet       itsMS;
      casa::CountedPtr<Imager>   itsImager;
      bool                       needsData;
      bool                       needsImage;
      bool                       needsFTMachine;
      bool                       needsWeight;

    private:
      void showHelpData (ostream& os, const string& name);
      void showHelpImage (ostream& os, const string& name);
      void showHelpFTMachine (ostream& os, const string& name);
      void showHelpWeight (ostream& os, const string& name);
      casa::Double observationReferenceFreq(
        const casa::MeasurementSet &ms,
        casa::uInt idDataDescription);
    };

    // Factory that can be used to generate new Operation objects.
    // The factory is defined as a singleton.
    typedef Singleton< ObjectFactory< Operation*(ParameterSet&), string > > OperationFactory;

  } //# namespace LofarFT
} //# namespace LOFAR

#endif
