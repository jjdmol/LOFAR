/***************************************************************************
 *   Copyright (C) 2007-8 by ASTRON, Adriaan Renting                       *
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
#ifndef __CS1_PP_RUN_DETAILS_H__
#define __CS1_PP_RUN_DETAILS_H__

// @file
// @brief Class to hold parameter settings for the steps in IDPPP
// @author Adriaan Renting (renting AT astron nl)

namespace LOFAR
{
  namespace CS1
  {
    // @ingroup CS1_pp_lib
    //
    // This class is basically a fancy struct of the variables that need to be set
    // in the parameterset for IDPPP for the various steps.
    // It has some extra functionality to validate and print the values.
    // It isn't using setter/getter functions because that seemed overkill for the required use.

    class RunDetails
    {
    public:
      RunDetails();
      ~RunDetails();

      unsigned int Fixed;        // BandpassCorrector
      unsigned int FreqWindow;   // FrequencyFlagger, MADFlagger
      unsigned int TimeWindow;   // ComplexMedianFlagger, MADFlagger
      double       Treshold;     // FrequencyFlagger
      double       MinThreshold; // ComplexMedianFlagger
      double       MaxThreshold; // ComplexMedianFlagger
      unsigned int Algorithm;    // FrequencyFlagger
      bool         Existing;     // all flaggers
      unsigned int NChan;        // DataSquasher
      unsigned int Start;        // DataSquasher
      unsigned int Step;         // DataSquasher
      bool         Skip;         // DataSquasher
      bool         Columns;      // DataSquasher
      unsigned int TimeStep;     // DataSquasher
      bool CheckValues(void);    // Method to do some validity checks on the values
      void PrintInfo(void);      // Prints all values to cout, mainly for debugging purposes

    protected:
    private:
    }; // class RunDetails
  }; // namespace CS1
}; // namespace LOFAR

#endif // __CS1_PP_RUN_DETAILS_H__
