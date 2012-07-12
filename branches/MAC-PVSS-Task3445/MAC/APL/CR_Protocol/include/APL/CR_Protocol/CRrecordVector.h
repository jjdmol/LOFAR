//#  CRrecordVector.h: Serialisable structure holding read requests for TBBs
//#
//#  Copyright (C) 2011
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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
//#  $Id: Pointing.h 14866 2010-01-23 00:07:02Z overeem $

#ifndef CR_RECORD_VECTOR_H_
#define CR_RECORD_VECTOR_H_

#include <Common/lofar_vector.h>
#include <APL/CR_Protocol/CRrecordRequest.h>

namespace LOFAR {
  namespace CR_Protocol {

class CRrecordVector
{
public:
	//@{
	// Constructors and destructors for a pointing.
	CRrecordVector() {};
	~CRrecordVector() {};
	//@}

	// --- Output function --- 
	ostream& print (ostream& os) const;

	//@{
	// --- marshalling methods --- 
	unsigned int getSize();
	unsigned int pack  (void* buffer);
	unsigned int unpack(void *buffer);
	//@}

	// --- datamembers ---
	vector<CRrecordRequest>	requests;
};

//
// operator<<
//
inline ostream& operator<< (ostream& os, const CRrecordVector& rv) 
{
	return (rv.print(os));
}

  }; // namespace CR_Protocol
}; // namespace LOFAR

#endif /* CR_RECORD_VECTOR_H_ */
