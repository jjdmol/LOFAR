//# OptionParser.h: Utility class to parse GNU style commandline options.
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_OPTIONPARSER_H
#define LOFAR_BBSCONTROL_OPTIONPARSER_H

// \file
// Utility class to parse GNU style commandline options.

#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class OptionParser
{
public:
    typedef vector<string>  ArgumentList;

    static ArgumentList makeArgumentList(int argc, char *argv[]);

    void appendOption(const string &name, const string &shortOption,
        const string &longOption, const string &description = "");
    void appendOptionWithArgument(const string &name, const string &shortOption,
        const string &longOption, const string &description = "");

    void appendOptionWithDefault(const string &name, const string &shortOption,
        const string &longOption, const string &defaultValue,
        const string &description = "");

    string documentation(size_t width = 56, const string &prefix = "\t\t\t")
        const;

    ParameterSet parse(ArgumentList &args) const;

private:
    struct OptionDescriptor
    {
        string  name;
        string  description;

        bool    hasShortOption;
        char    shortOption;
        string  longOption;

        bool    hasArgument;
        bool    hasDefault;
        string  defaultValue;
    };

    void copyDefaultOptions(ParameterSet &out) const;
    char parseShortOption(const string &option) const;
    string parseLongOption(const string &option) const;

    void appendOption(OptionDescriptor desc, const string &shortOption,
        const string &longOption);
    const OptionDescriptor &findShortOption(char option) const;
    const OptionDescriptor &findLongOption(const string &prefix) const;

    vector<OptionDescriptor>    itsOptions;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
