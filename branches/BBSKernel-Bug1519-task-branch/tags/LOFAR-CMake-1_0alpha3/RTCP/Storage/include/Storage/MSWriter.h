//  MSMriter.h: Base class MSWriter
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////

#ifndef LOFAR_STORAGE_MSWRITER_H
#define LOFAR_STORAGE_MSWRITER_H

#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

#include <Interface/StreamableData.h>

namespace LOFAR
{
  namespace RTCP
  {
    class MSWriter
    {
    public:
      MSWriter();
      virtual ~MSWriter();

      virtual int addBand(int, int, double, double);
      virtual int addBand(int, int, double, const double*, const double*);
      virtual void addField(double, double, unsigned);
      virtual void write(int, int, int, StreamableData*);

    private:

      int itsNrBand;
      int itsNrField;
      int itsNrAnt;
      int itsNrFreq;
      int itsNrCorr;
      int itsNrTimes;
      int itsNrPol;
      int itsNrChan;

    };
  } // namespace RTCP
} // namespace LOFAR

#endif
