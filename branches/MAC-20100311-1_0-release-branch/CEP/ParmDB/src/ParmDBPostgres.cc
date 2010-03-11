//# ParmDBPostgres.cc: 
//#
//# Copyright (C) 2007
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
#include <ParmDB/ParmDBPostgres.h>

#include <Common/lofar_set.h>
#include <Common/LofarLogger.h>

#include <ParmDB/ParmDBPostgresTransactors.h>


//# Skip all code if PostgreSQL is not configured in.
#if defined(HAVE_PGSQL)

namespace LOFAR
{
namespace ParmDB 
{

ParmDBPostgres::ParmDBPostgres(const string &database,
    const string &user,
    const string &password,
    const string &host,
    const string &port)
{
    //# Password not used yet.
    string opts("dbname=" + database  + " user=" + user + " host=" + host);
    if(!port.empty())
        opts += " port=" + port;

//    try
//    {
        LOG_DEBUG_STR("Connecting to database using options: " << opts);
        itsConnection.reset(new pqxx::connection(opts));
//    } CATCH_PQXX_AND_RETHROW;
}


//# Get the domain range (time,freq) of the given parameters in the table.
//# This is the minimum and maximum value of these axes for all parameters.
//# An empty name pattern is the same as * (all parms).
ParmDomain ParmDBPostgres::getRange(const std::string &parmNamePattern) const
{
    ParmDomain result;

    if(parmNamePattern.empty())
        itsConnection->perform(PQGetDomain(translatePattern("*"), result));
    else
        itsConnection->perform(
            PQGetDomain(translatePattern(parmNamePattern), result));
    return result;
}


ParmDomain
    ParmDBPostgres::getRange(const std::vector<std::string> &parmNames) const
{
    ParmDomain result;

    itsConnection->perform(PQGetDomain(parmNames, result));
    return result;
}


//# Get the parameter values for the given parameter and domain.
//# Note that the requested domain may contain multiple values.
//# A selection on parentId is done if >= 0.
ParmValueSet ParmDBPostgres::getValues(const std::string &parmName,
    const ParmDomain &domain,
    int parentId,
    ParmDBRep::TableType)
{
    vector<string> names;
    map<string, ParmValueSet> result;
    
    names.push_back(parmName);
    itsConnection->perform(PQGetValues(names, domain, parentId,
        result));
    
    ASSERT(result.find(parmName) != result.end());
    return result[parmName];
}


//# Get all values for a given domain and set of parm names.
//# If the parm name vector is empty, all parm names are taken.
void ParmDBPostgres::getValues(std::map<std::string,ParmValueSet> &result,
    const std::vector<std::string> &parmNames,
    const ParmDomain &domain,
    int parentId,
    ParmDBRep::TableType)
{
    itsConnection->perform(PQGetValues(parmNames, domain, parentId, 
        result));
}


//# Get the parameter values for the given parameters and domain.
//# Only * and ? should be used in the pattern (no [] and {}).
//# A selection on parentId is done if >= 0.
void ParmDBPostgres::getValues(std::map<std::string,ParmValueSet> &result,
    const std::string &parmNamePattern,
    const ParmDomain &domain,
    int parentId,
    ParmDBRep::TableType)
{
    itsConnection->perform(PQGetValues(translatePattern(parmNamePattern),
            domain, parentId, result));
}

//# Put the value for the given parameter and domain.
//# overwriteMask=true indicates that the solvableMask might be changed
//# and should be overwritten in an existing record.
void ParmDBPostgres::putValue(const std::string &parmName,
    ParmValue& value,
    ParmDBRep::TableType,
    bool overwriteMask)
{
    LOG_DEBUG_STR("itsDBTabRef: " << value.rep().itsDBTabRef);
    
    map<string, ParmValueSet> values;
    values[parmName].getValues().push_back(value);
    itsConnection->perform(PQPutValue(values));
}


//# Put the value for the given parameters and domain.
//# It only writes the parameters that have the same DBSeqNr as this ParmDB.
//# overwriteMask=true indicates that the solvableMask might be changed
//# and should be overwritten in an existing record.
void ParmDBPostgres::putValues(std::map<std::string,ParmValueSet>& values,
    ParmDBRep::TableType,
    bool overwriteMask)
{
    itsConnection->perform(PQPutValue(values));
}


//# Delete the records for the given parameters and domain.
//# If parentId>=0, only records with that parentid will be deleted.
void ParmDBPostgres::deleteValues(const std::string& parmNamePattern,
    const ParmDomain& domain,
    int parentId,
    ParmDBRep::TableType)
{
    itsConnection->perform(
        PQDeleteValues(translatePattern(parmNamePattern), domain, parentId));
}


//# Get the default value for the given parameters.
//# Only * and ? should be used in the pattern (no [] and {}).
void ParmDBPostgres::getDefValues(std::map<std::string,ParmValueSet>& result,
    const std::string& parmNamePattern)
{
    if(parmNamePattern.empty())
        itsConnection->perform(
            PQGetDefaultValues(translatePattern("*"), result));
    else
        itsConnection->perform(
            PQGetDefaultValues(translatePattern(parmNamePattern), result));
}


//# Put the default value.
void ParmDBPostgres::putDefValue(const std::string& parmName,
    const ParmValue& value)
{
    itsConnection->perform(PQPutDefaultValue(parmName, value));
    clearDefFilled();
}


//# Delete the default value records for the given parameters.
void ParmDBPostgres::deleteDefValues(const std::string& parmNamePattern)
{
    itsConnection->perform(
        PQDeleteDefValues(translatePattern(parmNamePattern)));
    clearDefFilled();
}


//# Get the names of all parms matching the given (filename like) pattern.
std::vector<std::string> ParmDBPostgres::getNames(const std::string& pattern,
    ParmDBRep::TableType)
{
    vector<string> result;
    
    itsConnection->perform(PQGetNames(translatePattern(pattern), result));
    return result;
}    


//# Clear database or table
void ParmDBPostgres::clearTables()
{
    //# Not yet implemented.
    ASSERT(false);
}


//# Fill the map with default values.
void ParmDBPostgres::fillDefMap(std::map<std::string,ParmValue>& defMap)
{
    map<string, ParmValueSet> tmp;
    
    this->getDefValues(tmp, "*");
    for(map<string, ParmValueSet>::const_iterator it = tmp.begin();
        it != tmp.end();
        ++it)
    {
        ASSERT(it->second.getValues().size() == 1);
        defMap[it->first] = it->second.getValues()[0];
    }
}
    
//# Function to translate a shell pattern to a Postgres (POSIX) regular 
//# expression. Note that the output is enclosed within a '^', '$' pair, because 
//# normally the Postgres regex matching operator matches the pattern anywhere 
//# in the string. Shell patterns, however, match at the beginning.
string ParmDBPostgres::translatePattern(const string &pattern)
{
    const char regexMeta[14] = {'|', '.', '?', '+', '*', '^', '$', '(', ')', 
        '[', ']', '{', '}', '\\'};
    set<char> regexMetaChars(regexMeta, regexMeta + 14);
    enum State{NORMAL, BRACKET, ESCAPE, ESCAPE_BRACKET};

    ostringstream result;
    result << "^";

    size_t idx = 0;
    size_t bracketPosition;
    uint curlyBraceCount = 0;

    State state = NORMAL;
    while(idx < pattern.size())
    {
        switch(state)
        {
            case NORMAL:
                switch(pattern[idx])
                {
                    //# Translate shell pattern meta characters.
                    case '\\':
                        state = ESCAPE;
                        break;

                    case '*':
                        result << ".*";
                        break;

                    case '?':
                        result << '.';
                        break;

                    case '{':
                        curlyBraceCount++;
                        result << "((";
                        break;

                    case ',':
                        if(curlyBraceCount > 0)
                            result << ")|(";
                        else
                            //# A literal ',' needs no escaping in the output 
                            //# regex.
                            result << ',';
                        break;

                    case '}':
                        if(curlyBraceCount > 0)
                        {
                            curlyBraceCount--;
                            result << "))";
                        }
                        else
                            //# A literal '}' needs to be escaped in the output 
                            //# regex.
                            result << "\\}";
                        break;

                    case '[':
                        bracketPosition = idx;
                        state = BRACKET;
                        result << pattern[idx];
                        break;

                    default:
                        //# The current character is not a shell pattern 
                        //# meta character, so it should be mapped to a non-
                        //# meta character in the output regex.
                        if(regexMetaChars.find(pattern[idx]) !=
                            regexMetaChars.end())
                        {
                            result << '\\';
                        }
                        result << pattern[idx];
                }
                break;

            case BRACKET:
                //# '\' is stripped within brackets. To include a literal ']' 
                //# put it at the first position. To include a literal '-' put 
                //# it first or last.
                switch(pattern[idx])
                {
                    //# Translate shell pattern meta characters.
                    case '\\':
                        state = ESCAPE_BRACKET;
                        break;

                    case '-':
                    case '^':
                        result << pattern[idx];
                        break;

                    case '!':
                        result << ((idx - bracketPosition == 1) ? '^' : '!');
                        break;

                    case ']':
                        //# ']' at the first position matches a literal ']'.
                        if(idx - bracketPosition > 1)
                            state = NORMAL;
                        result << pattern[idx];
                        break;

                    default:
                        //# In the output regex, meta characters lose their 
                        //# meaning inside brackets. Therefore, non-meta 
                        //# characters in the input shell pattern are passed 
                        //# through to the output unaltered.
                        result << pattern[idx];
                }
                break;

            case ESCAPE:
                //# A character following '\' should be interpreted as a 
                //# non-meta character and should therefore map to a non- 
                //# meta character in the output regex.
                if(regexMetaChars.find(pattern[idx]) != 
                    regexMetaChars.end())
                {
                    result << '\\';
                }
                result << pattern[idx];
                state = NORMAL;
                break;

            case ESCAPE_BRACKET:
                //# In the output regex, meta characters lose their meaning 
                //# inside brackets. Specifically, '\' loses its meaning. 
                //# Therefore, escaped characters in the input shell pattern are 
                //# passed through to the output regex without the preceding 
                //# '\'. Note that this implies that for instance [abc\]d] will 
                //# not match a literal ']' but will result in an error.        
                result << pattern[idx];
                state = BRACKET;
                break;
        }
        //# Move to the next input character.
        idx++;
    }
    result << "$";
    return result.str();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
