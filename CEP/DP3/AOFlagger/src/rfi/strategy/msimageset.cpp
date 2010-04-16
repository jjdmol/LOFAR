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

#include <iostream>
#include <sstream>

#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {

	MSImageSet::MSImageSet(std::string location) : _set(location), _imager(0),
		_imageKind(TimeFrequencyImager::Corrected),
		_readDipoleAutoPolarisations(true),
		_readDipoleCrossPolarisations(true),
		_readStokesI(false),
		_maxScanCounts(0),
		_scanCountPartOverlap(100),
		_readFlags(true)
	{
	}
	
	MSImageSet::~MSImageSet()
	{
		if(_imager != 0)
			delete _imager;
	}

	void MSImageSet::Initialize()
	{
		std::cout << "Initializing image set..." << std::endl;
		std::cout << "Antenna's: " << _set.AntennaCount() << std::endl;
		_set.GetBaselines(_baselines);
		std::cout << "Unique baselines: " << _baselines.size() << std::endl;
		TimeFrequencyImager::PartInfo(_set.Location(), _maxScanCounts-_scanCountPartOverlap, _timeScanCount, _partCount);
		std::cout << "Unique time stamps: " << _timeScanCount << std::endl;
		_bandCount = _set.MaxSpectralBandIndex()+1;
		std::cout << "Bands: " << _bandCount << std::endl;
		std::cout << "Number of parts: " << _partCount << std::endl;
	}
	
	void MSImageSetIndex::Previous()
	{
		if(_partIndex > 0)
			--_partIndex;
		else {
			_partIndex = static_cast<class MSImageSet&>(imageSet()).PartCount() - 1;

			if(_baselineIndex > 0)
				--_baselineIndex;
			else {
				_baselineIndex = static_cast<class MSImageSet&>(imageSet()).Baselines().size() - 1;
				LargeStepPrevious();
			}
		}
	}
	
	void MSImageSetIndex::Next()
	{
		++_partIndex;
		if(_partIndex >= static_cast<class MSImageSet&>(imageSet()).PartCount() )
		{
			_partIndex = 0;

			++_baselineIndex;
			if( _baselineIndex >= static_cast<class MSImageSet&>(imageSet()).Baselines().size() )
			{
				_baselineIndex = 0;
				LargeStepNext();
			}
		}
	}
	
	void MSImageSetIndex::LargeStepPrevious()
	{
		if(_band > 0)
			--_band;
		else {
			_band = static_cast<class MSImageSet&>(imageSet()).BandCount() - 1;
			_isValid = false;
		}
	}
	
	void MSImageSetIndex::LargeStepNext()
	{
		++_band;
		if(_band >= static_cast<class MSImageSet&>(imageSet()).BandCount())
		{
			_band = 0;
			_isValid = false;
		}
	}

	bool MSImageSet::InitImager(MSImageSetIndex &)
	{
		if(_imager == 0 )
			_imager = new TimeFrequencyImager(_set);
		_imager->SetImageKind(_imageKind);
		_imager->SetReadFlags(_readFlags);
		_imager->SetReadData(true);
		_imager->SetReadXX(_readDipoleAutoPolarisations);
		_imager->SetReadYY(_readDipoleAutoPolarisations);
		_imager->SetReadXY(_readDipoleCrossPolarisations);
		_imager->SetReadYX(_readDipoleCrossPolarisations);
		_imager->SetReadStokesI(_readStokesI);
		return true;
	}

	size_t MSImageSet::StartIndex(MSImageSetIndex &index)
	{
		size_t startIndex =
			(_timeScanCount * index._partIndex) / _partCount - LeftBorder(index);
		return startIndex;
	}

	size_t MSImageSet::EndIndex(MSImageSetIndex &index)
	{
		size_t endIndex =
			(_timeScanCount * (index._partIndex+1)) / _partCount + RightBorder(index);
		return endIndex;
	}

	size_t MSImageSet::LeftBorder(MSImageSetIndex &index)
	{
		if(index._partIndex > 0)
			return _scanCountPartOverlap/2;
		else
			return 0;
	}

	size_t MSImageSet::RightBorder(MSImageSetIndex &index)
	{
		if(index._partIndex + 1 < _partCount)
			return _scanCountPartOverlap/2 + _scanCountPartOverlap%2;
		else
			return 0;
	}

	class TimeFrequencyData *MSImageSet::LoadData(ImageSetIndex &index)
	{
		MSImageSetIndex &msIndex = static_cast<MSImageSetIndex&>(index);
		if(InitImager(msIndex))
		{
			size_t a1 = _baselines[msIndex._baselineIndex].first;
			size_t a2 = _baselines[msIndex._baselineIndex].second;
			size_t
				startIndex = StartIndex(msIndex),
				endIndex = EndIndex(msIndex);
			std::cout << "Loading baseline " << a1 << "x" << a2 << ", t=" << startIndex << "-" << endIndex << std::endl;
			_imager->Image(a1, a2, msIndex._band, startIndex, endIndex);
			std::cout << "Done loading baseline " << a1 << "x" << a2 << ", t=" << startIndex << "-" << endIndex << std::endl;
			class TimeFrequencyData *data = new ::TimeFrequencyData(_imager->GetData());
			return data;
		} else {
			return 0;
		}
	}

	void MSImageSet::LoadFlags(ImageSetIndex &index, TimeFrequencyData &destination)
	{
		MSImageSetIndex &msIndex = static_cast<MSImageSetIndex&>(index);
		if(InitImager(msIndex))
		{
			size_t a1 = _baselines[msIndex._baselineIndex].first;
			size_t a2 = _baselines[msIndex._baselineIndex].second;
			size_t
				startIndex = StartIndex(msIndex),
				endIndex = EndIndex(msIndex);
			_imager->SetReadData(false);
			_imager->SetReadFlags(true);
			_imager->Image(a1, a2, msIndex._band, startIndex, endIndex);
			switch(destination.PolarisationType())
			{
				case TimeFrequencyData::DipolePolarisation:
					if(_readStokesI)
						destination.SetGlobalMask(_imager->FlagStokesI());
					else if(_readDipoleAutoPolarisations && _readDipoleCrossPolarisations)
						destination.SetIndividualPolarisationMasks(_imager->FlagXX(), _imager->FlagXY(), _imager->FlagYX(), _imager->FlagYY());
					else
						throw BadUsageException("Loading flagging for a time frequency data with uncommon polarisation... Not yet implemented...");
					break;
				case TimeFrequencyData::StokesI:
					if(_readStokesI)
						destination.SetGlobalMask(_imager->FlagStokesI());
					else
						throw BadUsageException("Incorrect settings for flag reading");
					break;
				case TimeFrequencyData::AutoDipolePolarisation:
				case TimeFrequencyData::CrossDipolePolarisation:
				case TimeFrequencyData::SinglePolarisation:
				case TimeFrequencyData::XX:
				case TimeFrequencyData::XY:
				case TimeFrequencyData::YX:
				case TimeFrequencyData::YY:
					throw BadUsageException("Loading flagging for a time frequency data with uncommon polarisation... Not yet implemented...");
					break;
			}
		}
	}

	TimeFrequencyMetaDataCPtr MSImageSet::LoadMetaData(ImageSetIndex &index)
	{
		MSImageSetIndex &msIndex = static_cast<MSImageSetIndex&>(index);
		TimeFrequencyMetaData *metaData = new TimeFrequencyMetaData();
		metaData->SetAntenna1(_set.GetAntennaInfo(GetAntenna1(msIndex)));
		metaData->SetAntenna2(_set.GetAntennaInfo(GetAntenna2(msIndex)));
		metaData->SetBand(_set.GetBandInfo(msIndex._band));
		metaData->SetField(_set.GetFieldInfo(msIndex._field));
		std::vector<double> *times = _set.CreateObservationTimesVector();
		metaData->SetObservationTimes(*times);
		delete times;
		if(_imager != 0)
		{
			metaData->SetUVW(_imager->UVW());
		}
		return TimeFrequencyMetaDataCPtr(metaData);
	}

	std::string MSImageSetIndex::Description() const
	{
		std::stringstream sstream;
		size_t
			antenna1 = static_cast<class MSImageSet&>(imageSet()).Baselines()[_baselineIndex].first,
			antenna2 = static_cast<class MSImageSet&>(imageSet()).Baselines()[_baselineIndex].second;
		AntennaInfo info1 = static_cast<class MSImageSet&>(imageSet()).GetAntennaInfo(antenna1);
		AntennaInfo info2 = static_cast<class MSImageSet&>(imageSet()).GetAntennaInfo(antenna2);
		BandInfo bandInfo = static_cast<class MSImageSet&>(imageSet()).GetBandInfo(_band);
		double bandStart = round(bandInfo.channels.front().frequencyHz/100000.0)/10.0;
		double bandEnd = round(bandInfo.channels.back().frequencyHz/100000.0)/10.0;
		sstream
			<< info1.station << ' ' << info1.name << " x " << info2.station << ' ' << info2.name
			<< ", spect window " << _band << " (" << bandStart
			<< "MHz -" << bandEnd << "MHz)";
		return sstream.str();
	}

	size_t MSImageSet::FindBaselineIndex(size_t a1, size_t a2)
	{
		size_t index = 0;
		for(std::vector<std::pair<size_t,size_t> >::const_iterator i=_baselines.begin();
			i != _baselines.end() ; ++i)
		{
			if((i->first == a1 && i->second == a2) || (i->first == a2 && i->second == a1))
			{
				return index;
			}
			++index;
		}
		throw BadUsageException("Baseline not found");
	}

	void MSImageSet::WriteFlags(ImageSetIndex &index, TimeFrequencyData &data)
	{
		MSImageSetIndex &msIndex = static_cast<MSImageSetIndex&>(index);
		if(InitImager(msIndex))
		{
			size_t a1 = _baselines[msIndex._baselineIndex].first;
			size_t a2 = _baselines[msIndex._baselineIndex].second;
			size_t b = msIndex._band;
			size_t
				startIndex = StartIndex(msIndex),
				endIndex = EndIndex(msIndex);

			Mask2DCPtr xx, xy, yx, yy;

			xx = data.GetMask(TimeFrequencyData::XX);
			yy = data.GetMask(TimeFrequencyData::YY);

			if(data.PolarisationType() == TimeFrequencyData::AutoDipolePolarisation)
			{
				Mask2DPtr joined = Mask2D::CreateCopy(xx);
				joined->Join(yy);
				xy = joined;
				yx = joined;
			} else
			{
				xy = data.GetMask(TimeFrequencyData::XY);
				yx = data.GetMask(TimeFrequencyData::YX);
			}

			std::cout << "Writing flags: "
				<< xx->GetCount<true>() << " / "
				<< xy->GetCount<true>() << " / "
				<< yx->GetCount<true>() << " / "
				<< yy->GetCount<true>()
				<< " for baseline index " << a1 << "x" << a2 << " (sb " << b << "),t=" << startIndex << "-" << endIndex << std::endl;
			_imager->WriteNewFlagsPart(xx, xy, yx, yy, a1, a2, b, startIndex, endIndex, LeftBorder(msIndex), RightBorder(msIndex));
			std::cout << "Finished writing flags." << std::endl;
		} else {
			throw IOException("Could not initialize imager");
		}
	}
}
