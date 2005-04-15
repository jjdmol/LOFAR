//#  Person.h: one line description
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

#ifndef LOFAR_PL_DEMO_PERSON_H
#define LOFAR_PL_DEMO_PERSON_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PL/PLfwd.h>  // forward declarations for often used PL types.
#include <string>
#include <iosfwd>      // forward declarations for iostreams.

// @defgroup DemoClasses Demo classes
// This group of classes is used in some examples that demonstrate the use of
// the LOFAR Persistence Layer.  

// @defgroup DemoPerson Person demo
// @ingroup DemoClasses

//@{

// This is a simple class that will be used to demonstrate the use of the
// LOFAR Persistence Layer. It is simple in the sense that Person does not
// contain data members of other (user-defined) class types.
class Person {
public:
  // Default constructor.
  // It is good practice to provide a default constructor; you will need to
  // have one when e.g. you want to create an array of objects.
  // \note The Persistence Layer requires that a user-defined class is default
  // constructable. 
  Person ();

  // Construct a new person object.
  Person (const std::string& aName, const std::string& anAddress, 
          int anAge, int aGender);

  // Return itsName
  std::string getName() const;

  // Set the itsAge equal to \a anAge.
  void setAge(int anAge);

  // Print the contents of this object into the output stream \a os.
  void print (std::ostream& os) const;

private:
  // We need this friendship if we want to make our object persistent;
  // TPersistentObject<Person> needs to have access to our private
  // parts. Another solution would be to provide accessor methods for all our
  // private data members. Personally, I prefer this approach.
  friend class LOFAR::PL::TPersistentObject<Person>;

  // The name as a single string.
  std::string itsName;

  // The address as a single string.
  std::string itsAddress;

  // \note In normal situations you would record the date-of-birth, instead of
  // the person's age. However, for this simple example class, using the age
  // suffices.
  int    itsAge;

  // \note itsGender should actually be an \c enum. However, \c enums are not
  // supported by ordinary relational databases. Hence, we will use an \c int.
  //
  // Male == 0;  Female == 1.
  int    itsGender;
};

// Print the contents of a Person objects into the output stream \a os.
std::ostream& operator<<(std::ostream& os, const Person& p);

//@}

#endif
