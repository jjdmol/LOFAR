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

namespace rfiStrategy {
	
	class RSPImageSetIndex : public ImageSetIndex {
		friend class RSPImageSet;
		
		RSPImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _baselineIndex(0), _isValid(true) { }
		
		virtual void Previous()
		{
		}
		virtual void Next()
		{
		}
		virtual void LargeStepPrevious()
		{
		}
		virtual void LargeStepNext()
		{
		}
		virtual std::string Description() const
		{
		}
		virtual bool IsValid() const
		{
			return _isValid;
		}
		virtual RSPImageSetIndex *Copy() const
		{
			RSPImageSetIndex *index = new RSPImageSetIndex(imageSet());
			index->_baselineIndex = _baselineIndex;
			index->_isValid = _isValid;
			return index;
		}
		private:
			size_t _baselineIndex;
			bool _isValid;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class RSPImageSet : public ImageSet
	{
		public:
			RSPImageSet(const std::string &file) : _reader(file)
			{
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
			virtual void AddReadRequest(ImageSetIndex &)
			{
			}
			virtual void PerformReadRequests()
			{
				_reader.Read();
			}
			virtual BaselineData *GetNextRequested()
			{
				return 0;
			}
			virtual void AddWriteFlagsTask(ImageSetIndex &, std::vector<Mask2DCPtr> &)
			{
			}
			virtual void PerformWriteFlagsTask()
			{
			}
		private:
			RSPReader _reader;
	};

}

#endif // RSPIMAGESET_H
