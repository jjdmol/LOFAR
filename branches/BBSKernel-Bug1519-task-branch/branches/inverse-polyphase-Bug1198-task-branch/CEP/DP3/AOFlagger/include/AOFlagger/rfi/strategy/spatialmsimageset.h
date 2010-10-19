#ifndef SPATIALMSIMAGESET_H
#define SPATIALMSIMAGESET_H

#include <string>
#include <cstring>
#include <sstream>
#include <vector>

#include "imageset.h"

namespace rfiStrategy {

	class SpatialMSImageSetIndex : public ImageSetIndex {
		public:
			friend class SpatialMSImageSet;

			SpatialMSImageSetIndex(ImageSet &set) : ImageSetIndex(set), _timeIndex(0), _isValid(true)
			{
			}
			virtual void Previous();
			virtual void Next();
			virtual void LargeStepPrevious()
			{
				Previous();
			}
			virtual void LargeStepNext()
			{
				Next();
			}
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Time index " << _timeIndex;
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual ImageSetIndex *Copy() const
			{
				SpatialMSImageSetIndex *newIndex = new SpatialMSImageSetIndex(imageSet());
				newIndex->_timeIndex = _timeIndex;
				newIndex->_isValid = _isValid;
				return newIndex;
			}
		private:
			class SpatialMSImageSet &SMSSet() const;
			size_t _timeIndex;
			bool _isValid;
	};
	
	class SpatialMSImageSet : public ImageSet {
		public:
			SpatialMSImageSet(const std::string &location) : _set(location), _loader(_set)
			{
			}
			virtual ~SpatialMSImageSet()
			{
			}
			virtual ImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new SpatialMSImageSetIndex(*this);
			}
			virtual void Initialize()
			{
			}
			virtual std::string Name()
			{
				return "Spatial matrix"; 
			}
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index)
			{
				SpatialMSImageSetIndex &sIndex = static_cast<SpatialMSImageSetIndex&>(index);
				return new TimeFrequencyData(_loader.Load(sIndex._timeIndex));
			}
			virtual void LoadFlags(ImageSetIndex &/*index*/, TimeFrequencyData &/*destination*/)
			{
			}
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &/*index*/)
			{
				return TimeFrequencyMetaDataCPtr();
			}
			SpatialMatrixMetaData &SpatialMetaData()
			{
				return _loader.MetaData();
			}
			virtual void WriteFlags(ImageSetIndex &/*index*/, TimeFrequencyData &/*data*/)
			{
			}
			virtual size_t GetPart(ImageSetIndex &/*index*/)
			{
				return 0;
			}
			virtual size_t GetAntenna1(ImageSetIndex &/*index*/)
			{
				return 0;
			}
			virtual size_t GetAntenna2(ImageSetIndex &/*index*/)
			{
				return 0;
			}
			size_t GetTimeIndexCount()
			{
				return _loader.TimeIndexCount();
			}
		private:
			MeasurementSet _set;
			BaselineMatrixLoader _loader;
	};

	void SpatialMSImageSetIndex::Previous()
	{
		if(_timeIndex > 0)
			--_timeIndex;
		else
		{
			_timeIndex = SMSSet().GetTimeIndexCount()-1;
			_isValid = false;
		}
	}

	void SpatialMSImageSetIndex::Next()
	{
		++_timeIndex;
		if(_timeIndex == SMSSet().GetTimeIndexCount())
		{
			_timeIndex = 0;
			_isValid = false;
		}
	}

	class SpatialMSImageSet &SpatialMSImageSetIndex::SMSSet() const
	{
		return static_cast<SpatialMSImageSet&>(imageSet());
	}
}

#endif
