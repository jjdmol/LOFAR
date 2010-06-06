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
#include <stdexcept>

#include <AOFlagger/rfi/timefrequencystatistics.h>

#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {

	MSImageSet::MSImageSet(std::string location) : _set(location), _reader(0),
		_dataKind(ObservedData),
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
		if(_reader != 0)
			delete _reader;
	}

	void MSImageSet::Initialize()
	{
		std::cout << "Initializing image set..." << std::endl;
		std::cout << "Antenna's: " << _set.AntennaCount() << std::endl;
		_set.GetBaselines(_baselines);
		std::cout << "Unique baselines: " << _baselines.size() << std::endl;
		initReader();
		_reader->PartInfo(_maxScanCounts-_scanCountPartOverlap, _timeScanCount, _partCount);
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

	void MSImageSet::initReader()
	{
		if(_reader == 0 )
			_reader = new BaselineReader(_set);
		_reader->SetDataKind(_dataKind);
		_reader->SetReadFlags(_readFlags);
		_reader->SetReadData(true);
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
		initReader();
		size_t a1 = _baselines[msIndex._baselineIndex].first;
		size_t a2 = _baselines[msIndex._baselineIndex].second;
		size_t
			startIndex = StartIndex(msIndex),
			endIndex = EndIndex(msIndex);
		std::cout << "Loading baseline " << a1 << "x" << a2 << ", t=" << startIndex << "-" << endIndex << std::endl;
		_reader->AddReadRequest(a1, a2, msIndex._band, startIndex, endIndex);
		_reader->PerformReadRequests();
		std::vector<UVW> uvw;
		TimeFrequencyData data = _reader->GetNextResult(uvw);
		return new TimeFrequencyData(data);
	}

	void MSImageSet::LoadFlags(ImageSetIndex &index, TimeFrequencyData &destination)
	{
		/*
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
			switch(destination.Polarisation())
			{
				case DipolePolarisation:
					if(_readStokesI)
						destination.SetGlobalMask(_imager->FlagStokesI());
					else if(_readDipoleAutoPolarisations && _readDipoleCrossPolarisations)
						destination.SetIndividualPolarisationMasks(_imager->FlagXX(), _imager->FlagXY(), _imager->FlagYX(), _imager->FlagYY());
					else
						throw BadUsageException("Incorrect settings for flag reading");
					break;
				case StokesIPolarisation:
					if(_readStokesI)
						destination.SetGlobalMask(_imager->FlagStokesI());
					else
						throw BadUsageException("Incorrect settings for flag reading");
					break;
				case AutoDipolePolarisation:
					if(_readStokesI)
						destination.SetGlobalMask(_imager->FlagStokesI());
					else if(_readDipoleAutoPolarisations)
						destination.SetIndividualPolarisationMasks(_imager->FlagXX(), _imager->FlagYY());
					else
						throw BadUsageException("Incorrect settings for flag reading");
					break;
				case CrossDipolePolarisation:
				case SinglePolarisation:
				case XXPolarisation:
				case XYPolarisation:
				case YXPolarisation:
				case YYPolarisation:
				case StokesQPolarisation:
				case StokesUPolarisation:
				case StokesVPolarisation:
					throw BadUsageException("Loading flags for a time frequency data with uncommon polarisation... Not yet implemented...");
					break;
			}
			_imager->ClearImages();
			//TimeFrequencyStatistics stats(destination);
			//std::cout << "Flags read: " << TimeFrequencyStatistics::FormatRatio(stats.GetFlaggedRatio()) << std::endl;
		}*/
	}

	TimeFrequencyMetaDataCPtr MSImageSet::createMetaData(ImageSetIndex &index, std::vector<UVW> &uvw)
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
		if(_reader != 0)
		{
			metaData->SetUVW(uvw);
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
		AddWriteFlagsTask(index, data);
		_reader->PerformWriteRequests();
	}

	void MSImageSet::AddReadRequest(ImageSetIndex &index)
	{
		BaselineData newRequest(index);
		_baselineData.push_back(newRequest);
	}
	
	void MSImageSet::PerformReadRequests()
	{
		for(std::vector<BaselineData>::iterator i=_baselineData.begin();i!=_baselineData.end();++i)
		{
			MSImageSetIndex &index = static_cast<MSImageSetIndex&>(i->Index());
			_reader->AddReadRequest(GetAntenna1(index), GetAntenna2(index), index._band, StartIndex(index), EndIndex(index));
		}
		
		_reader->PerformReadRequests();
		
		for(std::vector<BaselineData>::iterator i=_baselineData.begin();i!=_baselineData.end();++i)
		{
			if(!i->Data().IsEmpty())
				throw std::runtime_error("ReadRequest() called, but a previous read request was not completely processed by calling GetNextRequested().");
			std::vector<UVW> uvw;
			TimeFrequencyData data = _reader->GetNextResult(uvw);
			i->SetData(data);
			TimeFrequencyMetaDataCPtr metaData = createMetaData(i->Index(), uvw);
			i->SetMetaData(metaData);
		}
	}
	
	BaselineData *MSImageSet::GetNextRequested()
	{
		BaselineData top = _baselineData.front();
		_baselineData.erase(_baselineData.begin());
		if(top.Data().IsEmpty())
			throw std::runtime_error("Calling GetNextRequested(), but requests were not read with LoadRequests.");
		return new BaselineData(top);
	}
	
	void MSImageSet::AddWriteFlagsTask(ImageSetIndex &index, TimeFrequencyData &data)
	{
		MSImageSetIndex &msIndex = static_cast<MSImageSetIndex&>(index);
		initReader();
		size_t a1 = _baselines[msIndex._baselineIndex].first;
		size_t a2 = _baselines[msIndex._baselineIndex].second;
		size_t b = msIndex._band;
		size_t
			startIndex = StartIndex(msIndex),
			endIndex = EndIndex(msIndex);

		Mask2DCPtr xx, xy, yx, yy;

		xx = data.GetMask(XXPolarisation);
		yy = data.GetMask(YYPolarisation);

		if(data.Polarisation() == AutoDipolePolarisation)
		{
			Mask2DPtr joined = Mask2D::CreateCopy(xx);
			joined->Join(yy);
			xy = joined;
			yx = joined;
		} else
		{
			xy = data.GetMask(XYPolarisation);
			yx = data.GetMask(YXPolarisation);
		}

		TimeFrequencyStatistics stats(data);
		std::cout << "Adding write flags task, flags: "
			<< TimeFrequencyStatistics::FormatRatio(stats.GetFlaggedRatio())
			<< " (" << xx->GetCount<true>() << " / "
			<< xy->GetCount<true>() << " / "
			<< yx->GetCount<true>() << " / "
			<< yy->GetCount<true>() << ")"
			<< " for baseline index " << a1 << "x" << a2 << " (sb " << b << "),t=" << startIndex << "-" << endIndex << std::endl;
			std::vector<Mask2DCPtr> dataVector;
			dataVector.push_back(xx);
			dataVector.push_back(xy);
			dataVector.push_back(yx);
			dataVector.push_back(yy);
		_reader->AddWriteTask(dataVector, a1, a2, b, startIndex, endIndex, LeftBorder(msIndex), RightBorder(msIndex));
	}
	
	void MSImageSet::PerformWriteFlagsTask()
	{
		_reader->PerformWriteRequests();
	}

}
