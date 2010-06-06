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
#include <AOFlagger/msio/baselinereader.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <set>
#include <stdexcept>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableIter.h>

#include <AOFlagger/msio/arraycolumniterator.h>
#include <AOFlagger/msio/scalarcolumniterator.h>

BaselineReader::BaselineReader(MeasurementSet &measurementSet)
	: _measurementSet(&measurementSet), _dataKind(ObservedData), _readData(true), _readFlags(true),
	_polarizationCount(0)
{
}

BaselineReader::~BaselineReader()
{
}

void BaselineReader::initObservationTimes(MeasurementSet &set)
{
	std::cout << "Initializing observation times..." << std::endl;
	if(_observationTimes.size() == 0)
	{
		const std::set<double> &times = set.GetObservationTimesSet();
		unsigned index = 0;
		for(std::set<double>::const_iterator i=times.begin();i!=times.end();++i)
		{
			_observationTimes.insert(std::pair<double,size_t>(*i, index));
			++index; 
		}
	}
}

void BaselineReader::initBaselineCache()
{
	// Pass one time through the entire measurement set and store the rownumbers of
	// the baselines.
	if(_baselineCache.empty())
	{
		casa::Table *table = _measurementSet->OpenTable(MeasurementSet::MainTable);
		casa::ROScalarColumn<int> antenna1Column(*table, "ANTENNA1"); 
		casa::ROScalarColumn<int> antenna2Column(*table, "ANTENNA2");
		casa::ROScalarColumn<int> windowColumn(*table, "DATA_DESC_ID");
		for(size_t i=0;i<table->nrow();++i) {
			int
				antenna1 = antenna1Column(i),
				antenna2 = antenna2Column(i),
				window = windowColumn(i);
			addRowToBaselineCache(antenna1, antenna2, window, i);
		}
		delete table;
	}
}

void BaselineReader::addRowToBaselineCache(int antenna1, int antenna2, int spectralWindow, size_t row)
{
	for(std::vector<BaselineCacheItem>::iterator i=_baselineCache.begin();i!=_baselineCache.end();++i)
	{
		if(i->antenna1 == antenna1 && i->antenna2 == antenna2 && i->spectralWindow == spectralWindow)
		{
			i->rows.push_back(row);
			return;
		}
	}
	BaselineCacheItem newItem;
	newItem.antenna1 = antenna1;
	newItem.antenna2 = antenna2;
	newItem.spectralWindow = spectralWindow;
	newItem.rows.push_back(row);
	_baselineCache.push_back(newItem);
}

void BaselineReader::AddRequest(size_t antenna1, size_t antenna2, size_t spectralWindow)
{
	initObservationTimes(*_measurementSet);
	
	addRequest(antenna1, antenna2, spectralWindow, 0, _observationTimes.size());
}

void BaselineReader::addRequestRows(Request request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows)
{
	for(std::vector<BaselineCacheItem>::const_iterator i=_baselineCache.begin();i!=_baselineCache.end();++i)
	{
		if(i->antenna1 == request.antenna1 && i->antenna2 == request.antenna2 && i->spectralWindow == request.spectralWindow)
		{
			for(std::vector<size_t>::const_iterator j=i->rows.begin();j!=i->rows.end();++j)
				rows.push_back(std::pair<size_t, size_t>(*j, requestIndex));
			break;
		}
	}
}

void BaselineReader::ReadRequests()
{
	initObservationTimes(*_measurementSet);
	initBaselineCache();

	// Each element contains (row number, corresponding request index)
	std::vector<std::pair<size_t, size_t> > rows;
	
	for(size_t i=0;i!=_requests.size();++i)
		addRequestRows(_requests[i], i, rows);
	std::sort(rows.begin(), rows.end());
	
	size_t timeCount = _observationTimes.size();
	int frequencyCount = _measurementSet->FrequencyCount();

	initializePolarizations();

	std::cout << "Reading " << _requests.size() << " requests with " << rows.size() << " rows total, flags=" << _readFlags << ", " << _polarizationCount << " polarizations." << std::endl;
	
	_results.clear();
	for(size_t i=0;i<_requests.size();++i)
	{
		_results.push_back(Result());
		size_t
			startIndex = _requests[i].startIndex,
			endIndex = _requests[i].endIndex;
			
		if(startIndex > timeCount)
		{
			std::cerr << "Warning: startIndex > timeCount" << std::endl;
		}
		if(endIndex > timeCount)
		{
			endIndex = timeCount;
			std::cerr << "Warning: endIndex > timeCount" << std::endl;
		}

		size_t width = endIndex-startIndex;
		std::cout << "Request " << i << ", start=" << startIndex << ", end=" << endIndex << std::endl;
		for(size_t p=0;p<_polarizationCount;++p)
		{
			if(_readData) {
				_results[i]._realImages.push_back(Image2D::CreateEmptyImagePtr(width, frequencyCount));
				_results[i]._imaginaryImages.push_back(Image2D::CreateEmptyImagePtr(width, frequencyCount));
			}
			if(_readFlags) {
				// The flags should be initialized to true, as a baseline might
				// miss some time scans that other baselines do have, and these
				// should be flagged.
				_results[i]._flags.push_back(Mask2D::CreateSetMaskPtr<true>(width, frequencyCount));
			}
		}
		_results[i]._uvw.resize(width);
	}

	casa::Table *table = _measurementSet->OpenTable(MeasurementSet::MainTable, true);

	casa::ROScalarColumn<double> timeColumn(*table, "TIME");
	casa::ROArrayColumn<float> weightColumn(*table, "WEIGHT");
	casa::ROArrayColumn<double> uvwColumn(*table, "UVW");
	casa::ROArrayColumn<bool> flagColumn(*table, "FLAG");
	casa::ROArrayColumn<casa::Complex> *modelColumn;

	casa::ROArrayColumn<casa::Complex> *dataColumn = 0;
	if(_readData)
		dataColumn = CreateDataColumn(_dataKind, *table);

	if(_dataKind == ResidualData) {
		modelColumn = new casa::ROArrayColumn<casa::Complex>(*table, "MODEL_DATA");
	} else {
		modelColumn = 0;
	}

	for(std::vector<std::pair<size_t, size_t> >::const_iterator i=rows.begin();i!=rows.end();++i) {
		size_t rowIndex = i->first;
		size_t requestIndex = i->second;
		
		double time = timeColumn(rowIndex);
		size_t
			timeIndex = _observationTimes.find(time)->second,
			startIndex = _requests[requestIndex].startIndex,
			endIndex = _requests[requestIndex].endIndex;
		bool timeIsSelected = timeIndex>=startIndex && timeIndex<endIndex;
		if(_readData && timeIsSelected) {
			if(_dataKind == WeightData)
				readWeights(requestIndex, timeIndex-startIndex, frequencyCount, weightColumn(rowIndex));
			else if(modelColumn == 0)
				readTimeData(requestIndex, timeIndex-startIndex, frequencyCount, (*dataColumn)(rowIndex), 0);
			else {
				const casa::Array<casa::Complex> model = (*modelColumn)(rowIndex); 
				readTimeData(requestIndex, timeIndex-startIndex, frequencyCount, (*dataColumn)(rowIndex), &model);
			}
		}
		if(_readFlags && timeIsSelected) {
			readTimeFlags(requestIndex, timeIndex-startIndex, frequencyCount, flagColumn(rowIndex));
		}
		if(timeIsSelected) {
			casa::Array<double> arr = uvwColumn(rowIndex);
			casa::Array<double>::const_iterator i = arr.begin();
			_results[requestIndex]._uvw[timeIndex-startIndex].u = *i;
			++i;
			_results[requestIndex]._uvw[timeIndex-startIndex].v = *i;
			++i;
			_results[requestIndex]._uvw[timeIndex-startIndex].w = *i;
		}
	}
	if(dataColumn != 0)
		delete dataColumn;
	
	std::cout << "Orinal flags: " << _results[0]._flags[0]->GetCount<true>() << std::endl;

	_requests.clear();
}

casa::ROArrayColumn<casa::Complex> *BaselineReader::CreateDataColumn(DataKind kind, casa::Table &table)
{
	switch(kind) {
		case ObservedData:
		default:
		return new casa::ROArrayColumn<casa::Complex>(table, "DATA");
		case CorrectedData:
		case ResidualData:
		return new casa::ROArrayColumn<casa::Complex>(table, "CORRECTED_DATA");
		case ModelData:
		return new casa::ROArrayColumn<casa::Complex>(table, "MODEL_DATA");
	}
}

void BaselineReader::WriteNewFlagsPart(std::vector<Mask2DCPtr> newValues, int antenna1, int antenna2, int spectralWindow, size_t timeOffset, size_t timeEnd, size_t leftBorder, size_t rightBorder)
{
	initializePolarizations();

	size_t frequencyCount = _measurementSet->FrequencyCount();

	initObservationTimes(*_measurementSet);

	casa::Table *table = _measurementSet->OpenTable(MeasurementSet::MainTable, true);
	casa::ROScalarColumn<int> antenna1Column(*table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(*table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(*table, "DATA_DESC_ID");
	casa::ROScalarColumn<double> timeColumn(*table, "TIME");
	casa::ArrayColumn<bool> flagColumn(*table, "FLAG");

	ScalarColumnIterator<int> antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	ScalarColumnIterator<int> antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);
	ScalarColumnIterator<int> windowIter = ScalarColumnIterator<int>::First(windowColumn);
	ScalarColumnIterator<double> timeIter = ScalarColumnIterator<double>::First(timeColumn);
	ArrayColumnIterator<bool> flagIter = ArrayColumnIterator<bool>::First(flagColumn);

	if(frequencyCount != newValues[0]->Height())
	{
		std::cerr << "The frequency count in the measurement set (" << frequencyCount << ") does not match the image!" << std::endl;
	}
	if(timeEnd - timeOffset != newValues[0]->Width())
	{
		std::cerr << "The number of time scans to write in the measurement set (" << (timeEnd - timeOffset) << ") does not match the image (" << newValues[0]->Width() << ") !" << std::endl;
	}

	size_t rowsWritten = 0;
	for(size_t i=0;i<table->nrow();++i) {
		if((*antenna1Iter) == (int) antenna1 &&
		   (*antenna2Iter) == (int) antenna2 &&
		   (*windowIter) == (int) spectralWindow)
		{
			double time = *timeIter;
			size_t timeIndex = _observationTimes.find(time)->second;
			if(timeIndex >= timeOffset + leftBorder && timeIndex < timeEnd - rightBorder)
			{
				casa::Array<bool> flag = *flagIter;
				casa::Array<bool>::iterator j = flag.begin();
				for(size_t f=0;f<(size_t) frequencyCount;++f) {
					for(size_t p=0;p<_polarizationCount;++p)
					{
						*j = newValues[0]->Value(timeIndex - timeOffset, f);
						++j;
					}
				}
				flagIter.Set(flag);
				++rowsWritten;
			}
		}

		++antenna1Iter;
		++antenna2Iter;
		++timeIter;
		++windowIter;
		++flagIter;
	}
	std::cout << "Rows written: " << rowsWritten << std::endl;

	delete table;
}

void BaselineReader::readTimeData(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model)
{
	casa::Array<casa::Complex>::const_iterator i = data.begin();
	casa::Array<casa::Complex>::const_iterator m;
	if(_dataKind == ResidualData)
		m = model->begin();

	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		num_t rv,iv;

		for(size_t p=0;p<_polarizationCount;++p)
		{
			if(_dataKind == ResidualData)
			{
				const casa::Complex &iData = *i;
				const casa::Complex &iModel = *m;
				++i; ++m;

				rv = iData.real() - iModel.real();
				iv = iData.imag() - iModel.imag();
			} else {
				const casa::Complex &complex = *i;
				++i;

				rv = complex.real();
				iv = complex.imag();
			}
			_results[requestIndex]._realImages[p]->SetValue(xOffset, f, rv);
			_results[requestIndex]._imaginaryImages[p]->SetValue(xOffset, f, iv);
		}
	}
}

void BaselineReader::readTimeFlags(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<bool> flag)
{
	casa::Array<bool>::const_iterator j = flag.begin();
	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		for(size_t p=0;p<_polarizationCount;++p)
		{
			bool v = *j;
			++j;
			_results[requestIndex]._flags[p]->SetValue(xOffset, f, v);
		} 
	}
}

void BaselineReader::readWeights(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<float> weight)
{
	casa::Array<float>::const_iterator j = weight.begin();
	float values[_polarizationCount];
	for(size_t p=0;p<_polarizationCount;++p) {
		values[p] = *j;
		++j;
	}
	for(size_t f=0;f<(size_t) frequencyCount;++f) {
		for(size_t p=0;p<_polarizationCount;++p)
		{
			_results[requestIndex]._realImages[p]->SetValue(xOffset, f, values[p]);
			_results[requestIndex]._imaginaryImages[p]->SetValue(xOffset, f, 0.0);
		}
	} 
}

TimeFrequencyData BaselineReader::GetNextResult(std::vector<class UVW> &uvw)
{
	size_t requestIndex = 0;
	TimeFrequencyData data;
	if(_polarizationCount == 4)
	{
		data = TimeFrequencyData(
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0],
			_results[requestIndex]._realImages[1], _results[requestIndex]._imaginaryImages[1],
			_results[requestIndex]._realImages[2], _results[requestIndex]._imaginaryImages[2],
			_results[requestIndex]._realImages[3], _results[requestIndex]._imaginaryImages[3]
			);
		data.SetIndividualPolarisationMasks(
			_results[requestIndex]._flags[0],
			_results[requestIndex]._flags[1],
			_results[requestIndex]._flags[2],
			_results[requestIndex]._flags[3]);
	} else if(_polarizationCount == 2)
	{
		data = TimeFrequencyData(AutoDipolePolarisation,
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0],
			_results[requestIndex]._realImages[1], _results[requestIndex]._imaginaryImages[1]);
		data.SetIndividualPolarisationMasks(
			_results[requestIndex]._flags[0],
			_results[requestIndex]._flags[1]);
	} else if(_polarizationCount == 1)
	{
		data = TimeFrequencyData(StokesIPolarisation,
			_results[requestIndex]._realImages[0], _results[requestIndex]._imaginaryImages[0]);
		data.SetGlobalMask(_results[requestIndex]._flags[0]);
	}
	uvw = _results[0]._uvw;
	
	_results.erase(_results.begin() + requestIndex);

	return data;
}

void BaselineReader::PartInfo(size_t maxTimeScans, size_t &timeScanCount, size_t &partCount)
{
	initObservationTimes(*_measurementSet);

	timeScanCount = _observationTimes.size();
	if(maxTimeScans == 0)
		partCount = 1;
	else
	{
		partCount = (timeScanCount + maxTimeScans - 1) / maxTimeScans;
		if(partCount == 0)
			partCount = 1;
	}
}

void BaselineReader::initializePolarizations()
{
	if(_polarizationCount == 0)
	{
		casa::Table *table = _measurementSet->OpenTable(MeasurementSet::PolarizationTable, false);
		casa::ROArrayColumn<int> corTypeColumn(*table, "CORR_TYPE"); 
		casa::Array<int> corType = corTypeColumn(0);
		casa::Array<int>::iterator iterend(corType.end());
		int polarizationCount = 0;
		for (casa::Array<int>::iterator iter=corType.begin(); iter!=iterend; ++iter)
		{
			switch(*iter) {
				case 1: //_stokesIIndex = polarizationCount; break;
				case 5:
				case 9: //_xxIndex = polarizationCount; break;
				case 6:
				case 10:// _xyIndex = polarizationCount; break;
				case 7:
				case 11:// _yxIndex = polarizationCount; break;
				case 8:
				case 12: //_yyIndex = polarizationCount; break;
				break;
				default:
				{
					std::stringstream s;
					s << "There is a polarization in the measurement set that I can not handle (" << *iter << ", polarization index " << polarizationCount << ").";
					throw std::runtime_error(s.str());
				}
			}
			++polarizationCount;
		}
		_polarizationCount = polarizationCount;
		delete table;
	}
}
