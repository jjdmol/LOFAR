//#  tAttr1.h: program to test the makeAttrib method(s).
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

#include "tAttr1.h"
#include "PO_tAttr1.h"
#include <PL/PersistenceBroker.h>
#include <PL/Attrib.h>

using namespace LOFAR::PL;
using namespace dtl;
using namespace std;

vector<string> theirAttr;

// Fill the database with values 0 and 1 for each attribute.
void filldb (PersistenceBroker& pb)
{
  F f;
  for (int i0=0; i0<2; i0++) {
    f.bas() = i0;
    for (int i1=0; i1<2; i1++) {
      f.cas() = i1;
      for (int i2=0; i2<2; i2++) {
	f.bs() = i2;
	for (int i3=0; i3<2; i3++) {
	  f.cs() = i3;
	  for (int i4=0; i4<2; i4++) {
	    f.ds() = i4;
	    for (int i5=0; i5<2; i5++) {
	      f.es() = i5;
	      for (int i6=0; i6<2; i6++) {
		f.fs() = i6;
		pb.save (TPersistentObject<F>(f));
	      }
	    }
	  }
	}
      }
    }
  }
}

// Clear the entire result vector.
void clearRes(bool res[128])
{
  for (int i=0; i<128; i++) {
    res[i] = false;
  }
}

// Set the flags where a result is expected.
// Return the nr of flags set.
int orRes(bool res[128], int value1, int ax1, int value2, int ax2)
{
  int nrf=0;
  int inx=0;
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if (i[ax1] == value1  ||  i[ax2] == value2) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int andRes(bool res[128], int value1, int ax1, int value2, int ax2)
{
  int nrf=0;
  int inx=0;
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if (i[ax1] == value1  &&  i[ax2] == value2) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int plusRes(bool res[128], int value, int ax1, int ax2)
{
  int nrf=0;
  int inx=0;
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if (i[ax1] + i[ax2] == value) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int plus2Res(bool res[128], int ax1, int ax2, int ax3)
{
  int nrf=0;
  int inx=0;
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if (i[ax1] + i[ax2] == i[ax3]) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int betweenRes(bool res[128], int value1, int ax1, int value2, int ax2)
{
  int nrf=0;
  int inx=0;
  if (value1 > value2) {
    int val=value1;
    value1=value2;
    value2=val;
  }
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if (value1 <= (i[ax1] - i[ax2]) &&
		    (i[ax1]-i[ax2]) <= value2) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int notBetweenRes(bool res[128], int value1, int ax1, int value2, int ax2)
{
  int nrf=0;
  int inx=0;
  if (value1 > value2) {
    int val=value1;
    value1=value2;
    value2=val;
  }
  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if ((i[ax1] - i[ax2]) < value1 ||
		    (i[ax1] - i[ax2]) > value2) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int inRes(bool res[128], int ax1, int ax2, int value1, 
	  int value2, int value3)
{
  int nrf=0;
  int inx=0;

  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if ((i[ax1] - i[ax2] == value1) ||
		    (i[ax1] - i[ax2] == value2) ||
		    (i[ax1] - i[ax2] == value3) ) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

int notInRes(bool res[128], int ax1, int ax2, int value1, 
	     int value2, int value3)
{
  int nrf=0;
  int inx=0;

  clearRes(res);
  int i[7];
  for (i[0]=0; i[0]<2; i[0]++) {
    for (i[1]=0; i[1]<2; i[1]++) {
      for (i[2]=0; i[2]<2; i[2]++) {
	for (i[3]=0; i[3]<2; i[3]++) {
	  for (i[4]=0; i[4]<2; i[4]++) {
	    for (i[5]=0; i[5]<2; i[5]++) {
	      for (i[6]=0; i[6]<2; i[6]++) {
		if ((i[ax1] - i[ax2] != value1) &&
		    (i[ax1] - i[ax2] != value2) &&
		    (i[ax1] - i[ax2] != value3) ) {
		  res[inx] = true;
		  nrf++;
		}
		inx++;
	      }
	    }
	  }
	}
      }
    }
  }
  return nrf;
}

// Test if the entire result is true.
bool checkRes (const bool exp[128], int nrexp,
	       Collection<TPersistentObject <F> >& set)
{
  // Check if correct nr of objects.
  if (int(set.size()) != nrexp) {
    cerr << "Expected " << nrexp << " matches, found " << set.size() << endl;
    return false;
  }
  cout << "   found " << nrexp << " matches" << endl;
  bool result = true;
  // Clear the result flags.
  bool res[128];
  clearRes(res);
  // Set correct flags in the result to true.
  Collection<TPersistentObject <F> >::iterator iter;
  for (iter=set.begin(); iter!=set.end(); ++iter) {
    F& f = iter->data();
    res[64*f.bas() + 32*f.cas() + 16*f.bs() + 8*f.cs() + 4*f.ds()
	+ 2*f.es() + f.fs()] = true;
  }
  // Check if all flags match the expected ones.
  for (int i=0; i<128; i++) {
    if (res[i] != exp[i]) {
      cerr << "Result mismatch for index " << i << endl;
      result = false;
    }
  }
  return result;
}

// Do a query on a single s value.
bool querySingle (PersistenceBroker& pb, int value, int ax)
{
  cout << "Query: " << theirAttr[ax] << '=' << value << endl;
  // Do a simple query e.s==value and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>(attrib(tpof, theirAttr[ax]) == value);
  // Set the expected result.
  bool exp[128];
  int nrexp = andRes(exp, value, ax, value, ax);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

// Do a query with an or of two attributes.
bool queryOr (PersistenceBroker& pb, int value1, int ax1,
	      int value2, int ax2)
{
  cout << "Query: " << theirAttr[ax1] << '=' << value1 << " || "
       << theirAttr[ax2] << '=' << value2 << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>(attrib(tpof, theirAttr[ax1]) == value1 ||
		   attrib(tpof, theirAttr[ax2]) == value2);
  // Set the expected result.
  bool exp[128];
  clearRes(exp);
  int nrexp = orRes (exp, value1, ax1, value2, ax2);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

// Do a query with an and of two attributes.
bool queryAnd (PersistenceBroker& pb, int value1, int ax1,
	       int value2, int ax2)
{
  cout << "Query: " << theirAttr[ax1] << '=' << value1 << " && "
       << theirAttr[ax2] << '=' << value2 << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>(attrib(tpof, theirAttr[ax1]) == value1 &&
		   attrib(tpof, theirAttr[ax2]) == value2);
  // Set the expected result.
  bool exp[128];
  int nrexp = andRes (exp, value1, ax1, value2, ax2);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryPlus (PersistenceBroker& pb, int value, int ax1, int ax2)
{
  cout << "Query: " << theirAttr[ax1] << '+'
       << theirAttr[ax2] << '=' << value << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>(attrib(tpof, theirAttr[ax1]) +
		   attrib(tpof, theirAttr[ax2]) == value);
  // Set the expected result.
  bool exp[128];
  int nrexp = plusRes (exp, value, ax1, ax2);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryPlus2 (PersistenceBroker& pb, int ax1, int ax2, int ax3)
{
  cout << "Query: " << theirAttr[ax1] << '+'
       << theirAttr[ax2] << '=' << theirAttr[ax3] << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>(attrib(tpof, theirAttr[ax1]) +
		   attrib(tpof, theirAttr[ax2]) ==
		   attrib(tpof, theirAttr[ax3]));
  // Set the expected result.
  bool exp[128];
  int nrexp = plus2Res (exp, ax1, ax2, ax3);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryBetween (PersistenceBroker& pb, int value1, int ax1, 
		   int value2, int ax2)
{
  if (value1 > value2) {
    int val=value1;
    value1=value2;
    value2=val;
  }

  cout << "Query: " << theirAttr[ax1] << "-"
       << theirAttr[ax2] << " between " << value1  
       << " and " << value2 << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>((attrib(tpof, theirAttr[ax1]) -
		    attrib(tpof, theirAttr[ax2]))  
		   .between(value1,value2));
  // Set the expected result.
  bool exp[128];
  int nrexp = betweenRes (exp, value1, ax1, value2, ax2);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryNotBetween (PersistenceBroker& pb, int value1, int ax1, 
		      int value2, int ax2)
{
  if (value1 > value2) {
    int val=value1;
    value1=value2;
    value2=val;
  }
  cout << "Query: " << theirAttr[ax1] << "-"
       << theirAttr[ax2] << " not between " << value1  
       << " and " << value2 << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>((attrib(tpof, theirAttr[ax1]) -
		    attrib(tpof, theirAttr[ax2]))  
		   .notBetween(value1,value2));
  // Set the expected result.
  bool exp[128];
  int nrexp = notBetweenRes (exp, value1, ax1, value2, ax2);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryIn (PersistenceBroker& pb, int ax1, int ax2, int value1, 
	      int value2, int value3)
{

  Collection<Query::Expr> c;
  c.add(value1);
  c.add(value2);
  c.add(value3);


  cout << "Query: " << theirAttr[ax1] << "-"
       << theirAttr[ax2] << " in (" << value1 << ", " 
       << value2 << ", " << value3 << ")" << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>((attrib(tpof, theirAttr[ax1]) -
		    attrib(tpof, theirAttr[ax2]))  
		   .in (c));
  // Set the expected result.
  bool exp[128];
  int nrexp = inRes (exp, ax1, ax2, value1, value2, value3);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

bool queryNotIn (PersistenceBroker& pb, int ax1, int ax2, 
		 int value1, int value2, int value3)
{

  Collection<Query::Expr> c;
  c.add(value1);
  c.add(value2);
  c.add(value3);


  cout << "Query: " << theirAttr[ax1] << "-"
       << theirAttr[ax2] << " not in (" << value1 << ", " 
       << value2 << ", " << value3 << ")" << endl;
  // Do the query and check the result.
  TPersistentObject<F> tpof;
  Collection<TPersistentObject <F> > set =
    pb.retrieve<F>((attrib(tpof, theirAttr[ax1]) -
		    attrib(tpof, theirAttr[ax2]))  
		   .notIn (c));
  // Set the expected result.
  bool exp[128];
  int nrexp = notInRes (exp, ax1, ax2, value1, value2, value3);
  // Check the result.
  return checkRes(exp, nrexp, set);
}

int main(int argc, const char* argv[])
{
  bool flag=true;
  try {
    Debug::initLevels(argc, argv);

    // Define the attribute names (in this order).
    theirAttr.push_back ("C::b.A::s");
    theirAttr.push_back ("C::A::s");
    theirAttr.push_back ("C::b.s");
    theirAttr.push_back ("C::s");
    theirAttr.push_back ("e.d.s");
    theirAttr.push_back ("e.s");
    theirAttr.push_back ("s");
    const int ATTR0 = 0;
    const int ATTR1 = 1;
    const int ATTR2 = 2;
    const int ATTR3 = 3;
    const int ATTR4 = 4;
    const int ATTR5 = 5;
    const int ATTR6 = 6;

    // Open the connection.
    PersistenceBroker pb;
    pb.connect("test","postgres","");
    // Skip filldb (which takes some time) if indicated.
    if (argc > 1  &&  std::string(argv[1]) == "1") {
      filldb (pb);
    }
    cout << endl << "Testing single Query: " << endl;
    flag &= querySingle (pb, 0, ATTR6);
    flag &= querySingle (pb, 0, ATTR5);
    flag &= querySingle (pb, 1, ATTR4);
    flag &= querySingle (pb, 0, ATTR3);
    flag &= querySingle (pb, 0, ATTR2);
    flag &= querySingle (pb, 0, ATTR1);
    flag &= querySingle (pb, 0, ATTR0);
    flag &= querySingle (pb, 2, ATTR0);

    cout << endl << "Testing AND: " << endl;
    flag &= queryAnd (pb, 0, ATTR3, 1, ATTR5);
    flag &= queryAnd (pb, 0, ATTR0, 1, ATTR4);

    cout << endl << "Testing +: " << endl;
    flag &= queryPlus (pb, 1, ATTR0, ATTR5);
    flag &= queryPlus (pb, 2, ATTR1, ATTR3);

    cout << endl << "Testing +2: " << endl;
    flag &= queryPlus2 (pb, ATTR2, ATTR1, ATTR3);

    cout << endl << "Testing OR: " << endl;
    flag &= queryOr (pb, 0, ATTR6, 1, ATTR6);
    flag &= queryOr (pb, 0, ATTR5, 1, ATTR5);
    flag &= queryOr (pb, 0, ATTR0, 1, ATTR0);
    flag &= queryOr (pb, 0, ATTR6, 0, ATTR5);
    flag &= queryOr (pb, 0, ATTR5, 0, ATTR1);
    flag &= queryOr (pb, 1, ATTR4, 0, ATTR0);

    cout << endl << "Testing BETWEEN: " << endl;
    flag &= queryBetween (pb, -3, ATTR0, 5, ATTR5); // 128
    flag &= queryBetween (pb, 0, ATTR2, 0, ATTR1);  // 64
    flag &= queryBetween (pb, 1, ATTR1, 2 , ATTR3); // 32
    flag &= queryBetween (pb, 4, ATTR3, 8 , ATTR6); // 0

    cout << endl << "Testing NOT BETWEEN: " << endl;
    flag &= queryNotBetween (pb, 3, ATTR6, 8, ATTR1);   // 128
    flag &= queryNotBetween (pb, 1, ATTR2, 16 , ATTR5); // 96 
    flag &= queryNotBetween (pb, -8, ATTR4, 0, ATTR1);  // 32
    flag &= queryNotBetween (pb, -1, ATTR1, 1, ATTR6);  // 0

    cout << endl << "Testing IN: " << endl;
    flag &= queryIn (pb, ATTR5, ATTR0, -1,0,1);      // 128
    flag &= queryIn (pb, ATTR5, ATTR4, 1  ,-1 ,-5);  // 64
    flag &= queryIn (pb, ATTR1, ATTR6,-1 ,-7 ,13);   // 32
    flag &= queryIn (pb, ATTR2, ATTR3,-9 ,-123 ,13); // 0

    cout << endl << "Testing NOT IN: " << endl;
    flag &= queryNotIn (pb, ATTR2, ATTR2, 4 ,6 ,11); //128
    flag &= queryNotIn (pb, ATTR5, ATTR0, -3,0,6);   // 64
    flag &= queryNotIn (pb, ATTR5, ATTR0, 1,0,6);   // 32
    flag &= queryNotIn (pb, ATTR5, ATTR1,-1 ,0 ,1);  // 0

  } catch (std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }
  // Return with correct status.
  return flag ? 0:1;
}
