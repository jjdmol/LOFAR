//# RectMatrix.cc: A class that holds a rectangular matrix that can be
//# addressed fast and easy to use (i.e. no pointers etc)
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#include <lofar_config.h>

#include <RectMatrix.h>

#if 0
template <typename valueType>
RectMatrix<valueType>::RectMatrix(vector<DimDef> dimdefv) :
  itsData(0)
{
  vector<DimDef>::iterator it = dimdefv.rbegin();
  lastSize = 1;
  for (; it<dimdefv.rend(); it++) {
    itsDimMap[it->itsName] = lastSize;
    itsDimSizeMap[lastSize] = it->itsSize;
    lastSize *= it->itsSize;
  }
  itsTotalSize = lastSize;
}
#endif
