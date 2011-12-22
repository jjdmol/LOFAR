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
#ifndef GUI_QUALITY__SUMMARYPAGE_H
#define GUI_QUALITY__SUMMARYPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>

#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/statisticsderivator.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SummaryPage : public Gtk::HBox {
	public:
		SummaryPage()
		{
			add(_textView);
			_textView.show();
			
			CloseStatistics();
		}
		
		void SetStatistics(class StatisticsCollection *statCollection)
		{
			_statCollection = statCollection;
			updateText();
		}
		void CloseStatistics()
		{
			_statCollection = 0;
			_textView.get_buffer()->set_text("No open measurement set");
		}
		bool HasStatistics() const
		{
			return _statCollection != 0;
		}
		
	private:
		void updateText()
		{
			DefaultStatistics statistics(_statCollection->PolarizationCount());
			Glib::RefPtr<Gtk::TextBuffer> buffer = _textView.get_buffer();
			
			buffer->set_text("Summary of statistics\n\n");
			
			_statCollection->GetGlobalCrossBaselineStatistics(statistics);
			buffer->insert(buffer->end(), "Statistics of cross-correlated baselines\n");
			addText(statistics);

			_statCollection->GetGlobalAutoBaselineStatistics(statistics);
			buffer->insert(buffer->end(), "\nStatistics of auto-correlated baselines\n");
			addText(statistics);
		}
		
		void addText(DefaultStatistics &statistics)
		{
			Glib::RefPtr<Gtk::TextBuffer > buffer = _textView.get_buffer();
			
			unsigned long totalRFICount = 0;
			unsigned long totalCount = 0;
			const unsigned polarizationCount = statistics.PolarizationCount();
			double variance[polarizationCount];
			double dVariance[polarizationCount];
			for(unsigned p=0;p<polarizationCount;++p)
			{
				totalRFICount += statistics.rfiCount[p];
				totalCount += statistics.rfiCount[p] + statistics.count[p];
				variance[p] = StatisticsDerivator::VarianceAmplitude(statistics.count[p], statistics.sum[p], statistics.sumP2[p]);
				dVariance[p] = StatisticsDerivator::VarianceAmplitude(statistics.dCount[p], statistics.dSum[p], statistics.dSumP2[p]);
			}
			
			double rfiRatioValue = round(((double) totalRFICount * 10000.0 / (double) totalCount)) * 0.01;
			
			std::ostringstream s;
			s << "Total RFI ratio = " << rfiRatioValue << "%\n";
			s << "Variance amplitude = ";
			addValues(variance, polarizationCount, s);
			s << " Jy\nDifferential variance amplitude = ";
			addValues(dVariance, polarizationCount, s);
			s << " Jy\n";
			buffer->insert(buffer->end(), s.str());
		}

		void addValues(const double *values, unsigned polarizationCount, std::ostringstream &s)
		{
			s << '[' << values[0];
			for(unsigned p=1;p<polarizationCount;++p)
			{
				s << ", " << values[p];
			}
			s << ']';
		}

		Gtk::TextView _textView;
		
		StatisticsCollection *_statCollection;
};

#endif
