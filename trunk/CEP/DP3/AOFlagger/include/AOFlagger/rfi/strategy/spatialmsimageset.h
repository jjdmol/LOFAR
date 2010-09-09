#ifndef SPATIALMSIMAGESET_H
#define SPATIALMSIMAGESET_H

#include <string>
#include <cstring>
#include <sstream>
#include <stack>

#include "imageset.h"

#include <AOFlagger/msio/spatialmatrixmetadata.h>
#include <AOFlagger/msio/measurementset.h>
#include <AOFlagger/msio/baselinematrixloader.h>

namespace rfiStrategy {

	class SpatialMSImageSetIndex : public ImageSetIndex {
		public:
			friend class SpatialMSImageSet;

			SpatialMSImageSetIndex(ImageSet &set) : ImageSetIndex(set), _timeIndex(0), _channelIndex(0), _isValid(true)
			{
			}
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Time index " << _timeIndex << ", channel " << _channelIndex;
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
				newIndex->_channelIndex = _channelIndex;
				newIndex->_isValid = _isValid;
				return newIndex;
			}
		private:
			inline class SpatialMSImageSet &SMSSet() const;
			size_t _timeIndex;
			size_t _channelIndex;
			bool _isValid;
	};
	
	class SpatialMSImageSet : public ImageSet {
		public:
			SpatialMSImageSet(const std::string &location) : _set(location), _loader(_set), _cachedTimeIndex(GetTimeIndexCount())
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
			virtual std::string File()
			{
				return _set.Location(); 
			}
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index)
			{
				SpatialMSImageSetIndex &sIndex = static_cast<SpatialMSImageSetIndex&>(index);
				if(sIndex._timeIndex != _cachedTimeIndex)
				{
					_loader.LoadPerChannel(sIndex._timeIndex, _timeIndexMatrices);
					_cachedTimeIndex = sIndex._timeIndex;
				}
				TimeFrequencyData *result = new TimeFrequencyData(_timeIndexMatrices[sIndex._channelIndex]);
				return result;
			}
			virtual void LoadFlags(ImageSetIndex &/*index*/, TimeFrequencyData &/*destination*/)
			{
			}
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &/*index*/)
			{
				return TimeFrequencyMetaDataCPtr();
			}
			SpatialMatrixMetaData SpatialMetaData(ImageSetIndex &index)
			{
				SpatialMSImageSetIndex &sIndex = static_cast<SpatialMSImageSetIndex&>(index);
				SpatialMatrixMetaData metaData(_loader.MetaData());
				metaData.SetChannelIndex(sIndex._channelIndex);
				metaData.SetTimeIndex(sIndex._timeIndex);
				return metaData;
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
			size_t GetFrequencyCount()
			{
				return _loader.FrequencyCount();
			}
			virtual void AddReadRequest(ImageSetIndex &index)
			{
				_baseline.push(BaselineData(index));
			}
			virtual void PerformReadRequests()
			{
				TimeFrequencyData *data = LoadData(_baseline.top().Index());
				_baseline.top().SetData(*data);
				_baseline.top().SetMetaData(TimeFrequencyMetaDataPtr());
				delete data;
			}
			virtual BaselineData *GetNextRequested()
			{
				BaselineData data = _baseline.top();
				_baseline.pop();
				return new BaselineData(data);
			}
			virtual void AddWriteFlagsTask(ImageSetIndex &, std::vector<Mask2DCPtr> &)
			{
			}
			virtual void PerformWriteFlagsTask()
			{
			}
		private:
			MeasurementSet _set;
			BaselineMatrixLoader _loader;
			std::stack<BaselineData> _baseline;
			std::vector<TimeFrequencyData> _timeIndexMatrices;
			size_t _cachedTimeIndex;
	};

	void SpatialMSImageSetIndex::LargeStepPrevious()
	{
		if(_timeIndex > 0)
			--_timeIndex;
		else
		{
			_timeIndex = SMSSet().GetTimeIndexCount()-1;
			_isValid = false;
		}
	}

	void SpatialMSImageSetIndex::LargeStepNext()
	{
		++_timeIndex;
		if(_timeIndex == SMSSet().GetTimeIndexCount())
		{
			_timeIndex = 0;
			_isValid = false;
		}
	}

	void SpatialMSImageSetIndex::Previous()
	{
		if(_channelIndex > 0)
			--_channelIndex;
		else
		{
			_channelIndex = SMSSet().GetFrequencyCount()-1;
			LargeStepPrevious();
		}
	}

	void SpatialMSImageSetIndex::Next()
	{
		++_channelIndex;
		if(_channelIndex == SMSSet().GetFrequencyCount())
		{
			_channelIndex = 0;
			LargeStepNext();
		}
	}

	class SpatialMSImageSet &SpatialMSImageSetIndex::SMSSet() const
	{
		return static_cast<SpatialMSImageSet&>(imageSet());
	}
}

#endif
