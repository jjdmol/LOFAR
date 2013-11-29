//# GenericEventWrapper.h: Interface definition class for a generic event
//#
//# Copyright (C) 2011
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

#ifndef PIPELINE_GENERICEVENTWRAPPER_H
#define PIPELINE_GENERICEVENTWRAPPER_H

// \file GenericEventWrapper.h
// Interface definition class for a generic event

#include <MACIO/GCF_Event.h>

// \addtogroup Pipeline
// @{

// Interface definition class for a generic event
class GenericEventWrapper {
private:
    LOFAR::MACIO::GCFEvent* my_event;
public:
    GenericEventWrapper() {
        this->my_event = new LOFAR::MACIO::GCFEvent;
    }
    GenericEventWrapper(LOFAR::MACIO::GCFEvent* event_ptr) {
        this->my_event = event_ptr;
    }
    virtual LOFAR::TYPES::uint16 get_signal() { return this->my_event->signal; }
    virtual LOFAR::MACIO::GCFEvent* get_event_ptr() { return my_event;}
};

// @}

#endif
