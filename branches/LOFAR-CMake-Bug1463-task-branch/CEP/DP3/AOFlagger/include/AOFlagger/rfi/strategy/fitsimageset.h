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
#ifndef FITSIMAGESET_H
#define FITSIMAGESET_H

#include <vector>
#include <set>

#include "imageset.h"

#include "../../baseexception.h"

#include "../../msio/antennainfo.h"
#include "../../msio/types.h"

namespace rfiStrategy {
	
	class FitsImageSetIndex : public ImageSetIndex {
		friend class FitsImageSet;
		
		FitsImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _baselineIndex(0), _band(0), _field(0), _isValid(true) { }
		
		virtual void Previous();
		virtual void Next();
		virtual void LargeStepPrevious();
		virtual void LargeStepNext();
		virtual std::string Description() const;
		virtual bool IsValid() const throw() { return _isValid; }
		virtual FitsImageSetIndex *Copy() const
		{
			FitsImageSetIndex *index = new FitsImageSetIndex(imageSet());
			index->_baselineIndex = _baselineIndex;
			index->_band = _band;
			index->_field = _field;
			index->_isValid = _isValid;
			return index;
		}
		private:
			size_t _baselineIndex, _band, _field;
			bool _isValid;
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class FitsImageSet : public ImageSet
	{
		public:
			FitsImageSet(const std::string &file);
			~FitsImageSet();
			virtual void Initialize();

			virtual FitsImageSet *Copy();

			virtual ImageSetIndex *StartIndex()
			{
				return new FitsImageSetIndex(*this);
			}
			virtual std::string Name()
			{
				return "Fits file";
			}
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index);
			virtual void LoadFlags(ImageSetIndex &, TimeFrequencyData &)
			{
				throw BadUsageException("Loading flags is not supported for fits files");
			}
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &index);
			virtual size_t GetPart(ImageSetIndex &) {
				return 0;
			}
			const std::vector<std::pair<size_t,size_t> > &Baselines() const throw() { return _baselines; }
			size_t BandCount() { return _bandCount; }
			class AntennaInfo GetAntennaInfo(unsigned antennaIndex) { return _antennaInfos[antennaIndex]; }
			virtual void WriteFlags(ImageSetIndex &, TimeFrequencyData &)
			{
				throw BadUsageException("Fits format is not supported for writing flags yet");
			}
			virtual size_t GetAntenna1(ImageSetIndex &index) {
				return _baselines[static_cast<FitsImageSetIndex&>(index)._baselineIndex].first;
			}
			virtual size_t GetAntenna2(ImageSetIndex &index) {
				return _baselines[static_cast<FitsImageSetIndex&>(index)._baselineIndex].second;
			}
		private:
			void ReadTable();
			void ReadAntennaTable();
			void ReadFrequencyTable();
			void ReadCalibrationTable();
			void ReadPrimaryTable(size_t baselineIndex, int band, int stokes);
			
			class FitsFile *_file;
			TimeFrequencyData *_data;
			std::vector<std::pair<size_t,size_t> > _baselines;
			size_t _bandCount;
			std::vector<double> _observationTimes;
			std::vector<UVW> _uvw;
			std::vector<AntennaInfo> _antennaInfos;
			std::vector<BandInfo> _bandInfos;
			size_t _currentBaselineIndex, _currentBandIndex;
			double _frequencyOffset;
	};

}

#endif
