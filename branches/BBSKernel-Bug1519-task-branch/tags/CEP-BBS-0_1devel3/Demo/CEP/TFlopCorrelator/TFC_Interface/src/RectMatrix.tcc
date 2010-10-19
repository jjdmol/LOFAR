//# RectMatrix.tcc: A class that holds a rectangular matrix that can be 
//# addressed fast and easy to use (i.e. no pointers etc)
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef TFLOP_CORRELATOR_MATRIX_TCC
#define TFLOP_CORRELATOR_MATRIX_TCC

#include<Common/LofarLogger.h>
#include<Common/lofar_vector.h>
#include<Common/lofar_map.h>
#include<Common/lofar_string.h>

using LOFAR::string;
using LOFAR::vector;
using LOFAR::map;


#if ENABLE_DBGASSERT
// this function can only be used in debug mode because
// cursor are not checked in optimized mode, so it can be invalid
template <typename valueType>
inline bool RectMatrix<valueType>::isCursorValid(const cursorType& cursor) const {
  return (cursor >= 0);
}
#endif


template <typename valueType>
RectMatrix<valueType>::RectMatrix(vector<DimDef>& dimdefv) :
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
  DBGASSERTSTR(itsDimMap.find(dimName) != itsDimMap.end(), "Cannot find dimension " << dimName << " in this RectMatrix");
  return *itsDimMap[dimName];};


template <typename valueType>
inline int RectMatrix<valueType>::getNElemInDim(const dimType& dim) const {
  return dim.noElem;};


template <typename valueType>
inline typename RectMatrix<valueType>::cursorType RectMatrix<valueType>::getCursor(const int pos) const { 
  return pos; };


template <typename valueType>
inline valueType& RectMatrix<valueType>::getValue(const cursorType& cursor) const {
  DBGASSERTSTR(isCursorValid(cursor), "Invalid cursor RectMatrix::getValue()");
  DBGASSERTSTR(cursor<itsTotalSize, "Index out of range in RectMatrix::getValue()");
  return itsData[cursor];};


template <typename valueType>
inline void RectMatrix<valueType>::setValue(const cursorType& cursor, const valueType& value) {
  DBGASSERTSTR(isCursorValid(cursor), "Invalid cursor RectMatrix::setValue()");
  DBGASSERTSTR(cursor<itsTotalSize, "Index out of range in RectMatrix::setValue()");
  itsData[cursor] = value; };


template <typename valueType>
inline void RectMatrix<valueType>::moveCursor(cursorType* cursorp, const dimType& dim) const {
  DBGASSERTSTR(isCursorValid(*cursorp), "Invalid cursor RectMatrix::moveCursor()");
#if ENABLE_DBGASSERT
  if (!areInSameDim(*cursorp, (*cursorp)+dim.memSize, dim)) 
  {
    *cursorp = -1;
  } else {
#else
  {
#endif
    *cursorp += dim.memSize;
  }
};


template <typename valueType>
inline void RectMatrix<valueType>::moveCursorN(cursorType* cursorp, const dimType& dim, const int& steps) const {
  DBGASSERTSTR(isCursorValid(*cursorp), "Invalid cursor RectMatrix::moveCursorN()");
#if ENABLE_DBGASSERT
  if (!areInSameDim(*cursorp, (*cursorp) + dim.memSize * steps, dim)) 
  {
    *cursorp = -1;
  } else {
#else
  {
#endif
    *cursorp += dim.memSize*steps; 
  }
};


template <typename valueType>
inline valueType* RectMatrix<valueType>::getBlock(const cursorType& cursor, const dimType& dim, const int noElemOfDim, const int noTotalElem) const {
  DBGASSERTSTR(isCursorValid(cursor), "Invalid cursor RectMatrix::getBlock()");
  DBGASSERTSTR(areInSameDim(cursor, cursor + dim.memSize * (noElemOfDim-1), dim), "Attempted to get block that crosses end of dimension");
  // this assert is only needed if we want to forbid getting pointers to higher dimension blocks
  //  DBGASSERTSTR(dim.memSize == sizeof(valueType), "Attempted to get block that is not of the lowest dimension");
  DBGASSERTSTR(dim.memSize * noElemOfDim == noTotalElem, "Attempted to get block but noTotalElem, noElemOfDim and dimension do not match");
  return &(itsData[cursor]); };


template <typename valueType>
inline void RectMatrix<valueType>::cpy2Matrix (cursorType srcCursor, 
					       dimType& srcDim,
					       RectMatrix& dstMatrix,
					       cursorType dstCursor, 
					       dimType& dstDim,
					       int noBlocks) {
  DBGASSERTSTR(isCursorValid(srcCursor), "Invalid srcCursor RectMatrix::cpy2Matrix()");
  DBGASSERTSTR(isCursorValid(dstCursor), "Invalid dstCursor RectMatrix::cpy2Matrix()");
  DBGASSERTSTR(srcDim.memSize == dstDim.memSize, "Dimension to be copied are not compatible");
  // This assert is not necessary anymore 
  //  DBGASSERTSTR((dstCursor+noBlocks*dstDim.memSize)<=dstMatrix.itsTotalSize, "Index out of range in destination in RectMatrix::cpy2Matrix");
  //  DBGASSERTSTR((srcCursor+noBlocks*srcDim.memSize)<=itsTotalSize, "Index out of range in source in RectMatrix::cpy2Matrix");
  DBGASSERTSTR(areInSameDim(dstCursor, dstCursor + dstDim.memSize * (noBlocks-1), dstDim), "Attempted to copy to matrix beyond end of dimension");
  DBGASSERTSTR(areInSameDim(srcCursor, srcCursor + srcDim.memSize * (noBlocks-1), srcDim), "Attempted to copy from matrix beyond end of dimension");
  memcpy(&(dstMatrix.itsData[dstCursor]), &(itsData[srcCursor]), noBlocks * srcDim.memSize * sizeof(valueType));
};


template <typename valueType>
inline void RectMatrix<valueType>::cpyFromBlock (valueType* srcPointer, 
						 int blockSize,
						 cursorType dstCursor, 
						 dimType& dstDim,
						 int noBlocks) {
  DBGASSERTSTR(isCursorValid(dstCursor), "Invalid cursor RectMatrix::cpyFromBlock()");
  DBGASSERTSTR(blockSize == dstDim.memSize, "Dimension to be copied  are not compatible");
  //  DBGASSERTSTR((dstCursor+noBlocks*dstDim.memSize)<=itsTotalSize, "Index out of range in destination in RectMatrix::cpyFromBlock");
  DBGASSERTSTR(areInSameDim(dstCursor, dstCursor + dstDim.memSize * (noBlocks - 1), dstDim), "Attempted to copy to matrix beyond end of dimension");
  memcpy(&(itsData[dstCursor]), srcPointer, noBlocks * blockSize * sizeof(valueType));
}


template <typename valueType>
inline void RectMatrix<valueType>::cpy2Block (cursorType srcCursor, 
					      dimType& srcDim,
					      valueType* dstPointer, 
					      int blockSize,
					      int noBlocks) {
  DBGASSERTSTR(isCursorValid(srcCursor), "Invalid cursor RectMatrix::cpy2Block()");
  DBGASSERTSTR(srcDim.memSize == blockSize, "Dimension to be copied  are not compatible");
  //  DBGASSERTSTR((srcCursor+noBlocks*srcDim.memSize)<=itsTotalSize, "Index out of range in source in RectMatrix::cpy2Block");
  DBGASSERTSTR(areInSameDim(srcCursor, srcCursor + srcDim.memSize * (noBlocks - 1), srcDim), "Attempted to copy from matrix beyond end of dimension");
  memcpy((void*)dstPointer, &(itsData[srcCursor]), noBlocks * blockSize * sizeof(valueType));
}


template <typename valueType>
inline void RectMatrix<valueType>::setBuffer(valueType* buffer, int size){
  DBGASSERTSTR(size == itsTotalSize, "cannot set buffer because of size mismatch");
  itsData = buffer;
};


template <typename valueType>
inline bool RectMatrix<valueType>::areInSameDim(const cursorType& cur1, const cursorType& cur2, const dimType& dim) const {
  DBGASSERTSTR(isCursorValid(cur1), "Invalid cursor RectMatrix::areInSameDim()");
  DBGASSERTSTR(isCursorValid(cur2), "Invalid cursor RectMatrix::areInSameDim()");
  // check if the cursors have the same position in this dimension
  // dim.total is the size of this dimension
  return ((cur1/dim.total) == (cur2/dim.total)); };

#endif
