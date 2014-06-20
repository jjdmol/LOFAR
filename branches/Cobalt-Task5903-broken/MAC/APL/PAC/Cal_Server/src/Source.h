//#  -*- mode: c++ -*-
//#  Source.h: definition of a sky source
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

#ifndef SOURCE_H_
#define SOURCE_H_

#include <string>
#include <vector>
#include <blitz/array.h>

namespace LOFAR {
  namespace CAL {

    class Source
    {
    public:

      /**
       * Constructor.
       * @param name Name of the source.
       * @param ra Right ascension of the source (in radians).
       * @param dec Declination of the source (in radians).
       */
      Source(std::string name, double ra, double dec, const blitz::Array<double, 2>& flux) :
	m_name(name), m_ra(ra), m_dec(dec), m_flux(flux) {}
      virtual ~Source();

      /**
       * Get the name of the source.
       * @return The name of the source.
       */
      std::string getName() const { return m_name; }

      /**
       * Get the right ascension.
       * @return The right ascension of the source (in radians).
       */
      double getRA() const { return m_ra; }

      /**
       * Get the declination of the source.
       * @return The declination of the source (in radians).
       */
      double getDEC() const { return m_dec; }

      /**
       * Get the positions of a source (in ra,dec).
       * @param ra Reference to the location to store the ra.
       * @param dec Reference to the location to store the dec.
       */
      void getPos(double& ra, double& dec) const;

      /**
       * Get the n-th (freq, flux) pair.
       * @param n Get n-th flux.
       * @param freq Reference to the location to store the frequency.
       * @param flux Reference to the location to store the flux.
       * @return true if point exists, false if 0 > n <= nfrequencies.
       */
      bool getFlux(int n, double& freq, double&) const;

      /**
       * Get the array containing (freq, flux) pairs
       * @return the blitz array with (freq, flux) pairs for the source
       */
      blitz::Array<double, 2> const & getFluxes() const;

    private:
      std::string m_name;
      double      m_ra;
      double      m_dec;
      blitz::Array<double, 2> m_flux; // array of frequency, flux pairs, 
    };

    class Sources
    {
    public:
      Sources();
      virtual ~Sources();

      /**
       * Get a reference to the source vector.
       * @return A reference to the vector of sources.
       */
      const std::vector<Source>& getSources() const { return m_sources; }

      /**
       * Get the source positions as a blitz array.
       * Dimensions: nsources x 2 (ra,dec)
       * @return The source positions as a two-dimensional blitz array (nsources x 2 (ra,dec)).
       */
      const blitz::Array<double, 2> getSourcePositions() const;

      /**
       * Get the number of sources.
       */
      int getNumSources() const { return m_sources.size(); }

      /**
       * Get all sources from file or database.
       * @param url The resource locator for the source database (e.g. filename, or database table).
       * @note Currently only a file name is supported.
       */
      void getAll(std::string url);
    
    private:

      std::vector<Source> m_sources;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SOURCE_H_ */

