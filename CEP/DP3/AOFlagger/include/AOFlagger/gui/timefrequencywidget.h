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

#include <cairomm/surface.h>

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
		enum TFMap { BWMap, InvertedMap, ColorMap, RedBlueMap, HotColdMap, RedYellowBlueMap };
		enum Range { MinMax, Winsorized, Specified };
		TimeFrequencyWidget();
		~TimeFrequencyWidget();

		bool ShowOriginalMask() const { return _showOriginalMask; }
		void SetShowOriginalMask(bool newValue) { _showOriginalMask = newValue; }

		bool ShowAlternativeMask() const { return _showAlternativeMask; }
		void SetShowAlternativeMask(bool newValue) { _showAlternativeMask = newValue; }

		void SetColorMap(TFMap colorMap) { _colorMap = colorMap; }
		void SetRange(enum Range range)
		{
			_range = range;
		}
		enum Range Range() const
		{
			return _range;
		}
		void SetUseLogScale(bool useLogScale)
		{
			_useLogScale = useLogScale;
		}
		void Update(); 

		Image2DCPtr Image() const { return _image; }
		void SetImage(Image2DCPtr image) { _image = image; }

		Mask2DCPtr OriginalMask() const { return _originalMask; }
		void SetOriginalMask(Mask2DCPtr mask) { _originalMask = mask; }

		Mask2DCPtr AlternativeMask() const { return _alternativeMask; }
		void SetAlternativeMask(Mask2DCPtr mask) { _alternativeMask = mask; }

		Mask2DCPtr GetActiveMask() const;

		void SetHighlighting(bool newValue) { _highlighting = newValue; }
		class ThresholdConfig &HighlightConfig() { return *_highlightConfig; }
		bool HasImage() const { return _image != 0; }
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
		void ResetDomains();
		size_t StartTime() const { return _startTime; }
		size_t EndTime() const { return _endTime; }
		size_t StartFrequency() const { return _startFrequency; }
		size_t EndFrequency() const { return _endFrequency; }
		void SetSegmentedImage(SegmentedImageCPtr segmentedImage) { _segmentedImage = segmentedImage; }
		TimeFrequencyMetaDataCPtr GetMetaData() { return _metaData; }
		void SetMetaData(TimeFrequencyMetaDataCPtr metaData) { _metaData = metaData; }

		sigc::signal<void, size_t, size_t> &OnMouseMovedEvent() { return _onMouseMoved; }
		sigc::signal<void, size_t, size_t> &OnButtonReleasedEvent() { return _onButtonReleased; }
		
		num_t Max() const { return _max; }
		num_t Min() const { return _min; }
		
		void SetMax(num_t max) { _max = max; }
		void SetMin(num_t min) { _min = min; }
		
		void SavePdf(const std::string &filename);
		void SaveSvg(const std::string &filename);
		void SavePng(const std::string &filename);
		
		void SetShowAxisDescriptions(bool showAxisDescriptions)
		{
			_showAxisDescriptions = showAxisDescriptions;
		}
		void Clear();
	private:
		void findMinMax(Image2DCPtr image, Mask2DCPtr mask, num_t &min, num_t &max);
		void update(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height);
		void redrawWithoutChanges(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height);
		void shrinkImageBufferHorizontally();
		bool toUnits(double mouseX, double mouseY, int &posX, int &posY);
		bool onExposeEvent(GdkEventExpose* ev);
		bool onMotion(GdkEventMotion *event);
		bool onButtonReleased(GdkEventButton *event);
		class ColorMap *createColorMap();

		bool _isInitialized;
		unsigned _initializedWidth, _initializedHeight;
		Cairo::RefPtr<Cairo::ImageSurface> _imageSurface;

		bool _showOriginalMask, _showAlternativeMask;
		enum TFMap _colorMap;
		TimeFrequencyMetaDataCPtr _metaData;
		Image2DCPtr _image;
		Mask2DCPtr _originalMask, _alternativeMask;
		bool _highlighting;
		class ThresholdConfig *_highlightConfig;
		double _leftBorderSize, _rightBorderSize, _topBorderSize, _bottomBorderSize;

		size_t _startTime, _endTime;
		size_t _startFrequency, _endFrequency;
		SegmentedImageCPtr _segmentedImage;
		class HorizontalPlotScale *_horiScale;
		class VerticalPlotScale *_vertScale;
		class ColorScale *_colorScale;
		bool _useLogScale, _showAxisDescriptions;
		num_t _max, _min;
		enum Range _range;

		sigc::signal<void, size_t, size_t> _onMouseMoved;
		sigc::signal<void, size_t, size_t> _onButtonReleased;
};

#endif
