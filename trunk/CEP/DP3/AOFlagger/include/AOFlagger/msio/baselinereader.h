/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BASELINEREADER_H
#define BASELINEREADER_H

#include <map>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "image2d.h"
#include "mask2d.h"
#include "measurementset.h"
#include "antennainfo.h"

typedef boost::shared_ptr<class BaselineReader> BaselineReaderPtr;
typedef boost::shared_ptr<const class BaselineReader> BaselineReaderCPtr;

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BaselineReader {
	public:
		explicit BaselineReader(const std::string &msFile);
		~BaselineReader();

		void SetReadFlags(bool readFlags) { _readFlags = readFlags; }
		void SetReadData(bool readData) { _readData = readData; }

		void AddReadRequest(size_t antenna1, size_t antenna2, size_t spectralWindow);
		void AddReadRequest(size_t antenna1, size_t antenna2, size_t spectralWindow, size_t startIndex, size_t endIndex)
		{
			addReadRequest(antenna1, antenna2, spectralWindow, startIndex, endIndex);
		}
		void PerformReadRequests();
		
		void AddWriteTask(std::vector<Mask2DCPtr> flags, int antenna1, int antenna2, int spectralWindow)
		{
			if(!flags.empty())
				AddWriteTask(flags, antenna1, antenna2, spectralWindow, 0, flags[0]->Width());
		}
		void AddWriteTask(std::vector<Mask2DCPtr> flags, int antenna1, int antenna2, int spectralWindow, size_t timeOffset, size_t timeEnd, size_t leftBorder=0, size_t rightBorder=0)
		{
			if(flags.size() != _polarizationCount)
			{
				std::stringstream s;
				s << "Trying to write image with " << flags.size() << " polarizations to a measurement set with " << _polarizationCount;
				throw std::runtime_error(s.str());
			}
			WriteRequest task;
			task.flags = flags;
			task.antenna1 = antenna1;
			task.antenna2 = antenna2;
			task.spectralWindow = spectralWindow;
			task.startIndex = timeOffset;
			task.endIndex = timeEnd;
			task.leftBorder = leftBorder;
			task.rightBorder = rightBorder;
			_writeRequests.push_back(task);
		}
		void PerformWriteRequests();
		
		void SetDataKind(enum DataKind kind) { _dataKind = kind; }

		class TimeFrequencyData GetNextResult(std::vector<class UVW> &uvw);
		void PartInfo(size_t maxTimeScans, size_t &timeScanCount, size_t &partCount);
		size_t PolarizationCount()
		{
			initializePolarizations();
			return _polarizationCount;
		}
	private:
		struct ReadRequest {
			int antenna1;
			int antenna2;
			int spectralWindow;
			size_t startIndex;
			size_t endIndex;
		};
		struct WriteRequest {
			WriteRequest() { }
			WriteRequest(const WriteRequest &source)
			: flags(source.flags), antenna1(source.antenna1), antenna2(source.antenna2), spectralWindow(source.spectralWindow), startIndex(source.startIndex), endIndex(source.endIndex),
			leftBorder(source.leftBorder), rightBorder(source.rightBorder)
			{
			}
			void operator=(const WriteRequest &source)
			{
				flags = source.flags;
				antenna1 = source.antenna1;
				antenna2 = source.antenna2;
				spectralWindow = source.spectralWindow;
				startIndex = source.startIndex;
				endIndex = source.endIndex;
				leftBorder = source.leftBorder;
				rightBorder = source.rightBorder;
			}
			std::vector<Mask2DCPtr> flags;
			int antenna1;
			int antenna2;
			int spectralWindow;
			size_t startIndex;
			size_t endIndex;
			size_t leftBorder;
			size_t rightBorder;
		};
		
		struct BaselineCacheItem
		{
			BaselineCacheItem() { }
			BaselineCacheItem(const BaselineCacheItem &source)
			: antenna1(source.antenna1), antenna2(source.antenna2), spectralWindow(source.spectralWindow), rows(source.rows)
			{
			}
			void operator=(const BaselineCacheItem &source)
			{
				antenna1 = source.antenna1;
				antenna2 = source.antenna2;
				spectralWindow = source.spectralWindow;
				rows = source.rows;
			}
			
			int antenna1, antenna2, spectralWindow;
			std::vector<size_t> rows;
		};
		
		struct Result {
			std::vector<Image2DPtr> _realImages;
			std::vector<Image2DPtr> _imaginaryImages;
			std::vector<Mask2DPtr> _flags;
			std::vector<class UVW> _uvw;
			class BandInfo _bandInfo;
		};
		
		void initializePolarizations();
		void initObservationTimes();
		void initBaselineCache();
		
		void addRowToBaselineCache(int antenna1, int antenna2, int spectralWindow, size_t row);
		void readUVWData();
		void addRequestRows(ReadRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);
		void addRequestRows(WriteRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);

		void addReadRequest(size_t antenna1, size_t antenna2, size_t spectralWindow, size_t startIndex, size_t endIndex)
		{
			ReadRequest request;
			request.antenna1 = antenna1;
			request.antenna2 = antenna2;
			request.spectralWindow = spectralWindow;
			request.startIndex = startIndex;
			request.endIndex = endIndex;
			_readRequests.push_back(request);
		}

		casa::ROArrayColumn<casa::Complex> *CreateDataColumn(DataKind kind, class casa::Table &table);
		void readTimeData(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model);
		void readTimeFlags(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<bool> flag);
		void readWeights(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<float> weight);

		MeasurementSet _measurementSet;
		enum DataKind _dataKind;
		bool _readData, _readFlags;
		
		std::vector<ReadRequest> _readRequests;
		std::vector<WriteRequest> _writeRequests;
		std::vector<BaselineCacheItem> _baselineCache;
		std::vector<Result> _results;
		
		std::map<double,size_t> _observationTimes;
		size_t _polarizationCount;
};

#endif // BASELINEREADER_H
