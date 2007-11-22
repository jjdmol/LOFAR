//#  Transport.h: declaration of some transportation classes.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_PL_TECHFLYER_TRANSPORT_H
#define LOFAR_PL_TECHFLYER_TRANSPORT_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PL/PLfwd.h>
#include <iosfwd>
#include <string>

// @defgroup DemoTransport Transport demo
// @ingroup DemoClasses
//
// This collection of classes define some means of transportation. The classes
// are used in an example to demonstrate the use of the LOFAR Persistence
// Layer. They both demonstrate the use of inheritance and composition.

//@{

class Engine 
{
public:
  Engine () : itsVolume (1.2) {}
  Engine (float vol) : itsVolume (vol) {}
  virtual ~Engine() {}
  virtual void print (std::ostream& os) const;
private:
  friend class LOFAR::PL::TPersistentObject<Engine>;
  float itsVolume;
};

class GasEngine : public Engine
{
public:
  GasEngine () : Engine (), itsValveCount (16) {}
  GasEngine (int valve, float vol) : 
    Engine (vol), itsValveCount (valve) {}
  virtual ~GasEngine() {}
  void setValveCount(int valves);
  virtual void print (std::ostream& os) const;
private:
  friend class LOFAR::PL::TPersistentObject<GasEngine>;
  int itsValveCount;
};

class DieselEngine : public Engine 
{
public:
  DieselEngine () : Engine (), itsTurboCharged (false) {}
  DieselEngine (bool chrg, float vol) : 
    Engine (vol), itsTurboCharged (chrg) {}
  virtual ~DieselEngine() {}
  virtual void print (std::ostream& os) const;
private:
  friend class LOFAR::PL::TPersistentObject<DieselEngine>;
  bool itsTurboCharged;
};

class Vehicle
{
public:
  Vehicle () : itsWheelCount (0), itsMake ("Unk") {}
  Vehicle (int wc, const std::string& make) : 
    itsWheelCount (wc), itsMake (make) {}
  virtual ~Vehicle() {}
  virtual void print (std::ostream& os) const;
  std::string getMake() const;
private:
  friend class LOFAR::PL::TPersistentObject<Vehicle>;
  int    itsWheelCount;
  std::string itsMake;
};

class MotorCycle : public Vehicle 
{
public:
  MotorCycle () :
    Vehicle (2, "unkMotorCycle"), itsEngine (20, 2.0) {}
  MotorCycle (const std::string& make, int valves, float vol) :
    Vehicle (2, make), itsEngine (valves, vol) {}
  virtual ~MotorCycle() {}
  void setValveCount(int valves);
  virtual void print (std::ostream& os) const;
private:
  friend class LOFAR::PL::TPersistentObject<MotorCycle>;
  GasEngine itsEngine;
};

class Tractor : public Vehicle 
{
public:
  Tractor () : 
    Vehicle (4, "unkTractor"), itsEngine (true, 4.0) {}
  Tractor (const std::string& make, bool turbo, float vol) : 
    Vehicle (4, make), itsEngine (turbo, vol) {}
  virtual ~Tractor() {}
  virtual void print (std::ostream& os) const;
private:
  friend class LOFAR::PL::TPersistentObject<Tractor>;
  DieselEngine itsEngine;
};


inline std::ostream& operator<<(std::ostream& os, const Engine& e)
{
  e.print(os);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vehicle& v)
{
  v.print(os);
  return os;
}

//@}

#endif

