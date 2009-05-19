//# LofarColumn.cc: A Column in the LOFAR Storage Manager
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <LofarStMan/LofarColumn.h>

#include <tables/Tables/DataManError.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MUvw.h>
#include <casa/Arrays/Array.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

using namespace casa;


namespace LOFAR {

  LofarColumn::~LofarColumn()
  {}
  Bool LofarColumn::isWritable() const
  {
    return False;
  }
  void LofarColumn::setShapeColumn (const IPosition&)
  {}
  void LofarColumn::prepareCol()
  {}

  Ant1Column::~Ant1Column()
  {}
  void Ant1Column::getIntV (uInt rownr, Int* dataPtr)
  {
    // Fill ColumnCache object.
    const Block<Int>& ants = itsParent->ant1();
    columnCache().setIncrement (1);
    uInt strow = rownr / ants.size() * ants.size();
    columnCache().setIncrement (1);
    columnCache().set (strow, strow + ants.size() - 1, ants.storage());
    *dataPtr = ants[rownr-strow];
  }

  Ant2Column::~Ant2Column()
  {}
  void Ant2Column::getIntV (uInt rownr, Int* dataPtr)
  {
    // Fill ColumnCache object.
    const Block<Int>& ants = itsParent->ant2();
    uInt strow = rownr / ants.size() * ants.size();
    columnCache().setIncrement (1);
    columnCache().set (strow, strow + ants.size() - 1, ants.storage());
    *dataPtr = ants[rownr-strow];
  }

  TimeColumn::~TimeColumn()
  {}
  void TimeColumn::getdoubleV (uInt rownr, Double* dataPtr)
  {
    // Get time of the block containing this row.
    uInt nrbasel = itsParent->ant1().size();
    uInt blnr = rownr / nrbasel;
    itsValue = itsParent->time (blnr);
    // Fill ColumnCache object.
    uInt strow = blnr * nrbasel;
    columnCache().setIncrement (0);
    columnCache().set (strow, strow + nrbasel - 1, &itsValue);
    *dataPtr = itsValue;
  }

  IntervalColumn::~IntervalColumn()
  {}
  void IntervalColumn::getdoubleV (uInt, Double* dataPtr)
  {
    itsValue = itsParent->interval();
    columnCache().setIncrement (0);
    columnCache().set (0, itsParent->getNRow()-1, &itsValue);
    *dataPtr = itsValue;
  }

  ZeroColumn::~ZeroColumn()
  {}
  void ZeroColumn::getIntV (uInt, Int* dataPtr)
  {
    itsValue = 0;
    columnCache().setIncrement (0);
    columnCache().set (0, itsParent->getNRow()-1, &itsValue);
    *dataPtr = 0;
  }

  FalseColumn::~FalseColumn()
  {}
  void FalseColumn::getBoolV (uInt, Bool* dataPtr)
  {
    itsValue = False;
    columnCache().setIncrement (0);
    columnCache().set (0, itsParent->getNRow()-1, &itsValue);
    *dataPtr = 0;
  }

  UvwColumn::~UvwColumn()
  {}
  void UvwColumn::prepareCol()
  {
    // Read the station positions from the ANTENNA subtable
    // and convert them to a baseline in ITRF.
    const TableRecord& keyset = itsParent->table().keywordSet();
    itsCanCalc = keyset.isDefined ("ANTENNA");
    if (itsCanCalc) {
      Table anttab (keyset.asTable ("ANTENNA"));
      AlwaysAssert (anttab.nrow() > 0, AipsError);
      int nrant = anttab.nrow();
      ROScalarMeasColumn<MPosition> antcol (anttab, "POSITION");
      MPosition arrayPos;
      Vector<Double> pos0;
      for (int i=0; i<nrant; ++i) {
        // Read antenna position and convert to ITRF.
        MPosition mpos = MPosition::Convert (antcol(i), MPosition::ITRF)();
        if (i == 0) {
          pos0 = mpos.getValue().getVector();
        }
        // Use position of middle station as array position.
        if (i == nrant/2) {
          arrayPos = mpos;
        }
        Vector<Double> pos = mpos.getValue().getVector();
        MVPosition mvpos((pos[0] - pos0[0]),
                         (pos[1] - pos0[1]),
                         (pos[2] - pos0[2]));
        itsAntMB.push_back (MBaseline (MVBaseline(mvpos), MBaseline::ITRF));
      }
      // Read the phase reference position from the FIELD subtable.
      // Only use the first value from the PHASE_DIR array in J2000.
      Table fldtab (itsParent->table().keywordSet().asTable ("FIELD"));
      AlwaysAssert (fldtab.nrow() == 1, AipsError);
      ROArrayMeasColumn<MDirection> fldcol (fldtab, "PHASE_DIR");
      itsPhaseDir = MDirection::Convert (*(fldcol(0).data()),
                                         MDirection::J2000)();
      // Create a reference frame. Use the middle antenna as array position.
      itsFrame.set (arrayPos);
      itsFrame.set (itsPhaseDir);
      // Initialize the rest which is used to cache the UVW per antenna.
      // The cache is only useful if the MS is accessed in time order, but that
      // is normally the case.
      itsLastBlNr = -1;
      itsAntUvw.resize (nrant);
      itsUvwFilled.resize (nrant);
      itsUvwFilled = false;
    }
  }
  IPosition UvwColumn::shape (uInt)
  {
    return IPosition(1,3);
  }
  void UvwColumn::getArraydoubleV (uInt rownr, Array<Double>* dataPtr)
  {
    if (!itsCanCalc) {
      *dataPtr = 0.;
    } else {
      // Get nr of the block containing this row.
      int nrbasel = itsParent->ant1().size();
      int blnr   = rownr / nrbasel;
      int antinx = rownr - blnr * nrbasel;
      int ant1   = itsParent->ant1()[antinx];
      int ant2   = itsParent->ant2()[antinx];
      // If a different block (i.e. time), we have to calculate the UVWs.
      if (blnr != itsLastBlNr) {
        itsLastBlNr  = blnr;
        Quantum<Double> tm(itsParent->time(blnr), "s");
        itsFrame.set (MEpoch(MVEpoch(tm.get("d").getValue()), MEpoch::UTC));
        itsUvwFilled = false;
      }
      // Calculate the UVWs for this timestamp if not done yet.
      int ant = ant1;
      for (int i=0; i<2; ++i) {
        if (!itsUvwFilled[ant]) {
          MBaseline& mbl = itsAntMB[ant];
          mbl.getRefPtr()->set(itsFrame);       // attach frame
          MBaseline::Convert mcvt(mbl, MBaseline::J2000);
          MVBaseline bas = mcvt().getValue();
          MVuvw jvguvw(bas, itsPhaseDir.getValue());
          itsAntUvw[ant] = Muvw(jvguvw, Muvw::J2000).getValue().getVector();
          itsUvwFilled[ant] = true;
        }
        ant = ant2;
      }
      // The UVW of the baseline is the difference of the antennae.
      *dataPtr = itsAntUvw[ant1] - itsAntUvw[ant2];
    }
  }

  DataColumn::~DataColumn()
  {}
  Bool DataColumn::isWritable() const
  {
    return True;
  }
  IPosition DataColumn::shape (uInt)
  {
    return IPosition(2, itsParent->npol(), itsParent->nchan());
  }
  void DataColumn::getArrayComplexV (uInt rownr, Array<Complex>* dataPtr)
  {
    Bool deleteIt;
    Complex* data = dataPtr->getStorage(deleteIt);
    itsParent->getData (rownr, data);
    dataPtr->putStorage (data, deleteIt);
  }
  void DataColumn::putArrayComplexV (uInt rownr, const Array<Complex>* dataPtr)
  {
    Bool deleteIt;
    const Complex* data = dataPtr->getStorage(deleteIt);
    itsParent->putData (rownr, data);
    dataPtr->freeStorage (data, deleteIt);
  }

  FlagColumn::~FlagColumn()
  {}
  IPosition FlagColumn::shape (uInt)
  {
    return IPosition(2, itsParent->npol(), itsParent->nchan());
  }
  void FlagColumn::getArrayBoolV (uInt rownr, Array<Bool>* dataPtr)
  {
    uInt npol = itsParent->npol();
    const uShort* data = itsParent->getNSample (rownr, False);
    const uShort* dataEnd = data + itsParent->nchan();
    if (dataPtr->contiguousStorage()) {
      for (Array<Bool>::contiter iter=dataPtr->cbegin();
           data<dataEnd; ++data) {
        Bool flagged = (*data == 0);
        for (uInt i=0; i<npol; ++i, ++iter) {
          *iter = flagged;
        }
      }
    } else {
      for (Array<Bool>::iterator iter=dataPtr->begin();
           data<dataEnd; ++data, ++iter) {
        Bool flagged = (*data == 0);
        for (uInt i=0; i<npol; ++i, ++iter) {
          *iter = flagged;
        }
      }
    }
  }

  WeightColumn::~WeightColumn()
  {}
  IPosition WeightColumn::shape (uInt)
  {
    return IPosition(1, itsParent->nchan());
  }
  void WeightColumn::getArrayfloatV (uInt rownr, Array<Float>* dataPtr)
  {
    Double maxn = itsParent->maxnSample();
    const uShort* data = itsParent->getNSample (rownr, True);
    const uShort* dataEnd = data + dataPtr->size();
    if (dataPtr->contiguousStorage()) {
      for (Array<Float>::contiter iter=dataPtr->cbegin();
           data<dataEnd; ++data, ++iter) {
        *iter = *data / maxn;
      }
    } else {
      for (Array<Float>::iterator iter=dataPtr->begin();
           data<dataEnd; ++data, ++iter) {
        *iter = *data / maxn;
      }
    }
  }

  SigmaColumn::~SigmaColumn()
  {}
  IPosition SigmaColumn::shape (uInt)
  {
    return IPosition(1, itsParent->nchan());
  }
  void SigmaColumn::getArrayfloatV (uInt, Array<Float>* dataPtr)
  {
    Array<Float>::iterator iterend = dataPtr->end();
    for (Array<Float>::iterator iter=dataPtr->begin();
         iter != iterend; ++iter) {
      *iter = 1.;
    }
  }

  FlagCatColumn::~FlagCatColumn()
  {}
  Bool FlagCatColumn::isShapeDefined (uInt)
  {
    return False;
  }
  IPosition FlagCatColumn::shape (uInt)
  {
    throw DataManError ("LofarStMan: no data in column FLAG_CATEGORY");
  }

} //# end namespace
