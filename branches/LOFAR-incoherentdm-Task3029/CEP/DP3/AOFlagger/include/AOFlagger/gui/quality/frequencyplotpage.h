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
#ifndef GUI_QUALITY__FREQUENCYPLOTPAGE_H
#define GUI_QUALITY__FREQUENCYPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/statisticsderivator.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class FrequencyPlotPage : public TwoDimensionalPlotPage {
	public:
    FrequencyPlotPage() : _ftButton("FT")
		{
		}
		
		virtual void processStatistics(class StatisticsCollection *statCollection, const std::vector<AntennaInfo> &antennas)
		{
			_statistics.clear();
			
			const std::map<double, class DefaultStatistics> &map = statCollection->FrequencyStatistics();
			
			for(std::map<double, class DefaultStatistics>::const_iterator i=map.begin();i!=map.end();++i)
			{
				_statistics.insert(std::pair<double, DefaultStatistics>(i->first/1000000.0, i->second));
			}
			_ftStatistics.clear();
		}
		
		virtual const std::map<double, class DefaultStatistics> &GetStatistics() const
		{
			if(_ftButton.get_active())
			{
				return _ftStatistics;
			}
			else
				return _statistics;
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name, const std::string &yAxisDesc)
		{
			if(_ftButton.get_active())
				plot.StartLine(name, "Time (μs)", yAxisDesc, false);
			else
				plot.StartLine(name, "Frequency (MHz)", yAxisDesc, false);
		}
		
		virtual void addCustomPlotButtons(Gtk::VBox &container)
		{
			_ftButton.signal_clicked().connect(sigc::mem_fun(*this, &FrequencyPlotPage::onFTButtonClicked));
			container.pack_start(_ftButton);
			_ftButton.show();
		}
	private:
		void onFTButtonClicked()
		{
			if(_ftStatistics.empty())
				createFTStatistics();
			updatePlot();
		}
		
		void createFTStatistics()
		{
			_ftStatistics = StatisticsDerivator::PerformFT(_statistics);
		}
		
		std::map<double, class DefaultStatistics> _statistics;
		std::map<double, class DefaultStatistics> _ftStatistics;
		Gtk::CheckButton _ftButton;
};

#endif
