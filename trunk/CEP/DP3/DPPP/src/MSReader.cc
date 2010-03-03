//# MSReader.cc: DPPP step reading from an MS
//# Copyright (C) 2010
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/MSReader.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/AverageInfo.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Conversion.h>
#include <iostream>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    MSReader::MSReader (const string& msName,
                        const ParameterSet& parset, const string& prefix)
      : itsMS         (msName),
        itsNrInserted (0),
        itsFlagCounter("MSReader")
    {
      // Get info from parset.
      itsStartChan        = parset.getUint   (prefix+"startchan", 0);
      uint nrChan         = parset.getUint   (prefix+"nchan", 0);
      string startTimeStr = parset.getString (prefix+"starttime", "");
      string endTimeStr   = parset.getString (prefix+"endtime", "");
      itsUseFlags         = parset.getBool   (prefix+"useflag", true);
      itsDataColName      = parset.getString (prefix+"datacolumn", "DATA");
      itsCountFlags       = parset.getBool   (prefix+"countflag", false);
      // Prepare the MS access and get time info.
      double startTime, endTime;
      prepare (startTime, endTime, itsInterval);
      // Start and end time can be given in the parset in case leading
      // or trailing time slots are missing.
      // They can also be used to select part of the MS.
      Quantity qtime;
      itsFirstTime = startTime;
      if (!startTimeStr.empty()) {
        if (!MVTime::read (qtime, startTimeStr)) {
          THROW (LOFAR::Exception, startTimeStr << " is an invalid date/time");
        }
        itsFirstTime = qtime.getValue("s");
        ASSERT (itsFirstTime < endTime);
        // Round to integer nr of intervals.
        if (itsFirstTime < startTime) {
          int nrt = int((startTime - itsFirstTime) / itsInterval + 0.5);
          itsFirstTime = startTime - nrt*itsInterval;
        }
      }
      itsLastTime = endTime;
      if (!endTimeStr.empty()) {
        if (!MVTime::read (qtime, endTimeStr)) {
          THROW (LOFAR::Exception, endTimeStr << " is an invalid date/time");
        }
        itsLastTime = qtime.getValue("s");
      }
      ASSERT (itsLastTime > itsFirstTime);
      itsNextTime = itsFirstTime;
      uint nAllChan = itsNrChan;
      // nchan=0 means until the last channel.
      ASSERTSTR (itsStartChan < nAllChan,
                 "startchan " << itsStartChan
                 << " exceeds nr of channels in MS");
      uint maxNrChan = nAllChan - itsStartChan;
      if (nrChan == 0) {
        itsNrChan = maxNrChan;
      } else {
        itsNrChan = std::min (nrChan, maxNrChan);
      }
      // Are all channels used?
      itsUseAllChan = itsStartChan==0 && itsNrChan==nAllChan;
      // Form the slicer to get channels and correlations from column.
      itsColSlicer = Slicer(IPosition(2, 0, itsStartChan),
                            IPosition(2, itsNrCorr, itsNrChan));
      // Form the slicer to get channels, corrs, and baselines from array.
      itsArrSlicer = Slicer(IPosition(3, 0, itsStartChan, 0),
                            IPosition(3, itsNrCorr, itsNrChan, itsNrBl));
      // Initialize the flag counters.
      if (itsCountFlags) {
        itsFlagCounter.init (itsNrBl, itsNrChan, itsNrCorr);
      }
    }

    MSReader::~MSReader()
    {}

    bool MSReader::process (const DPBuffer&)
    {
      // Use time from the current time slot in the MS.
      bool useIter = false;
      while (!itsIter.pastEnd()) {
        // Take time from row 0 in subset.
        double time = ROScalarColumn<double>(itsIter.table(), "TIME")(0);
        // Use the time slot if near or < nexttime, but > starttime.
        // In this way we cater for irregular times in some WSRT MSs.
        if (near(time, itsNextTime)  ||
            (time > itsFirstTime  &&  time < itsNextTime)) {
          itsNextTime = time;
          useIter = true;
          break;
        }
        if (time > itsNextTime) {
          // A time slot seems to be missing; insert one.
          break;
        }
        // Skip this time slot.
        itsIter.next();
      }
      // Stop if at the end.
      if (itsNextTime > itsLastTime  &&  !near(itsNextTime, itsLastTime)) {
        return false;
      }
      // Fill the buffer.
      itsBuffer.setTime (itsNextTime);
      ///cout << "read time " <<itsBuffer.getTime() - 4472025855.0<<endl;
      if (!useIter) {
        // Need to insert a fully flagged time slot.
        itsBuffer.setRowNrs (Vector<uint>());
        itsBuffer.getData().resize  (itsNrCorr, itsNrChan, itsNrBl);
        itsBuffer.getFlags().resize (itsNrCorr, itsNrChan, itsNrBl);
        itsBuffer.getData() = Complex();
        itsBuffer.getFlags() = true;
        // Calculate UVWs for them. Setup UVW object if not done yet.
        calcUVW();
        itsNrInserted++;
      } else {
        // Get from the MS.
        itsBuffer.setRowNrs (itsIter.table().rowNumbers(itsMS));
        ROArrayColumn<Complex> dataCol(itsIter.table(), itsDataColName);
        if (itsUseAllChan) {
          itsBuffer.setData (dataCol.getColumn());
        } else {
          itsBuffer.setData (dataCol.getColumn(itsColSlicer));
        }
        if (itsUseFlags) {
          ROArrayColumn<bool> flagCol(itsIter.table(), "FLAG");
          if (itsUseAllChan) {
            itsBuffer.setFlags (flagCol.getColumn());
          } else {
            itsBuffer.setFlags (flagCol.getColumn(itsColSlicer));
          }
          // Set flags if FLAG_ROW is set.
          ROScalarColumn<bool> flagrowCol(itsIter.table(), "FLAG_ROW");
          for (uint i=0; i<itsIter.table().nrow(); ++i) {
            if (flagrowCol(i)) {
              itsBuffer.getFlags()
                (IPosition(3,0,0,i),
                 IPosition(3,itsNrCorr-1,itsNrChan-1,i)) = true;
            }
          }
        } else {
          // Do not use FLAG from the MS.
          itsBuffer.getFlags().resize (itsNrCorr, itsNrChan, itsNrBl);
          itsBuffer.getFlags() = false;
        }
        // Flag invalid data (NaN, infinite).
        const Complex* dataPtr = itsBuffer.getData().data();
        bool* flagPtr = itsBuffer.getFlags().data();
        for (uint i=0; i<itsBuffer.getData().size();) {
          for (uint j=i; j<i+itsNrCorr; ++j) {
            if (!isFinite(dataPtr[j].real())  ||  !isFinite(dataPtr[j].imag())
                ||  flagPtr[j]) {
              // Flag all correlations if a single one is flagged.
              for (uint k=i; k<i+itsNrCorr; ++k) {
                flagPtr[k] = true;
              }
              break;
            }
          }
          i += itsNrCorr;
        }
        itsIter.next();
      }
      // Let the next step in the pipeline process this time slot.
      ASSERTSTR (itsBuffer.getData().shape()[2] == int(itsNrBl),
                 "#baselines is not the same for all time slots in the MS");
      getNextStep()->process (itsBuffer);
      ///      cout << "Reader: " << itsNextTime-4.75e9<<endl;
      itsNextTime += itsInterval;
      // Count the flags if needed.
      if (itsCountFlags) {
        const bool* flagPtr = itsBuffer.getFlags().data();
        for (uint i=0; i<itsNrBl; ++i) {
          for (uint j=0; j<itsNrChan; ++j) {
            if (*flagPtr) {
              itsFlagCounter.incrBaseline(i);
              itsFlagCounter.incrChannel(j);
            }
            flagPtr += itsNrCorr;    // only count 1st corr
          }
        }
      }
      return true;
    }

    void MSReader::finish()
    {
      getNextStep()->finish();
      cout << "MSReader inserted " << itsNrInserted << " missing time slots"
           << endl;
    }

    void MSReader::updateAverageInfo (AverageInfo& info)
    {
      info.init (itsNrCorr, itsStartChan, itsNrChan, itsNrBl,
                 int((itsLastTime - itsFirstTime)/itsInterval + 1.5),
                 itsInterval);
    }

    void MSReader::show (std::ostream& os) const
    {
      os << "MSReader" << std::endl;
      os << "  input MS:       " << msName() << std::endl;
      os << "  startchan:      " << itsStartChan << std::endl;
      os << "  nchan:          " << itsNrChan << std::endl;
      os << "  ncorrelations:  " << itsNrCorr << std::endl;
      os << "  nbaselines:     " << itsNrBl << std::endl;
      os << "  ntimes:         " << itsMS.nrow() / itsNrBl << std::endl;
      os << "  time interval:  " << itsInterval << std::endl;
      os << "  DATA column:    " << itsDataColName << std::endl;
    }

    void MSReader::showCounts (std::ostream& os) const
    {
      if (itsCountFlags) {
        os << endl << "Flag statistics of data read";
        os << endl << "============================" << endl;
        int64 nrtim = int((itsLastTime - itsFirstTime)/itsInterval + 1.5);
        itsFlagCounter.showBaseline (os, itsAnt1, itsAnt2, nrtim*itsNrChan);
        itsFlagCounter.showChannel  (os, nrtim);
      }
    }

    void MSReader::prepare (double& firstTime, double& lastTime,
                            double& interval)
    {
      ASSERT (itsMS.nrow() > 0);
      // Test if WEIGHT_SPECTRUM is present.
      TableDesc tdesc = itsMS.tableDesc();
      itsHasWeightSpectrum = false;
      if (tdesc.isColumn("WEIGHT_SPECTRUM")) {
        // The column is there, but it might not contain values. Test row 0.
        itsHasWeightSpectrum =
          ROArrayColumn<float>(itsMS, "WEIGHT_SPECTRUM").isDefined(0);
      }
      itsHasFullResFlags = tdesc.isColumn("LOFAR_FULL_RES_FLAG");
      if (itsHasFullResFlags) {
        ROTableColumn fullResFlagCol(itsMS, "LOFAR_FULL_RES_FLAG");
        itsFullResNChanAvg  = fullResFlagCol.keywordSet().asInt ("NCHAN_AVG");
        itsFullResNTimeAvg  = fullResFlagCol.keywordSet().asInt ("NTIME_AVG");
      } else {
        itsFullResNChanAvg = 1;
        itsFullResNTimeAvg = 1;
      }
      // Get the main table in the correct order.
      // Determine if the data are stored using LofarStMan.
      // If so, we know it is in time order.
      // (sorting on TIME with LofarStMan can be expensive).
      bool ordered = false;
      Record dminfo = itsMS.dataManagerInfo();
      for (unsigned i=0; i<dminfo.nfields(); ++i) {
        Record subrec = dminfo.subRecord(i);
        if (subrec.asString("TYPE") == "LofarStMan") {
          ordered = true;
          break;
        }
      }
      // If not in order, sort the main table (also on baseline).
      Table sortms(itsMS);
      Block<String> sortCols(3);
      sortCols[0] = "TIME";
      sortCols[1] = "ANTENNA1";
      sortCols[2] = "ANTENNA2";
      if (!ordered) {
        sortms = itsMS.sort(sortCols);
      }
      // Get first and last time and interval from MS.
      firstTime = ROScalarColumn<double>(sortms, "TIME")(0);
      lastTime  = ROScalarColumn<double>(sortms, "TIME")(sortms.nrow()-1);
      interval  = ROScalarColumn<double>(sortms, "INTERVAL")(0);
      // Create iterator over time. Do not sort again.
      itsIter = TableIterator (sortms, Block<String>(1, "TIME"),
                               TableIterator::Ascending,
                               TableIterator::NoSort);
      // Find the nr of corr, chan, and baseline.
      IPosition shp (ROArrayColumn<Complex>(itsMS, "DATA").shape(0));
      itsNrCorr = shp[0];
      itsNrChan = shp[1];
      itsNrBl   = itsIter.table().nrow();
      // Ensure we have only one band by checking the nr of unique baselines.
      Table sortab = itsIter.table().sort(sortCols, Sort::Ascending,
                                          Sort::QuickSort + Sort::NoDuplicates);
      ASSERTSTR (sortab.nrow() == itsNrBl,
                 "The MS appears to have multiple subbands");
      // Get the baselines.
      ROScalarColumn<int>(sortab, "ANTENNA1").getColumn (itsAnt1);
      ROScalarColumn<int>(sortab, "ANTENNA2").getColumn (itsAnt2);
      // Keep the row numbers of the first part to be used for the meta info
      // of possibly missing time slots.
      itsBaseRowNrs = itsIter.table().rowNumbers(itsMS);
    }

    void MSReader::calcUVW()
    {
      // Setup calculator object if not done yet.
      if (itsUVWCalc.empty()) {
        setupUVWCalc();
      }
      Matrix<double> uvws(3, itsNrBl);
      for (uint i=0; i<itsAnt1.size(); ++i) {
        uvws.column(i) = itsUVWCalc.getUVW (itsAnt1[i], itsAnt2[i],
                                            itsNextTime);
      }
      itsBuffer.setUVW (uvws);
    }

    void MSReader::setupUVWCalc()
    {
      // Read the station positions from the ANTENNA subtable.
      Table anttab = itsMS.keywordSet().asTable("ANTENNA");
      uint nant = anttab.nrow();
      ROScalarMeasColumn<MPosition> antcol (anttab, "POSITION");
      vector<MPosition> statPos;
      statPos.reserve (nant);
      for (uint i=0; i<nant; ++i) {
        statPos.push_back (antcol(i));
      }
      // Read the phase reference position from the FIELD subtable.
      // Only use the main value from the PHASE_DIR array.
      Table fldtab (itsMS.keywordSet().asTable ("FIELD"));
      AlwaysAssert (fldtab.nrow() == 1, AipsError);
      ROArrayMeasColumn<MDirection> fldcol (fldtab, "PHASE_DIR");
      // Create the UVW calculator.
      itsUVWCalc = UVWCalculator (*(fldcol(0).data()), statPos);
    }

    Matrix<double> MSReader::getUVW (const RefRows& rowNrs)
    {
      // Empty rownrs cannot happen for data, because in that case the buffer
      // should contain UVW for a missing time slot.
      ASSERT (! rowNrs.rowVector().empty());
      ROArrayColumn<double> dataCol(itsMS, "UVW");
      return dataCol.getColumnCells (rowNrs);
    }

    Cube<float> MSReader::getWeights (const RefRows& rowNrs)
    {
      if (rowNrs.rowVector().empty()) {
        // rowNrs can be empty if a time slot was inserted.
        Cube<float> weights(itsNrCorr, itsNrChan, itsNrBl);
        weights = 0;
        return weights;
      }
      // Get weights for entire spectrum if pesent.
      if (itsHasWeightSpectrum) {
        ROArrayColumn<float> wsCol(itsMS, "WEIGHT_SPECTRUM");
        // Using getColumnCells(rowNrs,itsColSlicer) fails for LofarStMan.
        // Hence work around it.
        Cube<float> weights = wsCol.getColumnCells (rowNrs);
        return (itsUseAllChan ? weights : weights(itsArrSlicer));
      }
      // No spectrum present, so get global weights and assign to each channel.
      ROArrayColumn<float> wCol(itsMS, "WEIGHT");
      Matrix<float> inArr = wCol.getColumnCells (rowNrs);
      Cube<float> outArr(itsNrCorr, itsNrChan, itsNrBl);
      const float* inPtr = inArr.data();
      float* outPtr = outArr.data();
      for (uint i=0; i<itsNrBl; ++i) {
        for (uint j=0; j<itsNrChan; ++j) {
          for (uint k=0; k<itsNrCorr; ++k) {
            *outPtr++ = inPtr[k];
          }
        }
        inPtr += itsNrCorr;
      }
      return outArr;
    }

    Cube<bool> MSReader::getFullResFlags (const RefRows& rowNrs)
    {
      // Return empty array if no fullRes flags.
      if (!itsHasFullResFlags  ||  rowNrs.rowVector().empty()) {
        return Cube<bool>();
      }
      ROArrayColumn<uChar> fullResFlagCol(itsMS, "LOFAR_FULL_RES_FLAG");
      int norigchan = itsNrChan * itsFullResNChanAvg;
      int origstart = itsStartChan * itsFullResNChanAvg;
      Array<uChar> chars = fullResFlagCol.getColumnCells (rowNrs);
      // The original flags are kept per channel, not per corr.
      // Per row the flags are stored as uchar[nchar,navgtime].
      // Each char contains a bit per channel, thus nchan/8 chars are needed.
      // ntimeavg is the nr of times used when averaging.
      // Return it as Cube<bool>[norigchan,ntimeavg,nrbl].
      IPosition chShape = chars.shape();
      IPosition ofShape(3, norigchan, chShape[1], chShape[2]);
      Cube<bool> flags(ofShape);
      // Now expand the bits to bools.
      // If all bits to convert are contiguous, do it all in one go.
      // Otherwise we have to iterate.
      if (ofShape[0] == chShape[0]*8) {
        Conversion::bitToBool (flags.data(), chars.data(), flags.size());
      } else {
        ASSERT (ofShape[0] < chShape[0]*8);
        const uChar* charsPtr = chars.data();
        bool* flagsPtr = flags.data();
        for (int i=0; i<ofShape[1]*ofShape[2]; ++i) {
          Conversion::bitToBool (flagsPtr, charsPtr, origstart, ofShape[0]);
          flagsPtr += ofShape[0];
          charsPtr += chShape[0];
        }
      }
      return flags;
    }

    Cube<Complex> MSReader::getData (const String& columnName,
                                     const RefRows& rowNrs)
    {
      // Empty rownrs cannot happen for data, because in that case the buffer
      // should contain data for a missing time slot.
      ASSERT (! rowNrs.rowVector().empty());
      ROArrayColumn<Complex> dataCol(itsMS, columnName);
      // Also work around LofarStMan/getColumnCells problem.
      Cube<Complex> data = dataCol.getColumnCells (rowNrs);
      return (itsUseAllChan ? data : data(itsArrSlicer));
    }

    void MSReader::putFlags (const RefRows& rowNrs,
                             const Cube<bool>& flags)
    {
      if (! rowNrs.rowVector().empty()) {
        itsMS.reopenRW();
        ArrayColumn<bool> flagCol(itsMS, "FLAG");
        flagCol.putColumnCells (rowNrs, itsColSlicer, flags);
      }
    }

    Vector<String> MSReader::antennaNames() const
    {
      Table tab(itsMS.keywordSet().asTable("ANTENNA"));
      ROScalarColumn<String> nameCol (tab, "NAME");
      return nameCol.getColumn();
    }

    Vector<double> MSReader::chanFreqs() const
    {
      Table tab(itsMS.keywordSet().asTable("SPECTRAL_WINDOW"));
      ROArrayColumn<double> freqCol (tab, "CHANFREQ");
      // Take only the channels used in the input.
      Vector<double> freqs(freqCol(0));
      return freqs(Slice(itsStartChan, itsNrChan));
    }

  } //# end namespace
}
