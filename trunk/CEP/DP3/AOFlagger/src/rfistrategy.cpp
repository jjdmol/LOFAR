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

#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/xmlwriter.h>

int main(int argc, char *argv[])
{
	std::cout << 
			"RFI strategy file writer\n"
			"This program will write an RFI strategy to a file, to run it with the\n"
			"rficonsole or the rfigui.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n"
			<< std::endl;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		std::string flag(argv[parameterIndex]+1);
		{
			std::cerr << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood." << std::endl;
			return 1;
		}
	}
	if((int) parameterIndex > argc-2)
	{
		std::cerr << "Usage: " << argv[0] << " <profile> <filename>\n\n"
			<< "Profiles:\n"
			<< "  fast     Fastest strategy that provides a moderate\n"
			<< "           result in quality. Will flag the measurement set using\n"
			<< "           Stokes-I values, and by using a relatively small\n"
			<< "           sliding window.\n"
			<< "  average  Best trade-off between speed and quality. Will\n"
			<< "           flag the measurement set using XX and YY values using\n"
			<< "           an average sliding window size.\n"
			<< "  best     Highest quality detection. Will flag each\n"
			<< "           polarization individually, using a relatively large\n"
			<< "           sliding window.\n"
			<< "  pedantic Pedantic detection. Like the 'best' profile,\n"
			<< "           but will flag all channels completely that are still\n"
			<< "           deviating from others after flagging. Flags about twice\n"
			<< "           as much.\n"
			<< "<filename> is the filename to which the strategy is written. This\n"
			<< "file should have the extension \".rfis\".\n\n"
			<< "All profiles implement the SumThreshold method. The details of this\n"
			<< "method are described in the article named \"Post-correlation radio\n"
			<< "frequency interference classiﬁcation methods\", submitted to MNRAS.\n";

		return 1;
	}

	std::string profile(argv[parameterIndex]), filename(argv[parameterIndex+1]);

	rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
	if(profile == "fast")
		strategy->LoadFastStrategy();
	else if(profile == "average")
		strategy->LoadAverageStrategy();
	else if(profile == "best")
		strategy->LoadBestStrategy();
	else if(profile == "pedantic")
		strategy->LoadBestStrategy(true);
	else {
		std::cerr << "Unknown profile: " << profile << std::endl;
		return 1;
	}
	rfiStrategy::XmlWriter writer;
	std::cout << "Writing strategy..." << std::endl;
	writer.WriteStrategy(*strategy, filename);
	delete strategy;
}
