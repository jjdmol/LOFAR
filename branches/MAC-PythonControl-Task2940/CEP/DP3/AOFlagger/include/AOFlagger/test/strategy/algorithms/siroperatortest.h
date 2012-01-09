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
#ifndef AOFLAGGER_SIROPERATORTEST_H
#define AOFLAGGER_SIROPERATORTEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/mask2d.h>

#include <AOFlagger/strategy/algorithms/siroperator.h>

#include <AOFlagger/util/rng.h>

class SIROperatorTest : public UnitTest {
	public:
		SIROperatorTest() : UnitTest("Scale invariant dilation")
		{
			AddTest(TestSingleDilation(), "Single dilation");
			AddTest(TestDilationSpeed(), "Dilation speed");
			AddTest(TestTimeDilation(), "Time dilation");
			AddTest(TestFrequencyDilation(), "Frequency dilation");
			AddTest(TestTimeDilationSpeed(), "Time dilation speed");
		}
		
	private:
		struct TestSingleDilation : public Asserter
		{
			void operator()();
		};
		struct TestDilationSpeed : public Asserter
		{
			void operator()();
		};
		struct TestTimeDilation : public Asserter
		{
			void operator()();
		};
		struct TestFrequencyDilation : public Asserter
		{
			void operator()();
		};
		struct TestTimeDilationSpeed : public Asserter
		{
			void operator()();
		};
		
		static std::string flagsToString(const bool *flags, unsigned size)
		{
			std::stringstream s;
			for(unsigned i=0;i<size;++i)
			{
				s << (flags[i] ? 'x' : ' ');
			}
			return s.str();
		}
		
		static void setFlags(bool *flags, const std::string &str)
		{
			unsigned index = 0;
			for(std::string::const_iterator i = str.begin(); i!=str.end(); ++i)
			{
				flags[index] = ((*i) == 'x');
				++index;
			}
		}

		static std::string maskToString(Mask2DCPtr mask)
		{
			std::stringstream s;
			for(unsigned y=0;y<mask->Height();++y)
			{
				for(unsigned x=0;x<mask->Width();++x)
				{
					s << (mask->Value(x, y) ? 'x' : ' ');
				}
			}
			return s.str();
		}
		
		static void setMask(Mask2DPtr mask, const std::string &str)
		{
			std::string::const_iterator i = str.begin();
			for(unsigned y=0;y<mask->Height();++y)
			{
				for(unsigned x=0;x<mask->Width();++x)
				{
					mask->SetValue(x, y, (*i) == 'x');
					++i;
				}
			}
		}

};

inline void SIROperatorTest::TestSingleDilation::operator()()
{
	bool *flags = new bool[40];
	setFlags(flags, "     x    ");
	
	SIROperator::Operate(flags, 10, 0.0);
	AssertEquals(flagsToString(flags, 10), "     x    ", "Eta=0.0, single center flagged, no enlarge");
	
	SIROperator::Operate(flags, 10, 0.4);
	AssertEquals(flagsToString(flags, 10), "     x    ", "Eta=0.4, single center flagged");
	
	SIROperator::Operate(flags, 10, 0.5);
	AssertEquals(flagsToString(flags, 10), "    xxx   ", "Eta=0.5, from one to three samples");

	SIROperator::Operate(flags, 10, 0.0);
	AssertEquals(flagsToString(flags, 10), "    xxx   ");
	
	SIROperator::Operate(flags, 10, 0.25);
	AssertEquals(flagsToString(flags, 10), "   xxxxx  ");

	SIROperator::Operate(flags, 10, 0.16);
	AssertEquals(flagsToString(flags, 10), "   xxxxx  ");

	SIROperator::Operate(flags, 10, 0.17);
	AssertEquals(flagsToString(flags, 10), "  xxxxxxx ");

	SIROperator::Operate(flags, 10, 1.0);
	AssertEquals(flagsToString(flags, 10), "xxxxxxxxxx");
	
	setFlags(flags, "xx xx     ");

	SIROperator::Operate(flags, 10, 0.0);
	AssertEquals(flagsToString(flags, 10), "xx xx     ");

	SIROperator::Operate(flags, 10, 0.19);
	AssertEquals(flagsToString(flags, 10), "xx xx     ", "Did not fill hole");

	setFlags(flags, "xx xx     ");
	SIROperator::Operate(flags, 10, 0.2);
	AssertEquals(flagsToString(flags, 10), "xxxxx     ", "Fills hole");
	
	setFlags(flags, "x         ");
	SIROperator::Operate(flags, 10, 0.5);
	AssertEquals(flagsToString(flags, 10), "xx        ", "Left border, isolated");
	SIROperator::Operate(flags, 10, 0.4);
	AssertEquals(flagsToString(flags, 10), "xxx       ", "Left border, combined");
	
	setFlags(flags, "         x");
	SIROperator::Operate(flags, 10, 0.5);
	AssertEquals(flagsToString(flags, 10), "        xx", "Right border, isolated");
	SIROperator::Operate(flags, 10, 0.4);
	AssertEquals(flagsToString(flags, 10), "       xxx", "Right border, combined");
	
	setFlags(flags, " x        ");
	SIROperator::Operate(flags, 10, 0.4);
	AssertEquals(flagsToString(flags, 10), " x        ", "Left border empty");
	
	setFlags(flags, "        x ");
	SIROperator::Operate(flags, 10, 0.4);
	AssertEquals(flagsToString(flags, 10), "        x ", "Right border empty");
	
	//               0    5    0    5    0    5    0    5    
	setFlags(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::Operate(flags, 40, 0.2);
	AssertEquals(flagsToString(flags, 40), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ", "Input: '     xxxxxx xx xx x x xxx xxxxx         '");

	setFlags(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::Operate(flags, 40, 0.3);
	AssertEquals(flagsToString(flags, 40), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ", "Input: '     xxxxxx xx xx x x xxx xxxxx         '");

	setFlags(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::Operate(flags, 40, 0.4);
	AssertEquals(flagsToString(flags, 40), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setFlags(flags, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	SIROperator::Operate(flags, 40, 0.3);
	AssertEquals(flagsToString(flags, 40), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	
	setFlags(flags, "      x   x  x xx xxx    ");
	SIROperator::Operate(flags, 25, 0.5);
	AssertEquals(flagsToString(flags, 25), "     xxxxxxxxxxxxxxxxxxxx");
	
	delete[] flags;
}

inline void SIROperatorTest::TestDilationSpeed::operator()()
{
	const unsigned flagsSize = 100000;
	bool flags[flagsSize];
	for(unsigned i=0;i<flagsSize; ++i)
	{
		flags[i] = (RNG::Uniform() >= 0.2);
	}
	SIROperator::Operate(flags, flagsSize, 0.1);
}

inline void SIROperatorTest::TestTimeDilation::operator()()
{
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(10, 1);
	setMask(mask, "     x    ");
	
	SIROperator::OperateHorizontally(mask, 0.0);
	AssertEquals(maskToString(mask), "     x    ", "Eta=0.0, single center flagged, no enlarge");
	
	SIROperator::OperateHorizontally(mask, 0.4);
	AssertEquals(maskToString(mask), "     x    ", "Eta=0.4, single center flagged");
	
	SIROperator::OperateHorizontally(mask, 0.5);
	AssertEquals(maskToString(mask), "    xxx   ", "Eta=0.5, from one to three samples");

	SIROperator::OperateHorizontally(mask, 0.0);
	AssertEquals(maskToString(mask), "    xxx   ");
	
	SIROperator::OperateHorizontally(mask, 0.25);
	AssertEquals(maskToString(mask), "   xxxxx  ");

	SIROperator::OperateHorizontally(mask, 0.16);
	AssertEquals(maskToString(mask), "   xxxxx  ");

	SIROperator::OperateHorizontally(mask, 0.17);
	AssertEquals(maskToString(mask), "  xxxxxxx ");

	SIROperator::OperateHorizontally(mask, 0.6);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");

	SIROperator::OperateHorizontally(mask, 1.0);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");
	
	setMask(mask, "xx xx     ");

	SIROperator::OperateHorizontally(mask, 0.0);
	AssertEquals(maskToString(mask), "xx xx     ");

	SIROperator::OperateHorizontally(mask, 0.19);
	AssertEquals(maskToString(mask), "xx xx     ", "Did not fill hole");
	
	SIROperator::OperateHorizontally(mask, 0.2);
	AssertEquals(maskToString(mask), "xxxxx     ", "Fill hole");
	
	mask = Mask2D::CreateSetMaskPtr<false>(40, 1);
	//             0    5    0    5    0    5    0    5    
	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateHorizontally(mask, 0.2);
	AssertEquals(maskToString(mask), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateHorizontally(mask, 0.3);
	AssertEquals(maskToString(mask), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateHorizontally(mask, 0.4);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setMask(mask, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	SIROperator::OperateHorizontally(mask, 0.3);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

inline void SIROperatorTest::TestFrequencyDilation::operator()()
{
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(1, 10);
	setMask(mask, "     x    ");
	
	SIROperator::OperateVertically(mask, 0.0);
	AssertEquals(maskToString(mask), "     x    ", "Eta=0.0, single center flagged, no enlarge");
	
	SIROperator::OperateVertically(mask, 0.4);
	AssertEquals(maskToString(mask), "     x    ", "Eta=0.4, single center flagged, eta 0.4");
	
	SIROperator::OperateVertically(mask, 0.5);
	AssertEquals(maskToString(mask), "    xxx   ", "Eta=0.5, from one to three samples");

	SIROperator::OperateVertically(mask, 0.0);
	AssertEquals(maskToString(mask), "    xxx   ");
	
	SIROperator::OperateVertically(mask, 0.25);
	AssertEquals(maskToString(mask), "   xxxxx  ");
	
	SIROperator::OperateVertically(mask, 0.16);
	AssertEquals(maskToString(mask), "   xxxxx  ");

	SIROperator::OperateVertically(mask, 0.17);
	AssertEquals(maskToString(mask), "  xxxxxxx ");

	SIROperator::OperateVertically(mask, 0.6);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");

	SIROperator::OperateVertically(mask, 1.0);
	AssertEquals(maskToString(mask), "xxxxxxxxxx");
	
	setMask(mask, "xx xx     ");

	SIROperator::OperateVertically(mask, 0.0);
	AssertEquals(maskToString(mask), "xx xx     ");

	SIROperator::OperateVertically(mask, 0.19);
	AssertEquals(maskToString(mask), "xx xx     ", "Did not fill hole");
	
	SIROperator::OperateVertically(mask, 0.2);
	AssertEquals(maskToString(mask), "xxxxx     ", "Fill hole");
	
	mask = Mask2D::CreateSetMaskPtr<false>(1, 40);
	//             0    5    0    5    0    5    0    5    
	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateVertically(mask, 0.2);
	AssertEquals(maskToString(mask), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateVertically(mask, 0.3);
	AssertEquals(maskToString(mask), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ");

	setMask(mask, "     xxxxxx xx xx x x xxx xxxxx         ");
	SIROperator::OperateVertically(mask, 0.4);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setMask(mask, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	SIROperator::OperateVertically(mask, 0.3);
	AssertEquals(maskToString(mask), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

inline void SIROperatorTest::TestTimeDilationSpeed::operator()()
{
	const unsigned flagsSize = 10000;
	const unsigned channels = 256;
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(flagsSize, channels);
	for(unsigned y=0;y<channels;++y)
	{
		for(unsigned i=0;i<flagsSize; ++i)
		{
			mask->SetValue(i, 0, (RNG::Uniform() >= 0.2));
		}
	}
	SIROperator::OperateHorizontally(mask, 0.1);
}

#endif
