//# ModelConfig.h: Aggregation of all the model configuration options.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_MODELCONFIG_H
#define LOFAR_BBSKERNEL_MODELCONFIG_H

// \file
// Aggregation of all the model configuration options.

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// @{
    
class IonoConfig
{
public:
    typedef shared_ptr<IonoConfig>        Pointer;
    typedef shared_ptr<const IonoConfig>  ConstPointer;

    IonoConfig();
    virtual ~IonoConfig();

    // Print the contents of \c *this in human readable form into the output
    // stream \a out.
    void print(ostream &out) const;

    uint32 rank;
};

class BeamConfig
{
public:
    typedef shared_ptr<BeamConfig>        Pointer;
    typedef shared_ptr<const BeamConfig>  ConstPointer;

    BeamConfig();
    virtual ~BeamConfig();

    // Print the contents of \c *this in human readable form into the output
    // stream \a out.
    virtual void print(ostream &out) const = 0;
    
    virtual const string &type() const = 0;
};


class HamakerDipoleConfig: public BeamConfig
{
public:
    typedef shared_ptr<HamakerDipoleConfig>       Pointer;
    typedef shared_ptr<const HamakerDipoleConfig> ConstPointer;

    HamakerDipoleConfig();
    ~HamakerDipoleConfig();
    
    // Print the contents of \c *this in human readable form into the output
    // stream \a out.
    void print(ostream &out) const;
    const string &type() const;

    string  coeffFile;
};


class YatawattaDipoleConfig: public BeamConfig
{
public:
    typedef shared_ptr<YatawattaDipoleConfig>       Pointer;
    typedef shared_ptr<const YatawattaDipoleConfig> ConstPointer;

    YatawattaDipoleConfig();
    ~YatawattaDipoleConfig();
    
    // Print the contents of \c *this in human readable form into the output
    // stream \a out.
    void print(ostream &out) const;
    const string &type() const;
    
    string  moduleTheta;
    string  modulePhi;
};


class ModelConfig
{
public:
    ModelConfig();
    ~ModelConfig();
    
    bool                      usePhasors;
    vector<string>            sources;
    vector<string>            components;
    IonoConfig::ConstPointer  ionoConfig;
    BeamConfig::ConstPointer  beamConfig;
};

ostream &operator<<(ostream&, const ModelConfig&);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
