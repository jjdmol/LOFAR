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
#ifndef TIMEFREQUENCYWIDGET_H
#define TIMEFREQUENCYWIDGET_H

#include <gtkmm/drawingarea.h>

#include <vector>

#include "../msio/image2d.h"
#include "../msio/timefrequencydata.h"
#include "../msio/timefrequencymetadata.h"
#include "../msio/segmentedimage.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class TimeFrequencyWidget : public Gtk::DrawingArea {
	public:
		enum TFMap { TFLogMap, TFBWMap, TFColorMap };
		enum TFImage { TFOriginalImage, TFRevisedImage, TFContaminatedImage, TFDifferenceImage };
		TimeFrequencyWidget();
		~TimeFrequencyWidget();
		void SetNewData(const class TimeFrequencyData &image, TimeFrequencyMetaDataCPtr metaData);
		void Init();
		void SetShowOriginalFlagging(bool newValue) throw() { _showOriginalFlagging = newValue; }
		void SetShowAlternativeFlagging(bool newValue) throw() { _showAlternativeFlagging = newValue; }
		void SetColorMap(TFMap colorMap) throw() { _colorMap = colorMap; }
		void Update() throw(); 
		void AddAlternativeFlagging(Mask2DCPtr mask) throw();
		Image2DCPtr Image() { return _image; }

		TimeFrequencyData GetActiveData() const
		{
			TimeFrequencyData data(getActiveDataWithOriginalFlags());
			data.SetNoMask();
			if(_showOriginalFlagging)
			{
				if(_showAlternativeFlagging)
				{
					data.SetMask(_original);
					data.JoinMask(_contaminated);
				} else
					data.SetMask(_original);
			} else {
				if(_showAlternativeFlagging)
					data.SetMask(_contaminated);
			}
			if(_startTime != 0 || _endTime != data.ImageWidth() || _startFrequency != 0 || _endFrequency != data.ImageHeight())
				data.Trim(_startTime, _startFrequency, _endTime, _endFrequency); 
			return data;
		}
		Mask2DCPtr GetActiveMask() const;

		TimeFrequencyData &OriginalData() { return _original; }
		const TimeFrequencyData &OriginalData() const { return _original; }

		TimeFrequencyData &RevisedData() { return _revised; }
		const TimeFrequencyData &RevisedData() const { return _revised; }

		void SetRevisedData(const TimeFrequencyData &data)
		{
			_revised = data;
		}
		const TimeFrequencyData &ContaminatedData() const { return _contaminated; }
		TimeFrequencyData &ContaminatedData() { return _contaminated; }
		void SetContaminatedData(const TimeFrequencyData &data)
		{
			_contaminated = data;
		} 
		void SetVisualizedImage(TFImage visualizedImage) throw() { _visualizedImage = visualizedImage; }
		void ClearBackground() throw();

		Mask2DCPtr Mask() const throw() { return _mask; }
		void SetHighlighting(bool newValue) throw() { _highlighting = newValue; }
		class ThresholdConfig &HighlightConfig() throw() { return *_highlightConfig; }
		bool HasImage() const throw() { return _hasImage; }
		void SetTimeDomain(size_t startTime, size_t endTime)
		{
			_startTime = startTime;
			_endTime = endTime;
		}
		void SetFrequencyDomain(size_t startFrequency, size_t endFrequency)
		{
			_startFrequency = startFrequency;
			_endFrequency = endFrequency;
		}
		size_t StartTime() const throw() { return _startTime; }
		size_t EndTime() const throw() { return _endTime; }
		size_t StartFrequency() const throw() { return _startFrequency; }
		size_t EndFrequency() const throw() { return _endFrequency; }
		void SetSegmentedImage(SegmentedImageCPtr segmentedImage) { _segmentedImage = segmentedImage; }
	private:
		void Clear();
		void findMinMax(Image2DCPtr image, Mask2DCPtr mask, long double &min, long double &max);
		void redraw();
		void ShrinkPixBufHorizontally();
		bool onExposeEvent(GdkEventExpose* ev);
		ColorMap *createColorMap();
		const TimeFrequencyData getActiveDataWithOriginalFlags() const
		{
			switch(_visualizedImage)
			{
				case TFOriginalImage:
				default:
					return _original;
				case TFRevisedImage:
					return _revised;
				case TFContaminatedImage:
					return _contaminated;
				case TFDifferenceImage:
					return getDifference();
			}
		}
		const TimeFrequencyData getDifference() const;

		bool _isInitialized;
		Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
		unsigned _pixBufWidth, _pixBufHeight;

		bool _showOriginalFlagging, _showAlternativeFlagging, _winsorizedStretch, _useColor;
		enum TFMap _colorMap;
		enum TFImage _visualizedImage;
		TimeFrequencyData _original, _revised, _contaminated;
		TimeFrequencyMetaDataCPtr _metaData;
		Image2DCPtr _image;
		Mask2DCPtr _mask;
		bool _highlighting;
		class ThresholdConfig *_highlightConfig;
		bool _hasImage;
		double _leftBorderSize, _rightBorderSize, _topBorderSize, _bottomBorderSize;

		size_t _startTime, _endTime;
		size_t _startFrequency, _endFrequency;
		SegmentedImageCPtr _segmentedImage;
		class VerticalNumericScale *_vertScale;
};

#endif
