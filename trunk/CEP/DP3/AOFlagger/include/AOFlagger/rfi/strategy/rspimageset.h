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
#ifndef RSPIMAGESET_H
#define RSPIMAGESET_H

#include <vector>
#include <set>

#include <AOFlagger/baseexception.h>

#include <AOFlagger/rfi/strategy/imageset.h>

#include <AOFlagger/msio/rspreader.h>
#include <deque>

namespace rfiStrategy {
	
	class RSPImageSetIndex : public ImageSetIndex {
		public:
			RSPImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _beamlet(0), _timeBlock(0), _isValid(true) { }
			
			inline virtual void Previous();
			inline virtual void Next();
			inline virtual void LargeStepPrevious();
			inline virtual void LargeStepNext();
			
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Raw file, time block " << _timeBlock <<", beamlet " << _beamlet;
				return s.str();
			}
			virtual bool IsValid() const
			{
				return _isValid;
			}
			virtual RSPImageSetIndex *Copy() const
			{
				RSPImageSetIndex *index = new RSPImageSetIndex(imageSet());
				index->_beamlet = _beamlet;
				index->_timeBlock = _timeBlock;
				index->_isValid = _isValid;
				return index;
			}
		private:
			friend class RSPImageSet;

			inline class RSPImageSet &RSPSet() const;
			
			unsigned long _beamlet, _timeBlock;
			bool _isValid;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RSPImageSet : public ImageSet
	{
		public:
			enum Mode { AllBeamletsMode, SingleBeamletMode, BeamletChannelMode };
			
			RSPImageSet(const std::string &file) : _reader(file), _mode(BeamletChannelMode)
			{
				_timestepCount = _reader.TimeStepCount(5);
				if(_mode == BeamletChannelMode) _timestepCount /= (unsigned long) 256;
			}
			~RSPImageSet()
			{
			}
			virtual void Initialize()
			{
			}

			virtual RSPImageSet *Copy()
			{
				return 0;
			}

			virtual ImageSetIndex *StartIndex()
			{
				return new RSPImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Raw RSP file";
			}
			virtual std::string File()
			{
				return _reader.File();
			}
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index)
			{
				return 0;
			}
			virtual size_t GetPart(ImageSetIndex &)
			{
				return 0;
			}
			virtual void WriteFlags(ImageSetIndex &, TimeFrequencyData &)
			{
				throw BadUsageException("RSP format does not support writing of flags");
			}
			virtual size_t GetAntenna1(ImageSetIndex &index)
			{
				return 0;
			}
			virtual size_t GetAntenna2(ImageSetIndex &index)
			{
				return 0;
			}
			virtual void AddReadRequest(ImageSetIndex &index)
			{
				RSPImageSetIndex &rspIndex = static_cast<RSPImageSetIndex&>(index);
				std::pair<TimeFrequencyData,TimeFrequencyMetaDataPtr> data;
				switch(_mode)
				{
					case AllBeamletsMode:
						data = _reader.ReadAllBeamlets(rspIndex._timeBlock * TimeblockSize(), (rspIndex._timeBlock+1ul) * TimeblockSize(), 5);
						break;
					case SingleBeamletMode:
						data = _reader.ReadSingleBeamlet(rspIndex._timeBlock * TimeblockSize(), (rspIndex._timeBlock+1ul) * TimeblockSize(), 5, rspIndex._beamlet);
						break;
					case BeamletChannelMode:
						data = _reader.ReadChannelBeamlet(rspIndex._timeBlock * TimeblockSize(), (rspIndex._timeBlock+1ul) * TimeblockSize(), 5, rspIndex._beamlet);
						break;
				}
				BaselineData *baseline = new BaselineData(data.first, data.second, index);
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
			virtual void AddWriteFlagsTask(ImageSetIndex &, std::vector<Mask2DCPtr> &)
			{
				throw BadUsageException("RSP format does not support writing of flags");
			}
			virtual void PerformWriteFlagsTask()
			{
				throw BadUsageException("RSP format does not support writing of flags");
			}
			unsigned long TimestepCount() const
			{
				return _timestepCount;
			}
			unsigned long BeamletCount() const
			{
				switch(_mode)
				{
					case AllBeamletsMode:
						return 1;
					case SingleBeamletMode:
						return 5;
					case BeamletChannelMode:
						return 5;
				}
			}
			unsigned long TimeblockSize() const
			{
				return 2048;
			}
		private:
			RSPReader _reader;
			std::deque<BaselineData*> _baselineBuffer;
			enum Mode _mode;
			unsigned long _timestepCount;
	};

	void RSPImageSetIndex::Previous()
	{
		if(_beamlet > 0)
		{
			--_beamlet;
		} else {
			_beamlet = RSPSet().BeamletCount()-1;
			LargeStepPrevious();
		}
	}
	
	void RSPImageSetIndex::Next()
	{
		++_beamlet;
		if(_beamlet >= RSPSet().BeamletCount())
		{
			_beamlet = 0;
			LargeStepNext();
		}
	}
	
	void RSPImageSetIndex::LargeStepPrevious()
	{
		const unsigned long modulo = (RSPSet().TimestepCount()+RSPSet().TimeblockSize()-1)/RSPSet().TimeblockSize();
		_timeBlock = (_timeBlock + modulo - 1) % modulo;
	}
	
	void RSPImageSetIndex::LargeStepNext()
	{
		++_timeBlock;
		const unsigned long modulo = (RSPSet().TimestepCount()+RSPSet().TimeblockSize()-1)/RSPSet().TimeblockSize();
		if(_timeBlock >= modulo)
		{
			_timeBlock = 0;
			_isValid = false;
		}
	}
	
	RSPImageSet &RSPImageSetIndex::RSPSet() const
	{
		return static_cast<RSPImageSet&>(imageSet());
	}
}

#endif // RSPIMAGESET_H
