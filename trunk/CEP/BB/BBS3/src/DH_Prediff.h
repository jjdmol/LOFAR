//  DH_Prediff.h:  DataHolder
//
//  Copyright (C) 2000, 2001
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
//
//////////////////////////////////////////////////////////////////////

#ifndef BBS3_DH_PREDIFF_H
#define BBS3_DH_PREDIFF_H

#include <Transport/DataHolder.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

/**
   This class is a DataHolder which contains the difference between measured
   and predicted data and derivatives for each solvable parm (spid) for a
   certain domain [baseline, time, frequency, polarization].
*/
 //# Forward Declarations
class ParmData;

class DH_Prediff: public DataHolder
{
public:

  enum woStatus{New,Assigned,Executed};

  explicit DH_Prediff (const string& name = "dh_prediff");

  DH_Prediff(const DH_Prediff&);

  virtual ~DH_Prediff();

  DataHolder* clone() const;

/*   // Create a TPO object and set the table name in it. */
/*   virtual void initPO (const string& tableName); */

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  void setBufferSize(const vector<uint32>& shape);
  const vector<uint32>& getBufferSize();

  void setDataSize(unsigned int size);
  unsigned int getDataSize() const;

  double* getDataBuffer();

  bool getParmData(vector<ParmData>& pdata); 
  void setParmData(const vector<ParmData>& pdata);

  bool moreDataToCome();
  void setMoreData(bool more);

  void dump();

  void clearData();

private:
  /// Forbid assignment.
  DH_Prediff& operator= (const DH_Prediff&);

  // Fill the pointers to the data in the blob.
  virtual void fillDataPointers();

  unsigned int* itsDataSize;    // Number of equations in data buffer
  double* itsDataBuffer;
  unsigned int* itsMoreData;   // More result to come?
};

inline void DH_Prediff::setDataSize(unsigned int size)
{ *itsDataSize = size; }

inline unsigned int DH_Prediff::getDataSize() const
{ return *itsDataSize; }

inline double* DH_Prediff::getDataBuffer()
{ return itsDataBuffer; }

inline bool DH_Prediff::moreDataToCome()
{ return ((*itsMoreData==0)?(false):(true));}

inline void DH_Prediff::setMoreData(bool more)
{ *itsMoreData = more; }

} // namespace LOFAR

#endif 

