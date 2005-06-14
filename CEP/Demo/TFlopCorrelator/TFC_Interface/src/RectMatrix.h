//# RectMatrix.h: A class that holds a rectangular matrix that can be 
//# addressed fast and easy to use (i.e. no pointers etc)
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef TFLOP_CORRELATOR_MATRIX
#define TFLOP_CORRELATOR_MATRIX

#include<Common/LofarLogger.h>
#include<Common/lofar_vector.h>
#include<Common/lofar_map.h>
#include<Common/lofar_string.h>

using LOFAR::string;
using LOFAR::vector;
using LOFAR::map;

class DimDef {
 public:
  DimDef(const string& newName, const int newSize):
    name(newName),
    size(newSize)
    {};
  string name;
  int size;
};

class dimType{
 public:
  dimType(int ms = 0, int ne = 0) :
    memSize(ms),
    noElem(ne){total = ms*ne;};
  int memSize; // how many values from this element to the next element in this dimension
  int noElem;  // how many elements in this dimension
  int total;   // memSize*noElem, used for for-loops
  int operator*(int steps) {
    return steps * memSize;};
  friend int operator*(int steps, dimType& dim) {
    return steps * dim.memSize;};
};


// this class holds a matrix
// for example programs using this class see tRectMatrix.cc in the test directory

template <typename valueType>
class RectMatrix {
 public:
  RectMatrix(vector<DimDef>);
  //  ~RectMatrix();
  
  typedef int cursorType; // is the offset of the datamember in itsData
  
  // get a dimension identifier by name
  dimType& getDim(string dimName);
  // get number of elements in this dimension
  // don't use this function to much because it uses a map!
  int getNElemInDim(const dimType& dim) const;
  // get a cursor at a certain position in the matrix
  // use this as 4*firstDim + 2* secondDim for element(4,2)
  cursorType getCursor(const int pos) const; 
  // get the value at the position of the cursor
  valueType& getValue(const cursorType& cursor) const;
  // set the value at the position of the cursor
  void setValue(const cursorType& cursor, const valueType& value);
  // move the cursor in a dimension
  void moveCursor(cursorType* cursor, const dimType& dim) const;
  // move the cursor in a dimension a certain number of steps
  void moveCursorN(cursorType* cursor, const dimType& dim, const int& steps) const;
  
  // copy a subblock of the matrix
  // note that the lower dimensions of both matrixes have to match
  // it makes no sense to copy a block of information if it doesn't have the same
  // meaning in the other matrix
  void cpy2Matrix (cursorType srcCursor, 
		   dimType& srcDim,
		   RectMatrix& dstMatrix,
		   cursorType dstCursor, 
		   dimType& dstDim,
		   int noBlocks);
  void cpyFromBlock (valueType* srcPointer, 
		     int blockSize,
		     cursorType dstCursor, 
		     dimType& dim,
		     int noBlocks);
  void cpy2Block (cursorType srcCursor, 
		  dimType& dim,
		  valueType* dstPointer, 
		  int blockSize,
		  int noBlocks);
  // point the matrix to the memory it should use
  void setBuffer(valueType* buffer, int size);
    
 private:
  friend class DataHolder;

  valueType* itsData;
  map<string, dimType*> itsDimMap;
  int itsTotalSize;
};

template <typename valueType>
RectMatrix<valueType>::RectMatrix(vector<DimDef> dimdefv) :
  itsData(0)
{
  vector<DimDef>::reverse_iterator it = dimdefv.rbegin();
  int lastSize = 1;
  for (; it<dimdefv.rend(); it++) {
    itsDimMap[it->name] = new dimType(lastSize, it->size);
    lastSize *= it->size;
  }
  itsTotalSize = lastSize;
}

template <typename valueType>
inline dimType& RectMatrix<valueType>::getDim(string dimName) {
  return *itsDimMap[dimName];};

template <typename valueType>
inline int RectMatrix<valueType>::getNElemInDim(const dimType& dim) const {
  return dim.noElem;};

template <typename valueType>
inline typename RectMatrix<valueType>::cursorType RectMatrix<valueType>::getCursor(const int pos) const { 
  return pos; };

template <typename valueType>
inline valueType& RectMatrix<valueType>::getValue(const cursorType& cursor) const {
  DBGASSERTSTR(cursor<itsTotalSize, "Index out of range in RectMatrix::getValue()");
  return itsData[cursor];};

template <typename valueType>
inline void RectMatrix<valueType>::setValue(const cursorType& cursor, const valueType& value) {
  DBGASSERTSTR(cursor<itsTotalSize, "Index out of range in RectMatrix::setValue()");
  itsData[cursor] = value; };

template <typename valueType>
inline void RectMatrix<valueType>::moveCursor(cursorType* cursorp, const dimType& dim) const {
  *cursorp += dim.memSize;};

template <typename valueType>
inline void RectMatrix<valueType>::moveCursorN(cursorType* cursorp, const dimType& dim, const int& steps) const {
  *cursorp += dim.memSize*steps; };

template <typename valueType>
inline void RectMatrix<valueType>::cpy2Matrix (cursorType srcCursor, 
					       dimType& srcDim,
					       RectMatrix& dstMatrix,
					       cursorType dstCursor, 
					       dimType& dstDim,
					       int noBlocks) {
  DBGASSERTSTR(srcDim.memSize == dstDim.memSize, "Dimension to be copied  are not compatible");
  DBGASSERTSTR((dstCursor+noBlocks*dstDim.memSize)<=dstMatrix.itsTotalSize, "Index out of range in destination in RectMatrix::cpy2Matrix");
  DBGASSERTSTR((srcCursor+noBlocks*srcDim.memSize)<=itsTotalSize, "Index out of range in source in RectMatrix::cpy2Matrix");
  memcpy(dstMatrix.itsData[dstCursor], itsData[srcCursor], noBlocks * srcDim.memSize * sizeof(valueType));
};
template <typename valueType>
inline void RectMatrix<valueType>::cpyFromBlock (valueType* srcPointer, 
						 int blockSize,
						 cursorType dstCursor, 
						 dimType& dstDim,
						 int noBlocks) {
  DBGASSERTSTR(blockSize == dstDim.memSize, "Dimension to be copied  are not compatible");
  cerr<<dstCursor<<" "<<dstDim.memSize<<" "<<itsTotalSize<<endl;
  DBGASSERTSTR((dstCursor+noBlocks*dstDim.memSize)<=itsTotalSize, "Index out of range in destination in RectMatrix::cpyFromBlock");
  memcpy(&(itsData[dstCursor]), srcPointer, noBlocks * blockSize * sizeof(valueType));
}
template <typename valueType>
inline void RectMatrix<valueType>::cpy2Block (cursorType srcCursor, 
					      dimType& srcDim,
					      valueType* dstPointer, 
					      int blockSize,
					      int noBlocks) {
  DBGASSERTSTR(srcDim.memSize == blockSize, "Dimension to be copied  are not compatible");
  DBGASSERTSTR((srcCursor+noBlocks*srcDim.memSize)<=itsTotalSize, "Index out of range in source in RectMatrix::cpy2Block");
  memcpy((void*)dstPointer, *(itsData[srcCursor]), noBlocks * blockSize * sizeof(valueType));
}

template <typename valueType>
inline void RectMatrix<valueType>::setBuffer(valueType* buffer, int size){
  DBGASSERTSTR(size == itsTotalSize, "cannot set buffer because of size mismatch");
  itsData = buffer;
};
    
#define MATRIX_FOR_LOOP(matrix, dim, cursor) \
int cursorMax = cursor + dim.total;\
    for (; cursor < cursorMax; matrix.moveCursor(&cursor, dim))
#define MATRIX_FOR_LOOP_OLD(matrix, dim, cursor) \
    for (int cursorIT = 0; cursorIT < matrix.getNElemInDim(dim); cursorIT ++ , matrix.moveCursor(&cursor, dim))

#endif
