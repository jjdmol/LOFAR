#include <AOFlagger/msio/timestepaccessor.h>

#include <ms/MeasurementSets/MeasurementSet.h>

void TimestepAccessor::Open()
{
	assertNotOpen();

	_totalChannelCount = 0;
	_highestFrequency = 0.0;
	_lowestFrequency = 0.0;
	_totalRowCount = 0;
	_currentRow = 0;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;

		casa::MeasurementSet ms(set.path);
		set.table = new casa::Table(set.path, casa::Table::Update);

		// Check number of polarizations
		casa::Table polTable = ms.polarization();
		casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE");
		if(_polarizationCount==0 && i==_sets.begin())
			_polarizationCount = corTypeColumn(0).shape()[0];
		else if(_polarizationCount != corTypeColumn(0).shape()[0])
			throw TimestepAccessorException("Number of polarizations don't match!");

		// Find lowest and highest frequency and check order
		set.lowestFrequency = 0.0;
		set.highestFrequency = 0.0;
		casa::Table spectralWindowTable = ms.spectralWindow();
		casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");
		for(unsigned b=0;b<spectralWindowTable.nrow();++b)
		{
			casa::Array<double> frequencyArray = frequencyCol(b);
			casa::Array<double>::const_iterator frequencyIterator = frequencyArray.begin();
			while(frequencyIterator != frequencyArray.end())
			{
				double frequency = *frequencyIterator;
				if(set.lowestFrequency == 0.0) set.lowestFrequency = frequency;
				if(frequency < set.lowestFrequency || frequency <= set.highestFrequency)
					throw TimestepAccessorException("Channels or bands are not ordered in increasing frequency");
				set.highestFrequency = frequency;
				++frequencyIterator;
			}
		}
		if(set.lowestFrequency < _highestFrequency)
			throw TimestepAccessorException("Sub-bands are not given in order of increasing frequency");
		if(_lowestFrequency == 0.0) _lowestFrequency = set.lowestFrequency;
		_highestFrequency = set.highestFrequency;
		
		set.antenna1Column = new casa::ROScalarColumn<int>(*set.table, "ANTENNA1"),
		set.antenna2Column = new casa::ROScalarColumn<int>(*set.table, "ANTENNA2");
		set.timeColumn = new casa::ROScalarColumn<double>(*set.table, "TIME");
		set.dataColumn = new casa::ArrayColumn<casa::Complex>(*set.table, _columnName);
		set.uvwColumn = new casa::ROArrayColumn<double>(*set.table, "UVW");
		// Set some general values
		set.bandCount = spectralWindowTable.nrow();
		set.channelsPerBand = (*set.dataColumn)(0).shape()[1];
		_totalChannelCount += set.bandCount * set.channelsPerBand;
		if(_totalRowCount == 0)
			_totalRowCount = set.table->nrow();
		else
			if(_totalRowCount != set.table->nrow())
				throw TimestepAccessorException("Sets do not have equal number of rows");
	}
	if(_startRow < _totalRowCount)
		_currentRow = _startRow;
	else
		_currentRow = _totalRowCount;
	_endRow = _totalRowCount;
	_bufferSize = 10000;
	_readBuffer = new BufferItem[_bufferSize];
	_readBufferPtr = 0;
	_inReadBuffer = 0;
	_writeBuffer = new BufferItem[_bufferSize];
	_inWriteBuffer = 0;
	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.Allocate(_polarizationCount, _totalChannelCount);
		_writeBuffer[i].data.Allocate(_polarizationCount, _totalChannelCount);
	}
	_isOpen = true;
}

void TimestepAccessor::Close()
{
	assertOpen();
	
	EmptyWriteBuffer();
	
	_isOpen = false;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		delete i->antenna1Column;
		delete i->antenna2Column;
		delete i->timeColumn;
		delete i->dataColumn;
		delete i->uvwColumn;
		delete i->table;
	}
	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.Free(_polarizationCount);
		_writeBuffer[i].data.Free(_polarizationCount);
	}
	delete[] _readBuffer;
	delete[] _writeBuffer;
}

bool TimestepAccessor::ReadNext(TimestepAccessor::TimestepIndex &index, TimestepAccessor::TimestepData &data)
{
	assertOpen();

	if(_readBufferPtr >= _inReadBuffer)
	{
		if(!FillReadBuffer())
			return false;
	}

	const BufferItem &item = _readBuffer[_readBufferPtr];
	index.row = item.row;
	item.data.CopyTo(data, _polarizationCount, _totalChannelCount);
	
	++_readBufferPtr;
	return true;
}

bool TimestepAccessor::FillReadBuffer()
{
	if(_currentRow >= _endRow)
		return false;
	
	for(unsigned i=0;i<_bufferSize;++i)
	{
		_readBuffer[i].data.timestep = 0.0;
	}
	
	unsigned valIndex = 0;
	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;
		_inReadBuffer = 0;
		
		while(_inReadBuffer < _bufferSize && _currentRow + _inReadBuffer < _endRow)
		{
			TimestepData &data = _readBuffer[_inReadBuffer].data;
			unsigned long row = _currentRow + _inReadBuffer;
			_readBuffer[_inReadBuffer].row = row;
			
			// Check timestep & read u,v coordinates & antenna's
			if(data.timestep == 0.0) {
				data.timestep = (*set.timeColumn)(row);
				casa::Array<double> uvwArray = (*set.uvwColumn)(row);
				casa::Array<double>::const_iterator uvwIterator = uvwArray.begin();
				data.u = *uvwIterator;
				++uvwIterator;
				data.v = *uvwIterator;
				data.antenna1 = (*set.antenna1Column)(row);
				data.antenna2 = (*set.antenna2Column)(row);
			}
			else {
				if(data.timestep != ((*set.timeColumn)(row)))
					throw TimestepAccessorException("Sets do not have same time steps");
				if(data.antenna1 != (unsigned) ((*set.antenna1Column)(row)))
					throw TimestepAccessorException("Sets do not have same antenna1 ordering");
				if(data.antenna2 != (unsigned) ((*set.antenna2Column)(row)))
					throw TimestepAccessorException("Sets do not have same antenna2 ordering");
			}

			// Copy data from tables in arrays
			casa::Array<casa::Complex> dataArray = (*set.dataColumn)(row);
			casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
			unsigned currentIndex = valIndex;
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					data.realData[p][currentIndex] = (*dataIterator).real();
					data.imagData[p][currentIndex] = (*dataIterator).imag();
					++dataIterator;
				}
				++currentIndex;
			}
			++_inReadBuffer;
		}
		valIndex += set.channelsPerBand;
	}
	_currentRow += _inReadBuffer;
	_readBufferPtr = 0;
	return true;
}


void TimestepAccessor::Write(TimestepAccessor::TimestepIndex &index, const TimestepAccessor::TimestepData &data)
{
	assertOpen();

	if(_inWriteBuffer >= _bufferSize)
		EmptyWriteBuffer();
	
	BufferItem &item = _writeBuffer[_inWriteBuffer];
	data.CopyTo(item.data, _polarizationCount, _totalChannelCount);
	item.row = index.row;
	
	++_inWriteBuffer;
	++_writeActionCount;
}

void TimestepAccessor::EmptyWriteBuffer()
{
	unsigned valIndex = 0;
	
	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		const SetInfo &set = *i;
		for(unsigned writeBufferIndex = 0; writeBufferIndex < _inWriteBuffer; ++writeBufferIndex)
		{
			const BufferItem &item = _writeBuffer[writeBufferIndex];

			// Copy data from arrays in tables
			casa::Array<casa::Complex> dataArray = (*set.dataColumn)(item.row);
			casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
			for(unsigned f=0;f<set.channelsPerBand;++f)
			{
				unsigned currentIndex = valIndex;
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					(*dataIterator).real() = item.data.realData[p][currentIndex];
					(*dataIterator).imag() = item.data.imagData[p][currentIndex];
					++dataIterator;
				}
				++currentIndex;
			}
			set.dataColumn->basePut(item.row, dataArray);
		}
		valIndex += set.channelsPerBand;
	}
	_inWriteBuffer = 0;
}
