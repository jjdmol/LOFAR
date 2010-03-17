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

#ifndef RFI_RFISTRATEGY_H
#define RFI_RFISTRATEGY_H 

#include "../../msio/types.h"
#include "../../msio/timefrequencydata.h"
#include "../../msio/timefrequencymetadata.h"

#include "../../types.h"

#include "../types.h"

#include "types.h"

class UVImager;

namespace rfiStrategy {
	class	ArtifactSet
	{
		public:
			ArtifactSet(boost::mutex *ioMutex) : _metaData(), _sensitivity(1.0L), _imageSet(0),
			_imageSetIndex(0), _imager(0),
			_ioMutex(ioMutex),
			_antennaFlagCountPlot(0), _frequencyFlagCountPlot(0),
			_frequencyPowerPlot(0), _timeFlagCountPlot(0)
			{
			}

			ArtifactSet(const ArtifactSet &source)
				: _originalData(source._originalData), _contaminatedData(source._contaminatedData),
				_revisedData(source._revisedData), _metaData(source._metaData), _sensitivity(source._sensitivity),
				_imageSet(source._imageSet), _imageSetIndex(source._imageSetIndex),
				_imager(source._imager), _ioMutex(source._ioMutex),
				_antennaFlagCountPlot(source._antennaFlagCountPlot), _frequencyFlagCountPlot(source._frequencyFlagCountPlot),
				_frequencyPowerPlot(source._frequencyPowerPlot),
				_timeFlagCountPlot(source._timeFlagCountPlot)
			{
			}

			~ArtifactSet()
			{
			}

			ArtifactSet &operator=(const ArtifactSet &source)
			{
				_originalData = source._originalData;
				_contaminatedData = source._contaminatedData;
				_revisedData = source._revisedData;
				_metaData = source._metaData;
				_sensitivity = source._sensitivity;
				_imageSet = source._imageSet;
				_imageSetIndex = source._imageSetIndex;
				_imager = source._imager;
				_ioMutex = source._ioMutex;
				_antennaFlagCountPlot = source._antennaFlagCountPlot;
				_frequencyFlagCountPlot = source._frequencyFlagCountPlot;
				_frequencyPowerPlot = source._frequencyPowerPlot;
				_timeFlagCountPlot = source._timeFlagCountPlot;
				return *this;
			}

			void SetOriginalData(const TimeFrequencyData &data)
			{
				_originalData = data;
			}

			void SetRevisedData(const TimeFrequencyData &data)
			{
				_revisedData = data;
			}

			void SetContaminatedData(const TimeFrequencyData &data)
			{
				_contaminatedData = data;
			}

			void SetSensitivity(long double sensitivity)
			{
				_sensitivity = sensitivity;
			}

			const TimeFrequencyData &OriginalData() const { return _originalData; }
			TimeFrequencyData &OriginalData() { return _originalData; }

			const TimeFrequencyData &RevisedData() const { return _revisedData; }
			TimeFrequencyData &RevisedData() { return _revisedData; }

			const TimeFrequencyData &ContaminatedData() const { return _contaminatedData; }
			TimeFrequencyData &ContaminatedData() { return _contaminatedData; }

			long double Sensitivity() const { return _sensitivity; }

			class ImageSet *ImageSet() const { return _imageSet; }
			void SetImageSet(class ImageSet *imageSet) { _imageSet = imageSet; }
			void SetNoImageSet() { _imageSet = 0; _imageSetIndex = 0; }
			
			class ImageSetIndex *ImageSetIndex() const { return _imageSetIndex; }
			void SetImageSetIndex(class ImageSetIndex *imageSetIndex) { _imageSetIndex = imageSetIndex; }

			class UVImager *Imager() const { return _imager; }
			void SetImager(class UVImager *imager) { _imager = imager; }
			
			bool HasImageSet() const { return _imageSet != 0; }
			bool HasImageSetIndex() const { return _imageSetIndex != 0; }
			bool HasImager() const { return _imager != 0; }

			bool HasMetaData() const { return _metaData != 0; }
			TimeFrequencyMetaDataCPtr MetaData()
			{
				return _metaData;
			}
			void SetMetaData(TimeFrequencyMetaDataCPtr metaData)
			{
				_metaData = metaData;
			}

			boost::mutex &IOMutex()
			{
				return *_ioMutex;
			}

			class AntennaFlagCountPlot *AntennaFlagCountPlot()
			{
				return _antennaFlagCountPlot;
			}
			void SetAntennaFlagCountPlot(class AntennaFlagCountPlot *plot)
			{
				_antennaFlagCountPlot = plot;
			}
			class FrequencyFlagCountPlot *FrequencyFlagCountPlot()
			{
				return _frequencyFlagCountPlot;
			}
			void SetFrequencyFlagCountPlot(class FrequencyFlagCountPlot *plot)
			{
				_frequencyFlagCountPlot = plot;
			}
			class FrequencyPowerPlot *FrequencyPowerPlot()
			{
				return _frequencyPowerPlot;
			}
			void SetFrequencyPowerPlot(class FrequencyPowerPlot *plot)
			{
				_frequencyPowerPlot = plot;
			}
			class TimeFlagCountPlot *TimeFlagCountPlot()
			{
				return _timeFlagCountPlot;
			}
			void SetTimeFlagCountPlot(class TimeFlagCountPlot *plot)
			{
				_timeFlagCountPlot = plot;
			}
		private:
			TimeFrequencyData _originalData;
			TimeFrequencyData _contaminatedData;
			TimeFrequencyData _revisedData;
			TimeFrequencyMetaDataCPtr _metaData;
			long double _sensitivity;

			class ImageSet *_imageSet;
			class ImageSetIndex *_imageSetIndex;
			class UVImager *_imager;

			boost::mutex *_ioMutex;
			class AntennaFlagCountPlot *_antennaFlagCountPlot;
			class FrequencyFlagCountPlot *_frequencyFlagCountPlot;
			class FrequencyPowerPlot *_frequencyPowerPlot;
			class TimeFlagCountPlot *_timeFlagCountPlot;
	};
}

#endif //RFI_RFISTRATEGY_H
