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

namespace LOFAR {

// 3 macros are provided which can be used to construct a for loop:
//
// MATRIX_FOR_LOOP(matrix, dim, cursor)
//   loops through 1 full dimension(dim) of matrix from cursor to end
//
// MATRIX_FOR_LOOP_PART(matrix, dim, cursor, noElem)
//   loops through from cursor to cursor + noElem in the dimension dim of the matrix 
//
// e.g.
// cursor = matrix.getCursor(0)
// MATRIX_FOR_LOOP(matrix, dim, cursor) {
//   matrix.setValue(cursor, 0);
// } 


// the definition of a dimension
// this class is only used when constructing a rectmatrix
class DimDef {
 public:
  DimDef(const string& newName, const int newSize):
    name(newName),
    size(newSize)
    {};
  string name;
  int size;
};


// a dimension of a RectMatrix
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
  RectMatrix(vector<DimDef>&);
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
  
  // get a block of memory
  valueType* getBlock(const cursorType& cursor, const dimType& dim, const int noElem, const int noTotalElem) const;

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
    
#ifdef ENABLE_DBGASSERT
  // cursors are only made invalid when DBGASSERT is enabled
  bool isCursorValid(const cursorType& cursor) const;
#endif

 private:
  bool areInSameDim(const cursorType& cur1, const cursorType& cur2, const dimType& dim) const;

  friend class DataHolder;

  valueType* itsData;
  map<string, dimType*> itsDimMap;
  int itsTotalSize;
};

#ifdef ENABLE_DBGASSERT

// Macro for defining a for-loop that walks through all the elements of 1 dimension
// for unoptimized code we can loop until the cursor is invalid
#define MATRIX_FOR_LOOP(matrix, dim, cursor) \
    for (; matrix.isCursorValid(cursor); matrix.moveCursor(&cursor, dim)) 

// Macro for defining a for-loop that walks through 1 dimension from cursor to cursor + noElem
#define MATRIX_FOR_LOOP_PART(matrix, dim, cursor, noElem) \
    int cursorMax = cursor + dim.memsize * noElem;\
    for (; matrix.isCursorValid(cursor)&&(cursor < cursorMax); matrix.moveCursor(&cursor, dim)) 

#else //ENABLE_DBGASSERT

    // for optimized code we use a slightly different stop condition
    // this algorithm can only be used when the cursor is not invalidated
#define MATRIX_FOR_LOOP(matrix, dim, cursor) \
    int cursorMax = cursor + dim.total; \
    for (; cursor < cursorMax; matrix.moveCursor(&cursor, dim)) 

#define MATRIX_FOR_LOOP_PART(matrix, dim, cursor, noElem) \
    int cursorMax = cursor + dim.memsize * noElem;\
    for (; cursor < cursorMax; matrix.moveCursor(&cursor, dim)) 

#endif //ENABLE_DBGASSERT

#include <TFC_Interface/RectMatrix.tcc>

}
#endif
