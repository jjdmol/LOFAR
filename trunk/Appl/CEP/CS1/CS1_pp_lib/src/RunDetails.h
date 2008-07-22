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
#ifndef __CS1_PP_RUN_DETAILS_H__
#define __CS1_PP_RUN_DETAILS_H__

namespace LOFAR
{
  namespace CS1
  {
    class RunDetails
    {
    public:
      RunDetails();
      ~RunDetails();

      int    Fixed;        // BandpassCorrector
      int    FreqWindow;   // FrequencyFlagger, MADFlagger
      int    TimeWindow;   // ComplexMedianFlagger, MADFlagger
      double Treshold;     // FrequencyFlagger
      double MinThreshold; // ComplexMedianFlagger
      double MaxThreshold; // ComplexMedianFlagger
      int    Algorithm;    // FrequencyFlagger
      bool   Existing;     // all flaggers
      int    NChan;        // DataSquasher
      int    Start;        // DataSquasher
      int    Step;         // DataSquasher
      bool   Skip;         // DataSquasher
      bool   Columns;      // DataSquasher
      void PrintInfo(void);

    protected:
    private:
    }; // class RunDetails
  }; // namespace CS1
}; // namespace LOFAR

#endif // __CS1_PP_RUN_DETAILS_H__
