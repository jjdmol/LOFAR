//  DH_Solution.h: DataHolder for BlackBoard solutions
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

#ifndef PSS3_DH_SOLUTION_H
#define PSS3_DH_SOLUTION_H

#include <Common/lofar_vector.h>
#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <BBS3/Quality.h>

namespace LOFAR
{

/**
   This class is a DataHolder which holds the parameters solved by
   a PSS3 knowledge source.
*/

class DH_Solution: public LOFAR::DH_PL
{
public:
  typedef PL::TPersistentObject<DH_Solution> PO_DH_SOL;

  explicit DH_Solution (const string& name="dh_solution");

  virtual ~DH_Solution();

  DataHolder* clone() const;

  // Get a reference to the PersistentObject.
  virtual PL::PersistentObject& getPO() const;

  // Create a TPO object and set the table name in it.
  virtual void initPO (const string& tableName);

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// overload the getcursize method;
  /// reported data size may be altered with setCurDataSize() method
  int  getCurDataSize() ;
  void setCurDataSize(const int nbytes) ;

  // Data access methods
  int getID() const;
  void setID(const int id);

  int getWorkOrderID() const;
  void setWorkOrderID(const int id);

  int getIterationNo() const;
  void setIterationNo(const int no);

  void setSolutionID(const int id);  // Set id of solution to retrieve from database
  int getSolutionID() const;

  Quality getQuality() const;
  void setQuality(const Quality& quality);

  unsigned int getNumberOfParam() const;
  void setNumberOfParam(const unsigned int no);

  bool getSolution(vector<string>& names, vector<double>& values);
  void setSolution(vector<string>& names, vector<double>& values);

/*   void getParamValues(vector<double>& values) const; */
/*   void setParamValues(const vector<double>& values); */

/*   void getParamNames(vector<string>& names) const; */
/*   void setParamNames(const vector<string>& names); */

  // Resets (clears) the contents of its DataPacket 
  void clearData();
  
  void dump();

private:
  /// Forbid assignment.
  DH_Solution& operator= (const DH_Solution&);
  DH_Solution(const DH_Solution&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsID;
  int*          itsWOID;
  int*          itsIteration;
  double*       itsFit;
  double*       itsMu;
  double*       itsStdDev;
  double*       itsChi;
  unsigned int* itsNumberOfParam;
   
  PO_DH_SOL*    itsPODHSOL; 

  int itsCurDataSize;

};

inline int DH_Solution::getID() const
{ return *itsID; }

inline void DH_Solution::setID(const int id)
{ *itsID = id; }

inline int DH_Solution::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_Solution::setWorkOrderID(const int id)
{ *itsWOID = id; }

inline int DH_Solution::getIterationNo() const
{ return *itsIteration; }

inline void DH_Solution::setIterationNo(const int no)
{ *itsIteration = no; }

inline unsigned int DH_Solution::getNumberOfParam() const
{ return *itsNumberOfParam; }

inline void DH_Solution::setNumberOfParam(const unsigned int no)
{ *itsNumberOfParam = no; }

inline int DH_Solution::getCurDataSize() 
{ return itsCurDataSize; }
   
inline void DH_Solution::setCurDataSize(const int nbytes)
{  itsCurDataSize = nbytes;  }

// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                               
  class DBRep<DH_Solution> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_Solution&);                        
    private: 
      int itsID;                   // Temporarily stored in separate fields
      int itsWOID;                 // in order to facilitate debugging
      int itsIteration;
      double itsFit;
      double itsMu;
      double itsStdDev;
      double itsChi;
      unsigned int itsNumberOfParam;
    };   
                                                      
} // end namespace PL   


} // namespace LOFAR

#endif 
