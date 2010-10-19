//#  BGL_Configuration.cc:
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

#include <lofar_config.h>

#include <CS1_Interface/BGL_Configuration.h>

#include <cassert>


namespace LOFAR {
namespace CS1 {


void BGL_Configuration::read(Stream *str)
{
  str->read(&itsMarshalledData, sizeof itsMarshalledData);

  itsInputPsets.resize(itsMarshalledData.itsInputPsetsSize);
  memcpy(&itsInputPsets[0], itsMarshalledData.itsInputPsets, itsMarshalledData.itsInputPsetsSize * sizeof(unsigned));

  itsOutputPsets.resize(itsMarshalledData.itsOutputPsetsSize);
  memcpy(&itsOutputPsets[0], itsMarshalledData.itsOutputPsets, itsMarshalledData.itsOutputPsetsSize * sizeof(unsigned));

  itsRefFreqs.resize(itsMarshalledData.itsRefFreqsSize);
  memcpy(&itsRefFreqs[0], itsMarshalledData.itsRefFreqs, itsMarshalledData.itsRefFreqsSize * sizeof(double));
}


void BGL_Configuration::write(Stream *str)
{
  itsMarshalledData.itsInputPsetsSize = itsInputPsets.size();
  assert(itsMarshalledData.itsInputPsetsSize <= MAX_PSETS);
  memcpy(itsMarshalledData.itsInputPsets, &itsInputPsets[0], itsMarshalledData.itsInputPsetsSize * sizeof(unsigned));

  itsMarshalledData.itsOutputPsetsSize = itsOutputPsets.size();
  assert(itsMarshalledData.itsOutputPsetsSize <= MAX_PSETS);
  memcpy(itsMarshalledData.itsOutputPsets, &itsOutputPsets[0], itsMarshalledData.itsOutputPsetsSize * sizeof(unsigned));

  itsMarshalledData.itsRefFreqsSize = itsRefFreqs.size();
  assert(itsMarshalledData.itsRefFreqsSize <= MAX_SUBBANDS);
  memcpy(itsMarshalledData.itsRefFreqs, &itsRefFreqs[0], itsMarshalledData.itsRefFreqsSize * sizeof(double));
  
  str->write(&itsMarshalledData, sizeof itsMarshalledData);
}


} // namespace CS1
} // namespace LOFAR
