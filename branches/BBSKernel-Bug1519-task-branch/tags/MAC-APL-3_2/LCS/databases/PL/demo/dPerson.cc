//#  dPerson.cc: demo program showing how to use the Persistence Layer.
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

// \file dPerson.cc
// Demo program that shows you how to use the Persistence Layer.

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "Person.h"
#include "PO_Person.h"
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Attrib.h>
#include <PL/Query/Expr.h>
#include <iostream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::PL;    // All classes of PL reside in this namespace.

int main(int argc, const char* argv[])
{
  // Initialize the logger
  INIT_LOGGER(argv[0]);

  // Create a vector of Person objects and add some objects.
  vector<Person> vp;
  vp.push_back(Person("Marcel", "loose@astron.nl",    39, 0));
  vp.push_back(Person("Jun",    "tanaka@astron.nl",   30, 0));
  vp.push_back(Person("Ellen",  "meijeren@astron.nl", 25, 1));
  vp.push_back(Person("Ger",    "diepen@astron.nl",   49, 0));

  // Show the contents of our Person vector.
  cout << endl << "Here are our Person objects:" << endl;
  for (unsigned i = 0; i < vp.size(); ++i) {
    cout << vp[i] << endl;
  }

  // Create a PersistenceBroker object. We will communicate with the database
  // through this broker object.
  PersistenceBroker broker;

  // Connect to the database. 
  try {

    // The connect() method takes three arguments:
    // \a aDsn: the ODBC Data Source Name; it is a logical name for a
    // database, e.g. \c "test".
    // \a aUid: the user-id that will be used to connect to the database,
    // e.g. \c "postgres".
    // \a aPwd: the password associated with the user-id, e.g. blank.
    // \note Only the first argument is mandatory.
    broker.connect("test","postgres");
  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }
  

  // Save our vector of persons.
  try {

    cout << endl << "Save them to the database ...  ";
    // We'll save the vector element by element.
    for (unsigned i = 0; i < vp.size(); ++i) {

      // Put our Person objects in a "TPO" container class. The "TPO" knows
      // how the contained object (our Person object) must be stored into and
      // retrieved from the database.
      TPersistentObject<Person> tpo(vp[i]);

      // Save our Person object to the database, using the "TPO" container.
      broker.save(tpo);
    }
    cout << "OK" << endl;
  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }


  // Let's try to retrieve some Person objects, using a query.
  try {

    // The result of a query is returned in a Collection of "TPOs".
    Collection< TPersistentObject<Person> > ctpo;

    cout << endl << "Let's try to make some queries..." << endl;

    cout << endl << "Show all persons that are over 30 years:" << endl;

    // Construct a query by specifying conditions for the class attributes.
    // E.g.: retrieve all persons that are older than 30 years.
    Query::Expr exp = attrib<Person>("age") > 30;

    // Make the query; store the result in the Collection of TPOs
    ctpo = broker.retrieve<Person>(exp);

    // Show the result on standard output, iterating over the collection
    Collection< TPersistentObject<Person> >::const_iterator cit;
    for (cit = ctpo.begin(); cit != ctpo.end(); ++cit) {
      cout << cit->data() << endl;
    }

    cout << endl << "Ger is already 50 years. Let's update the object ...  ";

    // Let's update the correct object in the Collection by matching the name.
    Collection< TPersistentObject<Person> >::iterator it; 
    for (it = ctpo.begin(); it != ctpo.end(); ++it) {
      if (it->data().getName() == "Ger") {
        it->data().setAge(50);
        broker.save(*it);
      }
    }
    cout << "OK" << endl;

    cout << endl << "Show all male persons:" << endl;

    // Construct the query by specifying the conditions.
    exp = attrib<Person>("gender") == 0; // remember MALE == 0

    // Make the query; store the result in the Collection of TPOs
    ctpo = broker.retrieve<Person>(exp);

    // Show the result on standard output, iterating over the collection
    for (cit = ctpo.begin(); cit != ctpo.end(); ++cit) {
      cout << cit->data() << endl;
    }

  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
