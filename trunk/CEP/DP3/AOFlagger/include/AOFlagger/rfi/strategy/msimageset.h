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
#ifndef MSIMAGESET_H
#define MSIMAGESET_H

#include <set>
#include <string>

#include "imageset.h"

#include "../../msio/antennainfo.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"
#include "../../msio/timefrequencyimager.h"
#include "../../msio/measurementset.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

namespace rfiStrategy {

	class MSImageSetIndex : public ImageSetIndex {
		friend class MSImageSet;
		
		MSImageSetIndex(class rfiStrategy::ImageSet &set) : ImageSetIndex(set), _baselineIndex(0), _band(0), _field(0), _partIndex(0), _isValid(true) { }
		
		virtual void Previous();
		virtual void Next();
		virtual void LargeStepPrevious();
		virtual void LargeStepNext();
		virtual std::string Description() const;
		virtual bool IsValid() const { return _isValid; }
		virtual MSImageSetIndex *Copy() const
		{
			MSImageSetIndex *index = new MSImageSetIndex(imageSet());
			index->_baselineIndex = _baselineIndex;
			index->_band = _band;
			index->_field = _field;
			index->_partIndex = _partIndex;
			index->_isValid = _isValid;
			return index;
		}
		private:
			size_t _baselineIndex, _band, _field, _partIndex;
			bool _isValid;
	};
	
	class MSImageSet : public ImageSet {
		public:
			MSImageSet(std::string location);
			~MSImageSet();

			virtual MSImageSet *Copy()
			{
				MSImageSet *newSet = new MSImageSet(_set.Location());
				newSet->_imager = 0;
				newSet->_dataKind = _dataKind;
				newSet->_readDipoleAutoPolarisations = _readDipoleAutoPolarisations;
				newSet->_readDipoleCrossPolarisations = _readDipoleCrossPolarisations;
				newSet->_readStokesI = _readStokesI;
				newSet->_baselines = _baselines;
				newSet->_bandCount = _bandCount;
				newSet->_maxScanCounts = _maxScanCounts;
				newSet->_partCount = _partCount;
				newSet->_timeScanCount = _timeScanCount;
				newSet->_scanCountPartOverlap = _scanCountPartOverlap;
				return newSet;
			}
	
			virtual std::string Name() { return _set.Location(); }
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index);
			//virtual BandInfo LoadBandInfo(ImageSetIndex &index);
			//virtual AntennaInfo LoadAntenna1Info(ImageSetIndex &index);
			//virtual AntennaInfo LoadAntenna2Info(ImageSetIndex &index);
			//virtual FieldInfo LoadFieldInfo(ImageSetIndex &index);
			virtual void Initialize();
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &index);
	
			virtual size_t GetAntenna1(ImageSetIndex &index) {
				return _baselines[static_cast<MSImageSetIndex&>(index)._baselineIndex].first;
			}
			virtual size_t GetAntenna2(ImageSetIndex &index) {
				return _baselines[static_cast<MSImageSetIndex&>(index)._baselineIndex].second;
			}
			virtual size_t GetPart(ImageSetIndex &index) {
				return static_cast<MSImageSetIndex&>(index)._partIndex;
			}

	
			virtual ImageSetIndex *StartIndex() { return new MSImageSetIndex(*this); }

			MSImageSetIndex *Index(size_t a1, size_t a2, size_t b)
			{
				MSImageSetIndex *index = new MSImageSetIndex(*this);
				index->_baselineIndex = FindBaselineIndex(a1, a2);
				index->_band = b;
				return index;
			}
			
			void SetDataKind(DataKind dataKind) { _dataKind = dataKind; }
			void SetReadAllPolarisations() throw()
			{
				_readDipoleAutoPolarisations = true;
				_readDipoleCrossPolarisations = true;
				_readStokesI = false;
			}
			void SetReadDipoleAutoPolarisations() throw()
			{
				_readDipoleAutoPolarisations = true;
				_readDipoleCrossPolarisations = false;
				_readStokesI = false;
			}
			void SetReadStokesI() throw()
			{
				_readStokesI = true;
				_readDipoleAutoPolarisations = false;
				_readDipoleCrossPolarisations = false;
			}
			void SetMaxScanCounts(size_t maxScanCounts) throw()
			{
				_maxScanCounts = maxScanCounts;
			}

			size_t AntennaCount() { return _set.AntennaCount(); }
			class ::AntennaInfo GetAntennaInfo(unsigned antennaIndex) { return _set.GetAntennaInfo(antennaIndex); }
			class ::BandInfo GetBandInfo(unsigned bandIndex) { return _set.GetBandInfo(bandIndex); }
			const std::set<double> &GetObservationTimesSet() { return _set.GetObservationTimesSet(); }
			std::vector<double> *CreateObservationTimesVector() { return _set.CreateObservationTimesVector(); }
			const std::vector<std::pair<size_t,size_t> > &Baselines() const { return _baselines; }
			size_t BandCount() const { return _bandCount; }
			virtual void WriteFlags(ImageSetIndex &index, TimeFrequencyData &data);
			size_t PartCount() const { return _partCount; }
			void SetReadFlags(bool readFlags) { _readFlags = readFlags; }
			void SetReadFlagsOnly() throw()
			{
				_readStokesI = false;
				_readDipoleAutoPolarisations = false;
				_readDipoleCrossPolarisations = false;
				_readFlags = true;
			}
			virtual void LoadFlags(ImageSetIndex &index, TimeFrequencyData &destination);
		private:
			size_t StartIndex(MSImageSetIndex &index);
			size_t EndIndex(MSImageSetIndex &index);
			size_t LeftBorder(MSImageSetIndex &index);
			size_t RightBorder(MSImageSetIndex &index);
			bool InitImager(MSImageSetIndex &index);
			size_t FindBaselineIndex(size_t a1, size_t a2);

			MeasurementSet _set;
			TimeFrequencyImager *_imager;
			DataKind _dataKind;
			bool _readDipoleAutoPolarisations, _readDipoleCrossPolarisations, _readStokesI;
			std::vector<std::pair<size_t,size_t> > _baselines;
			size_t _bandCount;
			size_t _maxScanCounts;
			size_t _partCount, _timeScanCount;
			size_t _scanCountPartOverlap;
			bool _readFlags;
	};

}
	
#endif
