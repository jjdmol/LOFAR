//# ParmDBPostgresTransactors.cc: 
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
#include <ParmDB/ParmDBPostgresTransactors.h>
#include <ParmDB/ParmDBPostgres.h>

//# Skip all code if PostgreSQL is not configured in.
#if defined(HAVE_PGSQL)

namespace LOFAR
{
namespace ParmDB 
{

ParmDomain convert_domain(pqxx::result::field in)
{
    istringstream iss(in.as<string>());

    istringstream::char_type ign;
    double xs, xe, ys, ye;

    LOG_DEBUG_STR("convert_domain input: " << in.as<string>());
    
    //# TODO: syntax/ioflag checking!
    //iss >> ign; //# '('
    iss >> ign; //# '('
    iss >> xe;
    iss >> ign; //# ','
    iss >> ye;
    iss >> ign; //# ')'
    iss >> ign; //# ','
    iss >> ign; //# '('
    iss >> xs;
    iss >> ign; //# ','
    iss >> ys;

    return ParmDomain(xs, xe, ys, ye);
}

string convert_domain(const ParmDomain &domain)
{
    const vector<double> &start = domain.getStart();
    const vector<double> &end = domain.getEnd();

    ASSERTSTR(start.size() == 2 && end.size() == 2,
        "Only 2-D domains are currently supported.");

    ostringstream result;
    result << "((" << end[0] << "," << end[1] << "),"
        "(" << start[0] << "," << start[1] << "))";

    return result.str();
}

PQGetDomain::PQGetDomain(const string &regex, ParmDomain &domain)
    :   itsRegex(regex),
        itsDomain(domain)
{
}

PQGetDomain::PQGetDomain(const vector<string> &names, ParmDomain &domain)
    :   itsNames(names),
        itsDomain(domain)
{
}

void PQGetDomain::operator()(argument_type& transaction)
{
    ostringstream query;
    
    //# Postgres stores a box as ((right upper corner), (left lower corner)).
    query << "SELECT "
    "min((domain[1])[0]) AS xs,"
    "min((domain[1])[1]) AS ys,"
    "max((domain[0])[0]) AS xe,"
    "max((domain[0])[1]) AS ye "
    "FROM xp.parameter";

    if(!itsRegex.empty())
        query << " WHERE name ~ '" + transaction.esc(itsRegex) + "'";
    else if(!itsNames.empty())
    {
        query << " WHERE name IN (";
        for(vector<string>::const_iterator it = itsNames.begin();
            it != itsNames.end();
            ++it)
        {
            query << "'" + transaction.esc(*it) + "'";
        }
        query << ")";
    }
    query << ";";

    LOG_DEBUG_STR("query: " << query.str());
    itsPQResult = transaction.exec(query.str());
}

void PQGetDomain::on_commit()
{
    //# Aggregate functions always return a single row.
    ASSERT(itsPQResult.size() == 1);

    try
    {
        itsDomain = ParmDomain(itsPQResult.front()["xs"].as<double>(),
            itsPQResult.front()["xe"].as<double>(),
            itsPQResult.front()["ys"].as<double>(),
            itsPQResult.front()["ye"].as<double>());
    }
    catch(PGSTD::domain_error &ex)
    {
        itsDomain = ParmDomain(0.0, 1.0, 0.0, 1.0);
    }
}


PQPutValue::PQPutValue(const map<string, ParmValueSet> &values)
    : itsValues(values)
{
}

void PQPutValue::operator()(argument_type& transaction)
{
    for(map<string, ParmValueSet>::const_iterator map_it = itsValues.begin();
        map_it != itsValues.end();
        ++map_it)
    {
        const string &name = map_it->first;
        const vector<ParmValue> &set = map_it->second.getValues();
        
        for(vector<ParmValue>::const_iterator set_it = set.begin();
            set_it != set.end();
            ++set_it)
        {
            const ParmValueRep &value = set_it->rep();

            ostringstream query;
            query << "SELECT xp.put_value";
            query << "(ROW(";
            query << "'" << transaction.esc(name) << "',";
            query << "'" << transaction.esc(value.itsType) << "',";
            query << "'" << transaction.esc(value.itsExpr) << "',";
            if(!value.itsConstants.empty())
                query << "'" << pack_vector(transaction, value.itsConstants)
                    << "',";
            else
                query << "NULL,";

            query << "'" << pack_vector(transaction, value.itsShape) << "',";
            if(!value.itsSolvMask.empty())
                query << "'" << pack_vector(transaction, value.itsSolvMask)
                    << "',";
            else
                query << "NULL,";

            query << value.itsPerturbation << ",";
            query << (value.itsIsRelPert ? "true" : "false") << ",";
            query << "'" << convert_domain(value.itsDomain) << "',";
            query << "'" << pack_vector(transaction, value.itsCoeff) << "'";
            query << "));";

            LOG_DEBUG_STR("query: " << query.str());
            transaction.exec(query.str());
        }
    }
}

//void PQPutValue::on_commit();


PQGetValues::PQGetValues(const string &regex,
    const ParmDomain &domain,
    int parentId,
    map<string, ParmValueSet> &result)
    :   itsRegex(regex),
        itsDomain(domain),
        itsParentId(parentId),
        itsResult(result)
{
}

PQGetValues::PQGetValues(const vector<string> &names,
    const ParmDomain &domain,
    int parentId,
    map<string, ParmValueSet> &result)
    :   itsNames(names),
        itsDomain(domain),
        itsParentId(parentId),
        itsResult(result)
{
}

void PQGetValues::operator()(argument_type& transaction)
{
    LOG_DEBUG_STR("regex: " << itsRegex);
    LOG_DEBUG_STR("quoted regex: " << transaction.esc(itsRegex));

    ostringstream query;
    query << "SELECT * FROM xp.parameter";
    
    if(!itsRegex.empty())
        query << " WHERE name ~ '" << transaction.esc(itsRegex) << "'";
    else if(itsNames.size() == 1)
        query << " WHERE name = '" << transaction.esc(itsNames.front()) << "'";
    else if(itsNames.size() > 1)
    {
        query << " WHERE name IN(";
        for(vector<string>::const_iterator it = itsNames.begin();
            it != itsNames.end();
            ++it)
        {
            query << "'" + transaction.esc(*it) + "'";
        }
        query << ")";
    }
    
    const vector<double> &start = itsDomain.getStart();
    const vector<double> &end = itsDomain.getEnd();
    if(start.size() == 2 && end.size() == 2)
        query << " AND domain && " << "'" << convert_domain(itsDomain) << "'";
    else
        ASSERTSTR(start.empty() && end.empty(),
            "Only 2-D domains are currently supported.");

//    if(itsParentId >= 0)
//        query << " AND parent_id = " << itsParentId;
    query << ";";

    LOG_DEBUG_STR("query: " << query.str());
    itsPQResult = transaction.exec(query.str());
}

void PQGetValues::on_commit()
{
    for(pqxx::result::const_iterator it = itsPQResult.begin();
        it != itsPQResult.end();
        ++it)
    {
        ParmValueSet &set = itsResult[(*it)["name"].as<string>()];
        set.getValues().push_back(process(*it));
    }
}

ParmValue PQGetValues::process(pqxx::result::tuple row)
{
    ParmValue resultPointer;
    ParmValueRep &result = resultPointer.rep();

    if(!row["constants"].is_null())
    {
        vector<double> constants =
            unpack_vector<double>(pqxx::binarystring(row["constants"]));
        result.setType(row["type"].as<string>(), constants);
    }
    else
        result.setType(row["type"].as<string>());

    result.itsExpr      = row["expression"].as<string>();

    vector<int> shape = unpack_vector<int>(pqxx::binarystring(row["shape"]));
    vector<double> coefficients =
        unpack_vector<double>(pqxx::binarystring(row["coefficients"]));

    if(!row["mask"].is_null())
    {
        vector<bool> mask =
            unpack_vector<bool>(pqxx::binarystring(row["mask"]));
        bool tmp[mask.size()];
        for(size_t i = 0; i < mask.size(); ++i)
            tmp[i] = mask[i];

        result.setCoeff(&coefficients[0], tmp, shape);
    }
    else
        result.setCoeff(&coefficients[0], shape);

    result.setDomain(convert_domain(row["domain"]));
    result.setPerturbation(row["perturbation"].as<double>(),
        row["pert_rel"].as<bool>());

    result.itsWeight    = 0;
    result.itsID        = row["id"].as<int>();
    result.itsParentID  = 0;
    result.itsTimeStamp = 0;
    result.itsDBTabRef  = 0;
    result.itsDBSeqNr   = 0;

    return resultPointer;
}


PQPutDefaultValue::PQPutDefaultValue(const string &name, const ParmValue value)
    :   itsName(name),
        itsValue(value)
{
}

void PQPutDefaultValue::operator()(argument_type& transaction)
{
    ostringstream query;
    const ParmValueRep &value = itsValue.rep();

    query << "SELECT xp.put_default_value(";
    query << "ROW(";
    query << "'" << transaction.esc(itsName) << "',";
    query << "'" << transaction.esc(value.itsType) << "',";
    query << "'" << transaction.esc(value.itsExpr) << "',";
    if(!value.itsConstants.empty())
        query << "'" << pack_vector(transaction, value.itsConstants) << "',";
    else
        query << "NULL,";

    query << "'" << pack_vector(transaction, value.itsShape) << "',";
    if(!value.itsSolvMask.empty())
        query << "'" << pack_vector(transaction, value.itsSolvMask) << "',";
    else
        query << "NULL,";

    query << "'" << pack_vector(transaction, value.itsCoeff) << "',";
    query << value.itsPerturbation << ",";
    query << (value.itsIsRelPert ? "true" : "false");
    query << "));";

    LOG_DEBUG_STR("query: " << query.str());

    transaction.exec(query.str());
}

PQGetDefaultValues::PQGetDefaultValues(const string &regex,
    map<string, ParmValueSet> &result)
    :   itsRegex(regex),
        itsResult(result)
{
}

void PQGetDefaultValues::operator()(argument_type& transaction)
{
    LOG_DEBUG_STR("regex: " << itsRegex);
    LOG_DEBUG_STR("quoted regex: " << transaction.esc(itsRegex));
    itsPQResult = transaction.exec("SELECT * FROM xp.default_parameter"
        " WHERE name ~ '" + transaction.esc(itsRegex) + "';");
}

void PQGetDefaultValues::on_commit()
{
    for(pqxx::result::const_iterator it = itsPQResult.begin();
        it != itsPQResult.end();
        ++it)
    {
        ParmValueSet tmp;
        tmp.getValues().push_back(process(*it));
        itsResult[(*it)["name"].as<string>()] = tmp;
    }
}

ParmValue PQGetDefaultValues::process(pqxx::result::tuple row)
{
    ParmValue resultPointer;
    ParmValueRep &result = resultPointer.rep();

    if(!row["constants"].is_null())
    {
        vector<double> constants =
            unpack_vector<double>(pqxx::binarystring(row["constants"]));
        result.setType(row["type"].as<string>(), constants);
    }
    else
        result.setType(row["type"].as<string>());

    result.itsExpr      = row["expression"].as<string>();

    vector<int> shape = unpack_vector<int>(pqxx::binarystring(row["shape"]));
    vector<double> coefficients =
        unpack_vector<double>(pqxx::binarystring(row["coefficients"]));

    if(!row["mask"].is_null())
    {
        vector<bool> mask =
            unpack_vector<bool>(pqxx::binarystring(row["mask"]));
        bool tmp[mask.size()];
        for(size_t i = 0; i < mask.size(); ++i)
            tmp[i] = mask[i];

        result.setCoeff(&coefficients[0], tmp, shape);
    }
    else
        result.setCoeff(&coefficients[0], shape);

    result.setPerturbation(row["perturbation"].as<double>(),
        row["pert_rel"].as<bool>());

    result.itsWeight    = 0;
    result.itsID        = row["id"].as<int>();
    result.itsParentID  = 0;
    result.itsTimeStamp = 0;
    result.itsDBTabRef  = 0;
    result.itsDBSeqNr   = 0;

    return resultPointer;
}


PQGetNames::PQGetNames(const string &regex, vector<string> &result)
    :   itsRegex(regex),
        itsResult(result)
{
}

void PQGetNames::operator()(argument_type& transaction)
{
    LOG_DEBUG_STR("regex: " << itsRegex);
    LOG_DEBUG_STR("quoted regex: " << transaction.esc(itsRegex));
    itsPQResult = transaction.exec("SELECT DISTINCT name FROM xp.parameter"
        " WHERE name ~ '" + transaction.esc(itsRegex) + "';");
}

void PQGetNames::on_commit()
{
    for(pqxx::result::const_iterator it = itsPQResult.begin();
        it != itsPQResult.end();
        ++it)
    {
        itsResult.push_back((*it)["name"].as<string>());
    }
}

PQDeleteValues::PQDeleteValues(const string &regex,
    const ParmDomain &domain,
    int parentId)
    :   itsRegex(regex),
        itsDomain(domain),
        itsParentId(parentId)
{
}
    
void PQDeleteValues::operator()(argument_type& transaction)
{
    LOG_DEBUG_STR("regex: " << itsRegex);
    LOG_DEBUG_STR("quoted regex: " << transaction.esc(itsRegex));
    
    ostringstream query;
//    query << "LOCK TABLE xp.parameter;";
    query << "DELETE FROM xp.parameter"
    " WHERE name ~ '" << transaction.esc(itsRegex) << "'";
    
    // TODO: should we delete all parms intersecting the domain
    // or all that fall completely within the domain, or just the ones
    // that exactly match the domain?
    const vector<double> &start = itsDomain.getStart();
    const vector<double> &end = itsDomain.getEnd();
    if(start.size() == 2 && end.size() == 2)
        query << " AND domain ~= " << "'" << convert_domain(itsDomain) << "'";
    else
        ASSERTSTR(start.empty() && end.empty(),
            "Only 2-D domains are currently supported.");
    
//    if(itsParentId >= 0)
//        query << " AND parent_id = " << itsParentId;
    query << ";";
    transaction.exec(query.str());
}

PQDeleteDefValues::PQDeleteDefValues(const string &regex)
    : itsRegex(regex)
{
}

void PQDeleteDefValues::operator()(argument_type& transaction)
{
    LOG_DEBUG_STR("regex: " << itsRegex);
    LOG_DEBUG_STR("quoted regex: " << transaction.esc(itsRegex));
    
    ostringstream query;
//    query << "LOCK TABLE xp.default_parameter;";
    query << "DELETE FROM xp.default_parameter"
    " WHERE name ~ '" << transaction.esc(itsRegex) << "';";
    transaction.exec(query.str());
}

} //# namespace BBS
} //# namespace LOFAR

#endif
