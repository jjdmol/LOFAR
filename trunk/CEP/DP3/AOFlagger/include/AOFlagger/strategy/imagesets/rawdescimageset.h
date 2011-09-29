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
#ifndef RAWDESCIMAGESET_H
#define RAWDESCIMAGESET_H

#include <deque>
#include <set>
#include <vector>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/rawdescfile.h>

#include <AOFlagger/strategy/imagesets/imageset.h>

#include <AOFlagger/msio/rawreader.h>

namespace rfiStrategy {
	
	class RawDescImageSetIndex : public ImageSetIndex {
		public:
			RawDescImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _isValid(true), _timeBlockIndex(0)
			{
			}
			
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Raw file";
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual RawDescImageSetIndex *Copy() const
			{
				RawDescImageSetIndex *index = new RawDescImageSetIndex(imageSet());
				index->_isValid = _isValid;
				index->_timeBlockIndex = _timeBlockIndex;
				return index;
			}
		private:
			friend class RawDescImageSet;

			inline class RawDescImageSet &RawDescSet() const;
			
			bool _isValid;
			size_t _timeBlockIndex;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RawDescImageSet : public ImageSet
	{
		public:
			RawDescImageSet(const std::string &file) : _rawDescFile(file)
			{
				_readers = new RawReader*[_rawDescFile.GetCount()];
				for(size_t i=0;i!=_rawDescFile.GetCount();++i)
				{
					_readers[i] = new RawReader(_rawDescFile.GetSet(i));
				}
			}
			
			~RawDescImageSet()
			{
				for(size_t i=0;i!=_rawDescFile.GetCount();++i)
				{
					delete _readers[i];
				}
				delete[] _readers;
			}
			
			virtual void Initialize()
			{
			}

			virtual RawDescImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new RawDescImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Raw file";
			}
			virtual std::string File()
			{
				return _rawDescFile.Filename();
			}
			virtual TimeFrequencyData *LoadData(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetPart(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetAntenna1(const ImageSetIndex &)
			{
				return 0;
			}
			virtual size_t GetAntenna2(const ImageSetIndex &)
			{
				return 0;
			}
			virtual void AddReadRequest(const ImageSetIndex &index)
			{
				const RawDescImageSetIndex &rawIndex = static_cast<const RawDescImageSetIndex&>(index);
				size_t readSize = (size_t) round(60.0 / _rawDescFile.TimeResolution());
				size_t readStart = readSize * rawIndex._timeBlockIndex;
				
				Image2DPtr image = Image2D::CreateUnsetImagePtr(readSize, _rawDescFile.GetCount());
				float data[readSize];
				for(size_t y=0;y<_rawDescFile.GetCount();++y)
				{
					_readers[y]->Read(readStart, readStart + readSize, data, 0, 0, 0);
					for(size_t x=0;x<readSize;++x)
					{
						image->SetValue(x, y, (num_t) data[x]);
					}
				}
				TimeFrequencyData tfData(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);
				TimeFrequencyMetaDataPtr metaData(new TimeFrequencyMetaData());
				
				std::vector<double> observationTimes;
				for(unsigned t=0;t<readSize;++t)
				{
					observationTimes.push_back((t + readStart) * _rawDescFile.TimeResolution());
				}
				metaData->SetObservationTimes(observationTimes);
				
				BandInfo bandInfo;
				bandInfo.channelCount = _rawDescFile.GetCount();
				for(unsigned i=0;i<bandInfo.channelCount;++i)
				{
					ChannelInfo channel;
					channel.frequencyHz = _rawDescFile.FrequencyStart() + _rawDescFile.FrequencyResolution() * i;
					channel.frequencyIndex = i;
					bandInfo.channels.push_back(channel);
				}
				metaData->SetBand(bandInfo);
				
				BaselineData *baseline = new BaselineData(tfData, metaData, index);
				_baselineBuffer.push_back(baseline);
			}
			virtual void PerformReadRequests()
			{
			}
			virtual BaselineData *GetNextRequested()
			{
				BaselineData *baseline = _baselineBuffer.front();
				_baselineBuffer.pop_front();
				return baseline;
			}
		private:
			RawDescFile _rawDescFile;
			RawReader **_readers;
			std::deque<BaselineData*> _baselineBuffer;
	};

	void RawDescImageSetIndex::Previous()
	{
		if(_timeBlockIndex > 0)
		{
			--_timeBlockIndex;
		}
	}
	
	void RawDescImageSetIndex::Next()
	{
		++_timeBlockIndex;
	}
	
	void RawDescImageSetIndex::LargeStepPrevious()
	{
	}
	
	void RawDescImageSetIndex::LargeStepNext()
	{
		_isValid = false;
	}
	
	RawDescImageSet &RawDescImageSetIndex::RawDescSet() const
	{
		return static_cast<RawDescImageSet&>(imageSet());
	}
}

#endif // RAWDESCIMAGESET_H
