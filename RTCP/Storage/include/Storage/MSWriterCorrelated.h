//  MSMriterCorrelated.h: a writer for correlated visibilities
//
//  Copyright (C) 2001
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
//  $Id: MSWriterImpl.h 11891 2008-10-14 13:43:51Z gels $
//
//////////////////////////////////////////////////////////////////////


#ifndef LOFAR_STORAGE_MSWRITERCORRELATED_H
#define LOFAR_STORAGE_MSWRITERCORRELATED_H

#include <Storage/MSWriterFile.h>
#include <Interface/Parset.h>

namespace LOFAR {
namespace RTCP {


class MSWriterCorrelated : public MSWriterFile
{
  public:
    MSWriterCorrelated(const string &msName, const Parset &parset);
    ~MSWriterCorrelated();

    virtual void write(StreamableData *data);

  protected:
    const Parset &itsParset;
};


}
}

#endif
