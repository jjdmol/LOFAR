//  DH_WorkOrder.h: Example DataHolder
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

#ifndef PSS3_DH_WORKORDER_H
#define PSS3_DH_WORKORDER_H

#include <Common/lofar_vector.h>

#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <PSS3/Quality.h>

namespace LOFAR
{

/**
   This class is a DataHolder which contains the work orders.
*/
 //# Forward Declarations
class KeyValueMap;

class DH_WorkOrder: public DH_PL
{
public:

  enum woStatus{New,Assigned,Executed};

  typedef PL::TPersistentObject<DH_WorkOrder> PO_DH_WO;

  explicit DH_WorkOrder (const string& name = "dh_workorder");

  DH_WorkOrder(const DH_WorkOrder&);

  virtual ~DH_WorkOrder();

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

  /// Data access methods.
  int getWorkOrderID() const;
  void setWorkOrderID(int id);

  unsigned int getStatus() const ;
  void setStatus(unsigned int status);

  string getKSType() const;
  void setKSType(const string& ksType);
  int getKSTypeLength();
  void setKSTypeLength(int length);

  unsigned int getStrategyNo() const;
  void setStrategyNo(unsigned int no);

  int getNoStartSolutions() const;
  void setNoStartSolutions(int no);

  int getParamNameLength();
  void setParamNameLength(int length);
  unsigned int getNumberOfParam() const;
  void setNumberOfParam(int number);

  void setVarData(const KeyValueMap& stratArgs, 
		  vector<string>& pNames, 
		  vector<int>& startSols);
  bool getVarData(KeyValueMap& stratArgs,
		  vector<string>& pNames,
		  vector<int>& startSols);

  void dump();

private:
  /// Forbid assignment.
  DH_WorkOrder& operator= (const DH_WorkOrder&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsWOID;
  unsigned int* itsStatus;
  char*         itsKSType;
  unsigned int* itsStrategyNo;
  int*          itsNoStartSols;
  unsigned int* itsNumberOfParam;

  PO_DH_WO*    itsPODHWO; 

  int itsCurDataSize;

  static int theirWriteCount;
  static int theirReadCount;

};

inline int DH_WorkOrder::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_WorkOrder::setWorkOrderID(int id)
{ *itsWOID = id; }

inline unsigned int DH_WorkOrder::getStatus() const
{ return *itsStatus; }

inline void DH_WorkOrder::setStatus(unsigned int status)
{ *itsStatus = status; }

inline string DH_WorkOrder::getKSType() const
{  return string(itsKSType); }

inline unsigned int DH_WorkOrder::getStrategyNo() const
{ return *itsStrategyNo; }

inline void DH_WorkOrder::setStrategyNo(unsigned int no)
{ *itsStrategyNo = no; }

inline int DH_WorkOrder::getNoStartSolutions() const
{ return *itsNoStartSols; }

inline void DH_WorkOrder::setNoStartSolutions(int no)
{ *itsNoStartSols = no; }

inline unsigned int DH_WorkOrder::getNumberOfParam() const
{ return *itsNumberOfParam; }

inline void DH_WorkOrder::setNumberOfParam(int number)
{ *itsNumberOfParam = number; }

inline int DH_WorkOrder::getCurDataSize() 
{ return itsCurDataSize; }
   
inline void DH_WorkOrder::setCurDataSize(const int nbytes)
{ itsCurDataSize = nbytes;  }

// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                                 
  class DBRep<DH_WorkOrder> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_WorkOrder&);                        
    private:                                                    
      int itsWOID;                    // Temporarily stored in separate fields
      unsigned int itsStatus;         // in order to facilitate debugging
      string itsKSType;
      unsigned int itsStrategyNo;
      int itsNoStartSols;
      unsigned int itsNumberOfParam;
    };   
                                                      
} // end namespace PL   

} // namespace LOFAR

#endif 

