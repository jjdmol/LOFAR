#ifndef BASELINEMATRIXLOADER_H
#define BASELINEMATRIXLOADER_H

#include <cstring>

#include "timefrequencydata.h"
#include "measurementset.h"

class BaselineMatrixLoader
{
	public:
		explicit BaselineMatrixLoader(MeasurementSet &measurementSet);
		~BaselineMatrixLoader();

		TimeFrequencyData Load(size_t timestep);

		size_t TimeIndexCount() const { return _timeIndexCount; }
		class SpatialMatrixMetaData &MetaData() const
		{
			return *_metaData;
		}
	private:
		casa::Table *_sortedTable;
		MeasurementSet _measurementSet;
		size_t _timeIndexCount;
		class SpatialMatrixMetaData *_metaData;
};

#endif
