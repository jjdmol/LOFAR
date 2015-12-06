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

    // Helper function to convert standard command line arguments to an
    // \c ArgumentList instance.
    static ArgumentList makeArgumentList(int argc, char *argv[]);

    // Define options recognized by the parser. These functions should be called
    // prior to calling \c parse() or \c documentation().
    //
    // An option can be given a to a short option name (e.g. -s), a long option
    // name (e.g. --long-option), or both. If both \p shortOption and
    // \p longOption are left empty an exception will be thrown.
    //
    // For long options that expect an argument, \p name in uppercase can be
    // used in \p description to refer to the argument value.
    // @{

    // Define a switch (i.e. an option without an argument).
    void addOption(const string &name, const string &shortOption,
        const string &longOption, const string &description = "");

    // Define an option that expects an argument.
    void addOptionWithArgument(const string &name, const string &shortOption,
        const string &longOption, const string &description = "");

    // Define an option that expects an argument. The value of the argument
    // defaults to \p defaultValue.
    void addOptionWithDefault(const string &name, const string &shortOption,
        const string &longOption, const string &defaultValue,
        const string &description = "");
    // @}

    // Generate GNU style option documentation. The documentation will be
    // wrapped on word boundaries such that its width in characters is smaller
    // than or equal to \p width (excluding the width of \p prefix).
    //
    // Note that escape sequences are not handled specially while word wrapping,
    // thus wrapping will not work correctly for text that contains tab,
    // newline, backspace, et cetera.
    string documentation(size_t width = 56, const string &prefix = "\t\t\t")
        const;

    // Parse the argument list. Options are removed from the input
    // \c ArgumentList instance, such that only non-option arguments are left
    // after \c parse() returns. The first argument is not considered for option
    // parsing because it contains the name of the application by convention.
    // The first non-option argument left in \p args after \c parse() returns is
    // therefore always equal to the first argument in \p args before \c parse()
    // was called.
    ParameterSet parse(ArgumentList &args) const;

private:
    // Description of an option.
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

    void addOption(OptionDescriptor desc, const string &shortOption,
        const string &longOption);
    const OptionDescriptor &findShortOption(char option) const;
    const OptionDescriptor &findLongOption(const string &prefix) const;

    vector<OptionDescriptor>    itsOptions;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
