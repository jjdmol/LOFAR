// ParamValue.h: Class to hold a general value
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.1.1.1  2003/02/21 11:14:36  schaaf
//  copy from BaseSim tag "CEPFRAME"
//
//  Revision 1.6  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.5  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.4  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.3  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.2  2001/09/04 10:08:57  gvd
//  Added support for vectors
//
//  Revision 1.1  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_PARAMVALUE_H
#define BASESIM_PARAMVALUE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include <Common/lofar_complex.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{

//# Forward Declarations
class ParamBlock;

/**
   The ParamHolder class is the interface to a set of parameters
   defined in its sub-class DataType.
   In the LOFARSim framework, an instance of this class is added to
   Data objects. Therefore, each WorkHolder in the framework can
   access the parameters.
*/

class ParamValue
{
public:
  enum DataType {DTBool, DTInt, DTFloat, DTDouble,
		 DTComplex, DTDComplex, DTString,
		 DTBlock,
		 DTValueVector,
		 DTVecBool, DTVecInt, DTVecFloat, DTVecDouble,
		 DTVecComplex, DTVecDComplex, DTVecString};

  /** @name Constructors
      @memo Construct value with given type
      @doc Construct value with given type. Default is empty vector<ParamValue>.
  */
  //@{
  ParamValue();
  ParamValue (bool);
  ParamValue (int);
  ParamValue (float);
  ParamValue (double);
  ParamValue (const complex<float>&);
  ParamValue (const complex<double>&);
  ParamValue (const char*);
  ParamValue (const string&);
  ParamValue (const vector<bool>&);
  ParamValue (const vector<int>&);
  ParamValue (const vector<float>&);
  ParamValue (const vector<double>&);
  ParamValue (const vector<complex<float> >&);
  ParamValue (const vector<complex<double> >&);
  ParamValue (const vector<string>&);
  ParamValue (const vector<ParamValue>&);
  ParamValue (const ParamBlock&);
  //@}

  /// Copy constructor (copy semantics).
  ParamValue (const ParamValue&);

  /// Assignment (copy semantics).
  ParamValue& operator= (const ParamValue&);

  ~ParamValue();

  /// Get the data type of the value.
  DataType dataType() const
    { return itsExtDT; }

  /// Is the value a vector?
  bool isVector() const
    { return itsDataType >= DTValueVector; }

  /// Is the value a block?
  bool isBlock() const
    { return itsDataType == DTBlock; }

  /// Return the size of a vector (1 is returned for a scalar).
  unsigned int size() const;

  /** @name Get functions
      @memo Get the value with the given type
      @doc Get the value with the given type.
  */
  //@{
  bool getBool() const;
  int getInt() const;
  float getFloat() const;
  double getDouble() const;
  complex<float> getComplex() const;
  complex<double> getDComplex() const;
  const string& getString() const;
  vector<bool> getVecBool() const;
  vector<int> getVecInt() const;
  vector<float> getVecFloat() const;
  vector<double> getVecDouble() const;
  vector<complex<float> > getVecComplex() const;
  vector<complex<double> > getVecDComplex() const;
  vector<string> getVecString() const;
  const vector<ParamValue>& getVector() const;
  const ParamBlock& getBlock() const;
  //@}

  /** @name Get functions for templated purposes
      @memo Get the value with the given type
      @doc Get the value with the given type.
  */
  //@{
  void get (bool& value) const
    { value = getBool(); }
  void get (int& value) const
    { value = getInt(); }
  void get (float& value) const
    { value = getFloat(); }
  void get (double& value) const
    { value = getDouble(); }
  void get (complex<float>& value) const
    { value = getComplex(); }
  void get (complex<double>& value) const
    { value = getDComplex(); }
  void get (string& value) const
    { value = getString(); }
  void get (vector<bool>& value) const
    { value = getVecBool(); }
  void get (vector<int>& value) const
    { value = getVecInt(); }
  void get (vector<float>& value) const
    { value = getVecFloat(); }
  void get (vector<double>& value) const
    { value = getVecDouble(); }
  void get (vector<complex<float> >& value) const
    { value = getVecComplex(); }
  void get (vector<complex<double> >& value) const
    { value = getVecDComplex(); }
  void get (vector<string>& value) const
    { value = getVecString(); }
  void get (vector<ParamValue>& value) const
    { value = getVector(); }
  void get (ParamBlock& value) const;
  //@}

  friend ostream& operator<< (ostream&, const ParamValue&);

private:
  /// Remove the value.
  void clear();

  /// Copy the value from another one.
  void copyValue (const ParamValue& that);


  DataType itsDataType;
  DataType itsExtDT;
  void*    itsValuePtr;
};

}

#endif 
