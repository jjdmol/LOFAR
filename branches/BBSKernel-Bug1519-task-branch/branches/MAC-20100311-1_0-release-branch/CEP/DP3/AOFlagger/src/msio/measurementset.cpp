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
#include <iostream>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSTable.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ExprNode.h>

#include <AOFlagger/msio/measurementset.h>
#include <AOFlagger/msio/arraycolumniterator.h>
#include <AOFlagger/msio/scalarcolumniterator.h>

MeasurementSet::MeasurementSet(const std::string &newLocation, const MeasurementSet &formatExample)
	: _location(newLocation), _maxSpectralBandIndex(-1),
	_maxFrequencyIndex(-1), _maxScanIndex(-1), _cacheInitialized(false)
{
	casa::Table *table = formatExample.OpenTable(MainTable, false);
	table->copy(newLocation, casa::Table::New, true);
	delete table;
}

MeasurementSet::~MeasurementSet()
{
}

casa::Table *MeasurementSet::OpenTable(TableType type, bool update) const
{
	if(type == MainTable) {
		casa::Table *table;
		if(update)
			table = new casa::Table(_location, casa::Table::Update);
		else
			table = new casa::Table(_location);
		return table;
	} else
	return 0;
}

size_t MeasurementSet::MaxSpectralBandIndex()
{
	if(_maxSpectralBandIndex==-1) {
		casa::Table *table = OpenTable(MainTable);
		casa::ROScalarColumn<int> windowColumn(*table, "DATA_DESC_ID");
		ScalarColumnIterator<int> windowIter = ScalarColumnIterator<int>::First(windowColumn);
		for(size_t i=0;i<table->nrow();++i,++windowIter) {
			if((*windowIter) > _maxSpectralBandIndex)
				_maxSpectralBandIndex = (*windowIter);
		}
		delete table;
	}
	return _maxSpectralBandIndex;
}

size_t MeasurementSet::FrequencyCount()
{
	if(_maxFrequencyIndex==-1) {
		casa::Table *table = OpenTable(MainTable);
		casa::ROArrayColumn<casa::Complex> dataColumn(*table, "DATA");
		if(table->nrow() > 0) {
			const casa::IPosition &shape = dataColumn.shape(0);
			if(shape.nelements() > 1)
				_maxFrequencyIndex = shape[1];
			else
				_maxFrequencyIndex = 0;
		} else 
			_maxFrequencyIndex = 0;
		delete table;
	}
	return _maxFrequencyIndex;
}

void MeasurementSet::CalculateScanCounts()
{
	if(_maxScanIndex==-1) {
		casa::Table *table = OpenTable(MainTable);
		casa::ROScalarColumn<int> scanColumn(*table, "SCAN_NUMBER");
		ScalarColumnIterator<int> scanIter = ScalarColumnIterator<int>::First(scanColumn);
		for(size_t i=0;i<table->nrow();++i,++scanIter) {
			if((*scanIter) + 1 > _maxScanIndex)
				_maxScanIndex = (*scanIter) + 1;
			if((*scanIter) + 1 < _minScanIndex)
				_minScanIndex = (*scanIter) + 1;
		}
		delete table;
	}
}

void MeasurementSet::DataMerge(const MeasurementSet &source)
{
	casa::Table *sourceTable = source.OpenTable(MainTable);

	unsigned newRows = sourceTable->nrow();
	unsigned sourceCols = sourceTable->tableDesc().ncolumn();

	casa::Table *destTable = OpenTable(MainTable, true);
	unsigned rowIndex = destTable->nrow();

	std::cout << "Adding " << newRows << " new rows..." << std::endl;
	destTable->addRow (newRows, false);

	std::cout << "Copying cells " << rowIndex << "-" << (newRows+rowIndex) << " for all columns ... " << std::endl;

	for(unsigned i=0;i<sourceCols;++i)
	{
		const std::string name = sourceTable->tableDesc().columnNames()[i];
		if(name != "FLAG_CATEGORY" && name != "WEIGHT_SPECTRUM") {
			if(i>0)
				std::cout << ",";
			std::cout << name << std::flush;
			casa::ROTableColumn sourceColumn = casa::ROTableColumn(*sourceTable, name);
			casa::TableColumn destColumn = casa::TableColumn(*destTable, name);
			for(unsigned j=0;j<newRows;++j)
				destColumn.put(rowIndex+j, sourceColumn, j);
		}
	}
	std::cout << std::endl;

	delete destTable;
	delete sourceTable;
}

size_t MeasurementSet::AntennaCount()
{
	casa::MeasurementSet ms(_location);
	casa::Table antennaTable = ms.antenna();
	size_t count = antennaTable.nrow();
	return count;
}

struct AntennaInfo MeasurementSet::GetAntennaInfo(unsigned antennaId)
{
	casa::MeasurementSet ms(_location);
	casa::Table antennaTable = ms.antenna();
	unsigned count = antennaTable.nrow();
	if(antennaId >= count) {
		throw;
	}
	casa::ROArrayColumn<double> positionCol(antennaTable, "POSITION"); 
	casa::ROScalarColumn<casa::String> nameCol(antennaTable, "NAME");
	casa::ROScalarColumn<double> diameterCol(antennaTable, "DISH_DIAMETER");
	casa::ROScalarColumn<casa::String> mountCol(antennaTable, "MOUNT");
	casa::ROScalarColumn<casa::String> stationCol(antennaTable, "STATION");

	ROArrayColumnIterator<double> p = ROArrayColumnIterator<double>::First(positionCol);
	ScalarColumnIterator<casa::String> n = ScalarColumnIterator<casa::String>::First(nameCol);
	ScalarColumnIterator<double> d = ScalarColumnIterator<double>::First(diameterCol);
	ScalarColumnIterator<casa::String> m = ScalarColumnIterator<casa::String>::First(mountCol);
	ScalarColumnIterator<casa::String> s = ScalarColumnIterator<casa::String>::First(stationCol);
	unsigned index = 0;
	while(index != antennaId)
	{
		++index; ++p; ++n; ++d; ++m; ++s;
	}
	AntennaInfo info;
	info.diameter = *d;
	info.id = antennaId;
	info.name = *n;
	casa::Array<double> position = *p;
	casa::Array<double>::iterator i = position.begin();
	info.position.x = *i;
	++i;
	info.position.y = *i;
	++i;
	info.position.z = *i;
	info.mount = *m;
	info.station = *s;

	return info;
}

struct BandInfo MeasurementSet::GetBandInfo(unsigned bandIndex)
{
	BandInfo band;
	casa::MeasurementSet ms(_location);
	casa::Table spectralWindowTable = ms.spectralWindow();
	casa::ROScalarColumn<int> numChanCol(spectralWindowTable, "NUM_CHAN");
	casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");

	band.windowIndex = bandIndex;
	band.channelCount = numChanCol(bandIndex);

	const casa::Array<double> &frequencies = frequencyCol(bandIndex);
	casa::Array<double>::const_iterator frequencyIterator = frequencies.begin();

	for(unsigned channel=0;channel<band.channelCount;++channel) {
		ChannelInfo channelInfo;
		channelInfo.frequencyIndex = channel;
		channelInfo.frequencyHz = frequencies(casa::IPosition(1, channel));
		band.channels.push_back(channelInfo);

		++frequencyIterator;
	}

	return band;
}

size_t MeasurementSet::FieldCount()
{
	casa::MeasurementSet ms(_location);
	casa::Table fieldTable = ms.field();
	size_t fieldCount = fieldTable.nrow();
	return fieldCount;
}

struct FieldInfo MeasurementSet::GetFieldInfo(unsigned fieldIndex)
{
	casa::MeasurementSet ms(_location);
	casa::Table fieldTable = ms.field();
	casa::ROArrayColumn<double> delayDirectionCol(fieldTable, "DELAY_DIR");
	const casa::Array<double> &delayDirection = delayDirectionCol(fieldIndex);
	casa::Array<double>::const_iterator delayDirectionIterator = delayDirection.begin();

	FieldInfo field;
	field.delayDirectionRA = *delayDirectionIterator;
	++delayDirectionIterator;
	field.delayDirectionDec = *delayDirectionIterator;
	field.delayDirectionDecNegCos = cosl(-field.delayDirectionDec);
	field.delayDirectionDecNegSin = sinl(-field.delayDirectionDec);

	return field;
}

MSIterator::MSIterator(class MeasurementSet &ms, bool hasCorrectedData) : _row(0)
{
	_table = ms.OpenTable(MeasurementSet::MainTable, false);
	_antenna1Col = new casa::ROScalarColumn<int>(*_table, "ANTENNA1");
	_antenna2Col = new casa::ROScalarColumn<int>(*_table, "ANTENNA2");
	_dataCol = new casa::ROArrayColumn<casa::Complex>(*_table, "DATA");
	_flagCol = new casa::ROArrayColumn<bool>(*_table, "FLAG");
	if(hasCorrectedData)
		_correctedDataCol = new casa::ROArrayColumn<casa::Complex>(*_table, "CORRECTED_DATA");
	else
		_correctedDataCol = 0;
	_fieldCol = new casa::ROScalarColumn<int>(*_table, "FIELD_ID");
	_timeCol = new casa::ROScalarColumn<double>(*_table, "TIME");
	_scanNumberCol = new casa::ROScalarColumn<int>(*_table, "SCAN_NUMBER");
	_uvwCol = new casa::ROArrayColumn<double>(*_table, "UVW");
}

MSIterator::~MSIterator()
{
	delete _antenna1Col;
	delete _antenna2Col;
	delete _dataCol;
	if(_correctedDataCol != 0)
		delete _correctedDataCol;
	delete _flagCol;
	delete _timeCol;
	delete _fieldCol;
	delete _table;
	delete _scanNumberCol;
	delete _uvwCol;
}

void MeasurementSet::InitCacheData()
{
	std::cout << "Initializing ms cache data..." << std::endl; 
	MSIterator iterator(*this, false);
	size_t antenna1=0xFFFFFFFF, antenna2 = 0xFFFFFFFF;
	double time = nan("");
	for(size_t row=0;row<iterator.TotalRows();++row)
	{
		size_t cur_a1 = iterator.Antenna1();
		size_t cur_a2 = iterator.Antenna2();
		double cur_time = iterator.Time();
		if(cur_a1 != antenna1 || cur_a2 != antenna2)
		{
			bool exists = false;
			for(vector<pair<size_t,size_t> >::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
			{
				if(i->first == cur_a1 && i->second == cur_a2)
				{
					exists = true;
					break;
				}
			}
			if(!exists)
				_baselines.push_back(std::pair<size_t,size_t>(cur_a1, cur_a2));
			antenna1 = cur_a1;
			antenna2 = cur_a2;
		}
		if(cur_time != time)
		{
			_observationTimes.insert(cur_time);
			time = cur_time;
		}
		++iterator;
	}
	_cacheInitialized = true;
}
