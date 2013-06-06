//# EmbraceColumn.h: A Column in the EMBRACE Storage Manager
//# Copyright (C) 2009
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef EMBRACE_EMBRACESTMAN_EMBRACECOLUMN_H
#define EMBRACE_EMBRACESTMAN_EMBRACECOLUMN_H


//# Includes
#include <EmbraceStMan/EmbraceStMan.h>
#include <Common/lofar_vector.h>
#include <tables/Tables/StManColumn.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MBaseline.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/Block.h>
#include <casa/OS/Conversion.h>

namespace EMBRACE {

// <summary>
// A column in the EMBRACE Storage Manager.
// </summary>

// <use visibility=local>

// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="tEmbraceStMan.cc">
// </reviewed>

// <prerequisite>
//# Classes you should understand before using this one.
//   <li> <linkto class=EmbraceStMan>EmbraceStMan</linkto>
// </prerequisite>

// <synopsis>
// For each column a specific Column class exists.
// </synopsis>

class EmbraceColumn : public casa::StManColumn
{
public:
  explicit EmbraceColumn (EmbraceStMan* parent, int dtype)
    : StManColumn (dtype),
      itsParent   (parent)
  {}
  virtual ~EmbraceColumn();
  // Most columns are not writable (only DATA is writable).
  virtual casa::Bool isWritable() const;
  // Set column shape of fixed shape columns; it does nothing.
  virtual void setShapeColumn (const casa::IPosition& shape);
  // Prepare the column. By default it does nothing.
  virtual void prepareCol();
protected:
  EmbraceStMan* itsParent;
};

// <summary>ANTENNA1 column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class Ant1Column : public EmbraceColumn
{
public:
  explicit Ant1Column (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~Ant1Column();
  virtual void getIntV (casa::uInt rowNr, casa::Int* dataPtr);
};

// <summary>ANTENNA2 column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class Ant2Column : public EmbraceColumn
{
public:
  explicit Ant2Column (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~Ant2Column();
  virtual void getIntV (casa::uInt rowNr, casa::Int* dataPtr);
};

// <summary>TIME and TIME_CENTROID column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class TimeColumn : public EmbraceColumn
{
public:
  explicit TimeColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~TimeColumn();
  virtual void getdoubleV (casa::uInt rowNr, casa::Double* dataPtr);
private:
  casa::Double itsValue;
};

// <summary>INTERVAL and EXPOSURE column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class IntervalColumn : public EmbraceColumn
{
public:
  explicit IntervalColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~IntervalColumn();
  virtual void getdoubleV (casa::uInt rowNr, casa::Double* dataPtr);
private:
  casa::Double itsValue;
};

// <summary>All columns in the EMBRACE Storage Manager with value 0.</summary>
// <use visibility=local>
class ZeroColumn : public EmbraceColumn
{
public:
  explicit ZeroColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~ZeroColumn();
  virtual void getIntV (casa::uInt rowNr, casa::Int* dataPtr);
private:
  casa::Int itsValue;
};

// <summary>All columns in the EMBRACE Storage Manager with value False.</summary>
// <use visibility=local>
class FalseColumn : public EmbraceColumn
{
public:
  explicit FalseColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~FalseColumn();
  virtual void getBoolV (casa::uInt rowNr, casa::Bool* dataPtr);
private:
  casa::Bool itsValue;
};

// <summary>UVW column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class UvwColumn : public EmbraceColumn
{
public:
  explicit UvwColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~UvwColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArraydoubleV (casa::uInt rowNr,
                                casa::Array<casa::Double>* dataPtr);
  virtual void prepareCol();
private:
  casa::MDirection              itsPhaseDir;    //# could be SUN, etc.
  casa::MDirection              itsJ2000Dir;    //# Phase dir in J2000
  casa::MeasFrame               itsFrame;
  vector<casa::MBaseline>       itsAntMB;
  vector<casa::Vector<double> > itsAntUvw;
  casa::Block<bool>             itsUvwFilled;
  int                           itsLastBlNr;
  bool                          itsCanCalc;     //# false = UVW cannot be calc.
};

// <summary>DATA column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class DataColumn : public EmbraceColumn
{
public:
  explicit DataColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~DataColumn();
  virtual casa::Bool isWritable() const;
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayComplexV (casa::uInt rowNr,
                                 casa::Array<casa::Complex>* dataPtr);
  virtual void putArrayComplexV (casa::uInt rowNr,
                                 const casa::Array<casa::Complex>* dataPtr);
};

// <summary>FLAG column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class FlagColumn : public EmbraceColumn
{
public:
  explicit FlagColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~FlagColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayBoolV (casa::uInt rowNr,
                              casa::Array<casa::Bool>* dataPtr);
};

// <summary>WEIGHT column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class WeightColumn : public EmbraceColumn
{
public:
  explicit WeightColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~WeightColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayfloatV (casa::uInt rowNr,
                               casa::Array<casa::Float>* dataPtr);
};

// <summary>SIGMA column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class SigmaColumn : public EmbraceColumn
{
public:
  explicit SigmaColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~SigmaColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayfloatV (casa::uInt rowNr,
                               casa::Array<casa::Float>* dataPtr);
};

// <summary>WEIGHT_SPECTRUM column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class WSpectrumColumn : public EmbraceColumn
{
public:
  explicit WSpectrumColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~WSpectrumColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayfloatV (casa::uInt rowNr,
                               casa::Array<casa::Float>* dataPtr);
};

// <summary>FLAG_CATEGORY column in the EMBRACE Storage Manager.</summary>
// <use visibility=local>
class FlagCatColumn : public EmbraceColumn
{
public:
  explicit FlagCatColumn (EmbraceStMan* parent, int dtype)
    : EmbraceColumn(parent, dtype) {}
  virtual ~FlagCatColumn();
  virtual casa::Bool isShapeDefined (casa::uInt rownr);
  virtual casa::IPosition shape (casa::uInt rownr);
};


} //# end namespace

#endif
