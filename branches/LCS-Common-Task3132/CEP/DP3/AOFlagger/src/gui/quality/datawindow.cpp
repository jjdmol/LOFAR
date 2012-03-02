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

#include <AOFlagger/gui/quality/datawindow.h>
#include <AOFlagger/gui/plot/plot2d.h>

#include <sstream>
#include <iomanip>

void DataWindow::SetData(const Plot2D &plot)
{
	std::stringstream _dataStream;
	_dataStream << std::setprecision(14);
	if(plot.PointSetCount() != 0)
	{
		const Plot2DPointSet &pointSet = plot.GetPointSet(0);
		const size_t valueCount = pointSet.Size();
		for(size_t i=0; i<valueCount; ++i)
		{
			const double
				x = pointSet.GetX(i),
				y = pointSet.GetY(i);
			if(pointSet.HasTickLabels())
			{
				std::string label = pointSet.TickLabels()[i];
				_dataStream << i << '\t' << label << '\t' << y << '\n';
			}
			else
				_dataStream << i << '\t' << x << '\t' << y << '\n';
		}
	}
	SetData(_dataStream.str());
}

