#include <AOFlagger/msio/timestepaccessor.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

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

		// Set some general values
		set.bandCount = spectralWindowTable.nrow();
		set.channelsPerBand = casa::ROArrayColumn<casa::Complex>(*set.table, "DATA")(0).shape()[1];
		_totalChannelCount += set.bandCount * set.channelsPerBand;
		if(_totalRowCount == 0)
			_totalRowCount = set.table->nrow();
		else
			if(_totalRowCount != set.table->nrow())
				throw TimestepAccessorException("Sets do not have equal number of rows");
	}
	_isOpen = true;
}

void TimestepAccessor::Close()
{
	assertOpen();

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		delete i->table;
	}
	_isOpen = false;
}

bool TimestepAccessor::ReadNext(TimestepAccessor::TimestepIndex &index, TimestepAccessor::TimestepData &data)
{
	assertOpen();

	double timeStep = 0.0;
	unsigned valIndex = 0;
	
	if(_currentRow >= _totalRowCount)
		return false;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		SetInfo &set = *i;
		casa::Table &table(*set.table);

		// Check timestep & read u,v coordinates & antenna's
		casa::ROScalarColumn<double> timeColumn = casa::ROScalarColumn<double>(table, "TIME");
		if(timeStep == 0.0) {
			casa::ROArrayColumn<double> uvwColumn = casa::ROArrayColumn<double>(table, "UVW");
			timeStep = timeColumn(_currentRow);
			casa::Array<double> uvwArray = uvwColumn(_currentRow);
			casa::Array<double>::const_iterator uvwIterator = uvwArray.begin();
			data.u = *uvwIterator;
			++uvwIterator;
			data.v = *uvwIterator;
			casa::ROScalarColumn<int>
				antenna1Column = casa::ROScalarColumn<int>(table, "ANTENNA1"),
				antenna2Column = casa::ROScalarColumn<int>(table, "ANTENNA2");
			data.antenna1 = antenna1Column(_currentRow);
			data.antenna2 = antenna2Column(_currentRow);
		}
		else if(timeStep != timeColumn(_currentRow))
			throw TimestepAccessorException("Sets do not have same time steps");

		// Copy data from tables in arrays
		casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
		casa::Array<casa::Complex> dataArray = dataColumn(_currentRow);
		casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
		for(unsigned f=0;f<set.channelsPerBand;++f)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				data.realData[p][valIndex] = (*dataIterator).real();
				data.imagData[p][valIndex] = (*dataIterator).imag();
				++dataIterator;
			}
			++valIndex;
		}
	}
	index.row = _currentRow;
	++_currentRow;
	return true;
}

void TimestepAccessor::Write(TimestepAccessor::TimestepIndex &index, const TimestepAccessor::TimestepData &data)
{
	assertOpen();

	unsigned valIndex = 0;

	for(SetInfoVector::iterator i=_sets.begin(); i!=_sets.end(); ++i)
	{
		const SetInfo &set = *i;

		// Copy data from arrays in tables
		casa::ArrayColumn<casa::Complex> dataColumn(*set.table, "DATA");
		casa::Array<casa::Complex> dataArray = dataColumn(index.row);
		casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
		for(unsigned f=0;f<set.channelsPerBand;++f)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				(*dataIterator).real() = data.realData[p][valIndex];
				(*dataIterator).imag() = data.imagData[p][valIndex];
				++dataIterator;
			}
			++valIndex;
		}
		dataColumn.basePut(index.row, dataArray);
	}
}
