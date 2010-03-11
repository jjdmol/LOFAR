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
#include <AOFlagger/rfi/strategy/fitsimageset.h>

#include <iostream>
#include <sstream>

#include <AOFlagger/msio/date.h>
#include <AOFlagger/msio/fitsfile.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencydata.h>

namespace rfiStrategy {
	
	FitsImageSet::FitsImageSet(const std::string &file)
	: ImageSet(), _data(0), _currentBaselineIndex(0), _frequencyOffset(0.0)
	{
		_file = new FitsFile(file);
		_file->Open(FitsFile::ReadOnlyMode);
	}
	
	FitsImageSet::~FitsImageSet()
	{
		delete _file;
		if(_data != 0)
			delete _data;
	}
	
	FitsImageSet *FitsImageSet::Copy()
	{
		FitsImageSet *newSet = new FitsImageSet(_file->Filename());
		newSet->_data = new TimeFrequencyData(*_data);
		newSet->_baselines = _baselines;
		newSet->_bandCount = _bandCount;
		newSet->_observationTimes = _observationTimes;
		newSet->_antennaInfos = _antennaInfos;
		return newSet;
	}

	void FitsImageSet::Initialize()
	{
		std::cout << "Keyword count: " << _file->GetKeywordCount() << std::endl;
		std::cout << "This file has " << _file->GetGroupCount() << " groups with " << _file->GetParameterCount() << " parameters." << std::endl;
		std::cout << "Group size: " << _file->GetGroupSize() << std::endl;
		_file->MoveToHDU(1);
		if(!_file->HasGroups() || _file->GetCurrentHDUType() != FitsFile::ImageHDUType)
			throw FitsIOException("Primary table is not a grouped image");
		long double *parameters = new long double[_file->GetParameterCount()];
		int baselineIndex = _file->GetGroupParameterIndex("BASELINE");
		size_t groupCount = _file->GetGroupCount();
		std::set<std::pair<size_t,size_t> > baselineSet;
		for(size_t g=0;g<groupCount;++g)
		{
			_file->ReadGroupParameters(g, parameters);
			int a1 = ((int) parameters[baselineIndex]) & 255;
			int a2 = ((int) parameters[baselineIndex] >> 8) & 255;
			baselineSet.insert(std::pair<size_t,size_t>(a1,a2));
		}
		delete[] parameters;
		std::cout << "Baselines in file: " << baselineSet.size() << std::endl;
		for(std::set<std::pair<size_t,size_t> >::const_iterator i=baselineSet.begin();i!=baselineSet.end();++i)
			_baselines.push_back(*i);
		_bandCount = _file->GetCurrentImageSize(5);
	}

	TimeFrequencyData *FitsImageSet::LoadData(ImageSetIndex &index)
	{
		FitsImageSetIndex &fitsIndex = static_cast<FitsImageSetIndex&>(index);
		if(_data != 0 && _currentBaselineIndex == fitsIndex._baselineIndex && _currentBandIndex == fitsIndex._band)
			return new TimeFrequencyData(*_data);

		if(_data != 0)
		{
			delete _data;
			_data = 0;
		}
		_frequencyOffset = 0.0;

		_file->MoveToHDU(1);
		ReadPrimaryTable(fitsIndex._baselineIndex, fitsIndex._band, 0);
		for(int hduIndex=2;hduIndex <= _file->GetHDUCount();hduIndex++)
		{
			_file->MoveToHDU(hduIndex);
			switch(_file->GetCurrentHDUType())
			{
				case FitsFile::BinaryTableHDUType:
					std::cout << "Binary table found." << std::endl;
					ReadTable();
					break;
				case FitsFile::ASCIITableHDUType:
					std::cout << "ASCII table found." << std::endl;
					ReadTable();
					break;
				case FitsFile::ImageHDUType:
					std::cout << "Image found." << std::endl;
					break;
			}
		}
		_currentBaselineIndex = fitsIndex._baselineIndex;
		_currentBandIndex = fitsIndex._band;

		return new TimeFrequencyData(*_data);
	}

	void FitsImageSet::ReadPrimaryTable(size_t baselineIndex, int band, int stokes)
	{
		if(!_file->HasGroups() || _file->GetCurrentHDUType() != FitsFile::ImageHDUType)
			throw FitsIOException("Primary table is not a grouped image");

		_observationTimes.clear();
		_uvw.clear();

		int keywordCount = _file->GetKeywordCount();
		for(int i=1;i<=keywordCount;++i)
			std::cout << "Keyword " << i << ": " << _file->GetKeyword(i) << "=" << _file->GetKeywordValue(i) << " ("  << _file->GetKeywordComment(i) << ")" << std::endl;

		long double *parameters = new long double[_file->GetParameterCount()];
		int baseline = _baselines[baselineIndex].first + (_baselines[baselineIndex].second<<8);
		int baselineColumn = _file->GetGroupParameterIndex("BASELINE");
		size_t
			complexCount = _file->GetCurrentImageSize(2),
			stokesStep = complexCount,
			stokesCount = _file->GetCurrentImageSize(3),
			frequencyStep = stokesCount*complexCount,
			frequencyCount = _file->GetCurrentImageSize(4),
			bandStep = frequencyStep*frequencyCount;
		std::vector<long double> valuesR[frequencyCount];
		std::vector<long double> valuesI[frequencyCount];
		long double *data = new long double[_file->GetImageSize()];
		size_t groupCount = _file->GetGroupCount();
		int date1Index = _file->GetGroupParameterIndex("DATE");
		int date2Index = _file->GetGroupParameterIndex("DATE", 2);
		int uuIndex, vvIndex, wwIndex;
		if(_file->HasGroupParameter("UU"))
		{
			uuIndex = _file->GetGroupParameterIndex("UU");
			vvIndex = _file->GetGroupParameterIndex("VV");
			wwIndex = _file->GetGroupParameterIndex("WW");
		} else {
			uuIndex = _file->GetGroupParameterIndex("UU---SIN");
			vvIndex = _file->GetGroupParameterIndex("VV---SIN");
			wwIndex = _file->GetGroupParameterIndex("WW---SIN");
		}
		size_t match = 0;
		double frequencyFactor = 1.0;
		if(_frequencyOffset != 0.0)
			frequencyFactor = _frequencyOffset;
		for(size_t g=0;g<groupCount;++g)
		{
			_file->ReadGroupParameters(g, parameters);
			if(parameters[baselineColumn] == baseline)
			{
				double date = parameters[date1Index] + parameters[date2Index];
				UVW uvw;
				uvw.u = parameters[uuIndex] * frequencyFactor;
				uvw.v = parameters[vvIndex] * frequencyFactor;
				uvw.w = parameters[wwIndex] * frequencyFactor;

				_file->ReadGroupData(g, data);
				for(size_t f=0;f<frequencyCount;++f)
				{
					size_t index = stokes*stokesStep + frequencyStep*f + bandStep*band;
					long double r = data[index];
					long double i = data[index + 1];
					valuesR[f].push_back(r);
					valuesI[f].push_back(i);
				}
				_observationTimes.push_back(Date::JDToAipsMJD(date));
				_uvw.push_back(uvw);
				++match;
			}
		}
		std::cout << match << " rows in table matched baseline." << std::endl;
		delete[] data;
		delete[] parameters;

		std::cout << "Image is " << valuesR[0].size() << " x " << frequencyCount << std::endl;
		if(valuesR[0].size() == 0)
			throw BadUsageException("Baseline not found!");
		Image2DPtr
			real = Image2D::CreateEmptyImagePtr(valuesR[0].size(), frequencyCount),
			imaginary = Image2D::CreateEmptyImagePtr(valuesR[0].size(), frequencyCount);
		for(size_t i=0;i<valuesR[0].size();++i)
		{
			for(size_t f=0;f<frequencyCount;++f)
			{
				real->SetValue(i, f, valuesR[f][i]);
				imaginary->SetValue(i, f, valuesI[f][i]);
			}
		}
		_data = new TimeFrequencyData(TimeFrequencyData::StokesI, real, imaginary);
	}

	void FitsImageSet::ReadTable()
	{
		std::cout << "Row count: " << _file->GetRowCount() << std::endl;
		std::cout << "Column count: " << _file->GetColumnCount() << std::endl;
		for(int i= 1;i <= _file->GetColumnCount(); ++i)
		{
			std::cout << "Column type " << i << ": " << _file->GetColumnType(i) << std::endl;
		}
		std::string extName = _file->GetKeywordValue("EXTNAME");
		for(int i=1;i<=_file->GetKeywordCount();++i)
			std::cout << "Keyword " << i << ": " << _file->GetKeyword(i) << "=" << _file->GetKeywordValue(i) << " ("  << _file->GetKeywordComment(i) << ")" << std::endl;
		if(extName == "AIPS AN")
			ReadAntennaTable();
		else if(extName == "AIPS FQ")
			ReadFrequencyTable();
		else if(extName == "AIPS CL")
			ReadCalibrationTable();
	}
	
	void FitsImageSet::ReadAntennaTable()
	{
		std::cout << "Found antenna table" << std::endl;
		_frequencyOffset = _file->GetDoubleKeywordValue("FREQ");
		for(std::vector<BandInfo>::iterator i=_bandInfos.begin();i!=_bandInfos.end();++i)
		{
			for(std::vector<ChannelInfo>::iterator j=i->channels.begin();j!=i->channels.end();++j) {
				j->frequencyHz += _frequencyOffset;
			}
		}
		for(std::vector<UVW>::iterator i=_uvw.begin();i!=_uvw.end();++i)
		{
			i->u = i->u * _frequencyOffset;
			i->v = i->v * _frequencyOffset;
			i->w = i->w * _frequencyOffset;
		}
		_antennaInfos.clear();
		for(int i=1;i<=_file->GetRowCount();++i)
		{
			AntennaInfo info;
			char name[9];
			long double pos[3];
			_file->ReadTableCell(i, 1, name);
			_file->ReadTableCell(i, 2, pos, 3);
			info.name = name;
			info.position.x = pos[0];
			info.position.y = pos[1];
			info.position.z = pos[2];
			_antennaInfos.push_back(info);
			//std::cout << info.position.x << " " << info.position.y << " " << info.position.z << std::endl;
		}
	}

	void FitsImageSet::ReadFrequencyTable()
	{
		std::cout << "Found frequency table" << std::endl;
		const size_t numberIfs = _file->GetIntKeywordValue("NO_IF");
		std::cout << "Number of ifs: " << numberIfs << std::endl;
		_bandInfos.clear();
		BandInfo bandInfo;
		for(int i=1;i<=_file->GetRowCount();++i)
		{
			long double freqSel, ifFreq[numberIfs], chWidth[numberIfs], totalBandwidth[numberIfs], sideband[numberIfs];
			_file->ReadTableCell(i, 1, &freqSel, 1);
			_file->ReadTableCell(i, 2, ifFreq, numberIfs);
			_file->ReadTableCell(i, 3, chWidth, numberIfs);
			_file->ReadTableCell(i, 4, totalBandwidth, numberIfs);
			_file->ReadTableCell(i, 5, sideband, numberIfs);
			for(size_t b=0;b<numberIfs;++b)
			{
				for(size_t channel=0;channel<_data->ImageHeight();++channel)
				{
					ChannelInfo channelInfo;
					channelInfo.channelWidthHz = chWidth[b];
					channelInfo.effectiveBandWidthHz = chWidth[b];
					channelInfo.frequencyHz = _frequencyOffset + ifFreq[b] + (chWidth[b] * channel);
					channelInfo.frequencyIndex = channel;
					channelInfo.resolutionHz = chWidth[b];
					bandInfo.channels.push_back(channelInfo);
				}

				bandInfo.windowIndex = 0;
				_bandInfos.push_back(bandInfo);
			}
		}
	}

	void FitsImageSet::ReadCalibrationTable()
	{
		std::cout << "Found calibration table with " << _file->GetRowCount() << " rows." << std::endl;
		/*for(int i=1;i<=_file->GetRowCount();++i)
		{
			long double weight;
			_file->ReadTableCell(i, 21, &weight, 1);
			std::cout << i << "," << weight << std::endl;
		}*/
	}

	void FitsImageSetIndex::Previous()
	{
		if(_baselineIndex > 0)
			--_baselineIndex;
		else {
			_baselineIndex = static_cast<class FitsImageSet&>(imageSet()).Baselines().size() - 1;
			LargeStepPrevious();
		}
	}
	
	void FitsImageSetIndex::Next()
	{
		++_baselineIndex;
		if( _baselineIndex >= static_cast<class FitsImageSet&>(imageSet()).Baselines().size() )
		{
			_baselineIndex = 0;
			LargeStepNext();
		}
	}

	void FitsImageSetIndex::LargeStepPrevious()
	{
		if(_band > 0)
			--_band;
		else {
			_band = static_cast<class FitsImageSet&>(imageSet()).BandCount() - 1;
			_isValid = false;
		}
	}
	
	void FitsImageSetIndex::LargeStepNext()
	{
		++_band;
		if(_band >= static_cast<class FitsImageSet&>(imageSet()).BandCount())
		{
			_band = 0;
			_isValid = false;
		}
	}

	std::string FitsImageSetIndex::Description() const {
		int a1 = static_cast<class FitsImageSet&>(imageSet()).Baselines()[_baselineIndex].first;
		int a2 = static_cast<class FitsImageSet&>(imageSet()).Baselines()[_baselineIndex].second;
		AntennaInfo info1 = static_cast<class FitsImageSet&>(imageSet()).GetAntennaInfo(a1);
		AntennaInfo info2 = static_cast<class FitsImageSet&>(imageSet()).GetAntennaInfo(a2);
		std::stringstream s;
		s << "fits correlation " << info1.name << " x " << info2.name << ", band " << _band;
		return s.str();
	}

	TimeFrequencyMetaDataCPtr FitsImageSet::LoadMetaData(ImageSetIndex &index)
	{
		FitsImageSetIndex &fitsIndex = static_cast<FitsImageSetIndex&>(index);
		LoadData(fitsIndex);
		TimeFrequencyMetaData *metaData = new TimeFrequencyMetaData();
		metaData->SetObservationTimes(_observationTimes);
		metaData->SetUVW(_uvw);
		metaData->SetBand(_bandInfos[fitsIndex._band]);
		std::cout << "Loaded metadata for: " << Date::AipsMJDToString(_observationTimes[0]) << ", band " << fitsIndex._band << " (" << Frequency::ToString(_bandInfos[fitsIndex._band].channels[0].frequencyHz) << " - " << Frequency::ToString(_bandInfos[fitsIndex._band].channels.rbegin()->frequencyHz) << ")" << std::endl;
		return TimeFrequencyMetaDataCPtr(metaData);
	}
}
