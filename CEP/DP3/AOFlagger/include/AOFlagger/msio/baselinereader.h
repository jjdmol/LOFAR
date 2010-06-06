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

#include "image2d.h"
#include "mask2d.h"
#include "measurementset.h"
#include "antennainfo.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BaselineReader {
	public:
		explicit BaselineReader(MeasurementSet &measurementSet);
		~BaselineReader();
		void AddRequest(size_t antenna1, size_t antenna2, size_t spectralWindow);
		void AddRequest(size_t antenna1, size_t antenna2, size_t spectralWindow, size_t startIndex, size_t endIndex)
		{
			addRequest(antenna1, antenna2, spectralWindow, startIndex, endIndex);
		}
		void ReadRequests();

		void SetReadFlags(bool readFlags) { _readFlags = readFlags; }
		void SetReadData(bool readData) { _readData = readData; }

		void WriteNewFlags(std::vector<Mask2DCPtr> newValues, int antenna1, int antenna2, int spectralWindow)
		{
			WriteNewFlagsPart(newValues, antenna1, antenna2, spectralWindow, 0, newValues[0]->Width());
		}
		void WriteNewFlagsPart(std::vector<Mask2DCPtr> newValues, int antenna1, int antenna2, int spectralWindow, size_t timeOffset, size_t timeEnd, size_t leftBorder=0, size_t rightBorder=0);
		void SetDataKind(enum DataKind kind) { _dataKind = kind; }

		class TimeFrequencyData GetNextResult(std::vector<class UVW> &uvw);
		void PartInfo(size_t maxTimeScans, size_t &timeScanCount, size_t &partCount);
	private:
		struct Request {
			int antenna1;
			int antenna2;
			int spectralWindow;
			size_t startIndex;
			size_t endIndex;
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

		void initObservationTimes(MeasurementSet &set);
		void initBaselineCache();
		void addRowToBaselineCache(int antenna1, int antenna2, int spectralWindow, size_t row);
		void readUVWData();
		void addRequestRows(Request request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);

		void addRequest(size_t antenna1, size_t antenna2, size_t spectralWindow, size_t startIndex, size_t endIndex)
		{
			Request request;
			request.antenna1 = antenna1;
			request.antenna2 = antenna2;
			request.spectralWindow = spectralWindow;
			request.startIndex = startIndex;
			request.endIndex = endIndex;
			_requests.push_back(request);
		}

		casa::ROArrayColumn<casa::Complex> *CreateDataColumn(DataKind kind, class casa::Table &table);
		void readTimeData(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model);
		void readTimeFlags(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<bool> flag);
		void readWeights(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<float> weight);

		class MeasurementSet *_measurementSet;
		enum DataKind _dataKind;
		bool _readData, _readFlags;
		
		std::vector<Request> _requests;
		std::vector<BaselineCacheItem> _baselineCache;
		std::vector<Result> _results;
		
		std::map<double,size_t> _observationTimes;
		size_t _polarizationCount;
};

#endif // BASELINEREADER_H
