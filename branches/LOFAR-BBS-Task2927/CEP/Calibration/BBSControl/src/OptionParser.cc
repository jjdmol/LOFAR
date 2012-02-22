//# OptionParser.cc: Utility class to parse GNU style commandline options.
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

#include <lofar_config.h>
#include <BBSControl/OptionParser.h>
#include <BBSControl/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_algorithm.h>
#include <iterator>
#include <cctype>

namespace LOFAR
{
namespace BBS
{

namespace
{
    // Check if \p in starts with \p prefix.
    bool starts_with(const string &in, const string &prefix);

    // Convert string to uppercase.
    string toupper(string in);

    // Split input into lines of width characters long (excluding prefix). Lines
    // are split on a word boundary when possible. Special characters such as
    // tab or backspace are not handled properly.
    string wrap(const string &in, size_t width, const string &prefix = "");

    // Ensure the option character is an alphanumeric character (i.e. isalnum()
    // returns true).
    void assertOptionCharacterValidity(char option);

} //# unnamed namespace

void OptionParser::addOption(const string &name, const string &shortOption,
    const string &longOption, const string &description)
{
    OptionDescriptor desc = {name, description, false, '\0', "", false,
        false, ""};
    addOption(desc, shortOption, longOption);
}

void OptionParser::addOptionWithArgument(const string &name,
    const string &shortOption, const string &longOption,
    const string &description)
{
    OptionDescriptor desc = {name, description, false, '\0', "", true,
        false, ""};
    addOption(desc, shortOption, longOption);
}

void OptionParser::addOptionWithDefault(const string &name,
    const string &shortOption, const string &longOption,
    const string &defaultValue,const string &description)
{
    OptionDescriptor desc = {name, description, false, '\0', "", true, true,
        defaultValue};
    addOption(desc, shortOption, longOption);
}

string OptionParser::documentation(size_t width, const string &prefix) const
{
    ostringstream oss;
    for(vector<OptionDescriptor>::const_iterator it = itsOptions.begin(),
        end = itsOptions.end(); it != end; ++it)
    {
        oss << "  ";

        if(it->hasShortOption)
        {
            oss << "-" << it->shortOption;
        }
        else
        {
            oss << "  ";
        }

        if(!it->longOption.empty())
        {
            oss << (it->hasShortOption ? ", " : "  ");
            oss << "--" << it->longOption;

            if(it->hasArgument)
            {
                oss << "=" << toupper(it->name);
            }
        }

        if(it->hasDefault)
        {
            oss << " (" << it->defaultValue << ")";
        }

        oss << endl;

        if(it->description.size())
        {
            oss << wrap(it->description, width, prefix) << endl;
        }
    }

    return oss.str();
}

ParameterSet OptionParser::parse(OptionParser::ArgumentList &args) const
{
    ParameterSet options;
    copyDefaultOptions(options);

    // Skip first argument (that is never an option).
    ArgumentList::iterator it = args.begin(), end = args.end();
    ++it;

    // Parse options. Non-option arguments are swapped to the front of the
    // argument list.
    ArgumentList::iterator pivot = it;
    while(it != end)
    {
        if(*it == "-")
        {
            // A literal "-" is an ordinary non-option argument.
            swap(*pivot++, *it++);
            continue;
        }

        if(*it == "--")
        {
            // "--" stops argument parsing (to support non-options arguments
            // that start with "-" or "--".
            ++it;
            break;
        }

        if(starts_with(*it, "--"))
        {
            // Parse long option.
            const size_t pos = it->find('=', 2);
            const bool foundEquals = (pos != string::npos);

            string option = it->substr(2, (foundEquals ? pos - 2
                : string::npos));
            const OptionDescriptor &desc = findLongOption(option);

            if(desc.hasArgument)
            {
                if(!foundEquals || it->size() <= (pos + 1))
                {
                    THROW(OptionParserException, "Argument required for option:"
                        " --" << option);
                }

                options.replace(desc.name, it->substr(pos + 1));
            }
            else
            {
                if(foundEquals)
                {
                    THROW(OptionParserException, "No argument allowed for"
                        " option: --" << option);
                }

                options.replace(desc.name, "T");
            }
        }
        else if(starts_with(*it, "-"))
        {
            ASSERT(it->size() > 1);

            // Parse short option.
            char option = (*it)[1];
            assertOptionCharacterValidity(option);

            const OptionDescriptor &desc = findShortOption(option);
            if(desc.hasArgument)
            {
                // It is allowed to pass a short option and its argument as a
                // single command line argument. Otherwise, the next command
                // line argument is the option's argument.
                if(it->size() > 2)
                {
                    options.replace(desc.name, it->substr(2));
                }
                else
                {
                    ++it;

                    if(it == end)
                    {
                        THROW(OptionParserException, "Argument required for"
                            " option: -" << option);
                    }

                    options.replace(desc.name, *it);
                }
            }
            else
            {
                options.replace(desc.name, "T");

                if(it->size() > 2)
                {
                    // Multiple short options without arguments may be passed
                    // as a single command line argument.
                    for(size_t i = 2; i < it->size(); ++i)
                    {
                        char option = (*it)[i];
                        assertOptionCharacterValidity(option);

                        const OptionDescriptor &desc =
                            findShortOption((*it)[i]);

                        if(desc.hasArgument)
                        {
                            THROW(OptionParserException, "Options that require"
                                " an argument should be given separately: -"
                                << option);
                        }
                        options.replace(desc.name, "T");
                    }
                }
            }
        }
        else
        {
            // Non-option argument: swap to the front of the argument list.
            swap(*pivot++, *it);
        }

        ++it;
    }

    // Remaining argument are non-option arguments, so swap them to the front
    // of the argument list.
    while(it != end)
    {
        swap(*pivot++, *it++);
    }

    // Remove all parsed options from the argument list.
    args.erase(pivot, end);
    return options;
}

void OptionParser::copyDefaultOptions(ParameterSet &out) const
{
    for(vector<OptionDescriptor>::const_iterator it = itsOptions.begin(),
        end = itsOptions.end(); it != end; ++it)
    {
        if(it->hasDefault)
        {
            out.replace(it->name, it->defaultValue);
        }
    }
}

char OptionParser::parseShortOption(const string &option) const
{
    if(option.size() != 2 || option[0] != '-')
    {
        THROW(OptionParserException, "Short options should consist of \"-\""
            " followed by a single alphanumeric character: " << option);
    }
    assertOptionCharacterValidity(option[1]);
    return option[1];
}

string OptionParser::parseLongOption(const string &option) const
{
    if(option.size() <= 2 || !starts_with(option, "--"))
    {
        THROW(OptionParserException, "Long options should start with \"--\": "
            << option);
    }
    return option.substr(2);
}

void OptionParser::addOption(OptionParser::OptionDescriptor desc,
    const string &shortOption, const string &longOption)
{
    if(shortOption.empty() && longOption.empty())
    {
        THROW(OptionParserException, "Expected either a short option name, a"
            " long option name, or both.");
    }

    if(!shortOption.empty())
    {
        desc.shortOption = parseShortOption(shortOption);
        desc.hasShortOption = true;
    }

    if(!longOption.empty())
    {
        desc.longOption = parseLongOption(longOption);
    }

    itsOptions.push_back(desc);
}

const OptionParser::OptionDescriptor &OptionParser::findShortOption(char option)
    const
{
    for(vector<OptionDescriptor>::const_iterator it = itsOptions.begin(),
        end = itsOptions.end(); it != end; ++it)
    {
        if(it->hasShortOption && it->shortOption == option)
        {
            return *it;
        }
    }

    THROW(OptionParserException, "Unrecognized option: -" << option);
}

const OptionParser::OptionDescriptor&
OptionParser::findLongOption(const string &prefix) const
{
    size_t count = 0;
    vector<OptionDescriptor>::const_iterator option = itsOptions.end();
    for(vector<OptionDescriptor>::const_iterator it = itsOptions.begin(),
        end = itsOptions.end(); it != end; ++it)
    {
        if(starts_with(it->longOption, prefix))
        {
            option = it;
            ++count;
        }
    }

    if(count != 1)
    {
        THROW(OptionParserException, (count == 0 ? "Unrecognized" : "Ambiguous")
            << " option: --" << prefix);
    }

    return *option;
}

OptionParser::ArgumentList OptionParser::makeArgumentList(int argc,
    char *argv[])
{
    ArgumentList arguments;
    for(int i = 0; i < argc; ++i)
    {
        arguments.push_back(string(argv[i]));
    }
    return arguments;
}

namespace
{
    bool starts_with(const string &in, const string &prefix)
    {
        string::const_iterator it_in = in.begin(), end_in = in.end(),
            it_prefix = prefix.begin(), end_prefix = prefix.end();

        while(it_prefix != end_prefix)
        {
            if(it_in == end_in || *it_in != *it_prefix)
            {
                return false;
            }

            ++it_in;
            ++it_prefix;
        }

        return true;
    }

    string toupper(string in)
    {
        transform(in.begin(), in.end(), in.begin(),
            static_cast<int(*)(int)>(std::toupper));
        return in;
    }

    string wrap(const string &in, size_t width, const string &prefix)
    {
        ostringstream oss;

        string buffer = prefix;
        size_t pivot = string::npos;

        string::const_iterator it = in.begin(), end = in.end();
        while(it != end)
        {
            if(*it == ' ')
            {
                pivot = buffer.size();
            }

            buffer.push_back(*it);

            if(buffer.size() - prefix.size() > width)
            {
                if(pivot != string::npos)
                {
                    // Split buffer before pivot and skip space directly after
                    // pivot.
                    oss << buffer.substr(0, pivot) << endl;
                    buffer = prefix + buffer.substr(pivot + 1);
                }
                else
                {
                    // Split at width characters and move the last character to
                    // the next line.
                    oss << buffer.substr(0, prefix.size() + width) << endl;
                    buffer = prefix + *buffer.rbegin();
                }

                pivot = string::npos;
            }

            ++it;
        }

        // Write remainder.
        if(buffer.size() > prefix.size())
        {
            oss << buffer;
        }

        return oss.str();
    }

    void assertOptionCharacterValidity(char option)
    {
        if(!isalnum(option))
        {
            THROW(OptionParserException, "Invalid option character: "
                << option);
        }
    }
} //# unnamed namespace

} //# namespace BBS
} //# namespace LOFAR
