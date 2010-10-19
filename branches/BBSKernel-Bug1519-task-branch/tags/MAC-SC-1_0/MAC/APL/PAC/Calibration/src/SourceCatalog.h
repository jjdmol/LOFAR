//#  -*- mode: c++ -*-
//#  SourceCatalog.h: definition of the Sky model (source catalog)
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

#ifndef SOURCECATALOG_H_
#define SOURCECATALOG_H_

#include <vector>
#include <blitz/array.h>
#include "Source.h"
#include "Timestamp.h"

namespace CAL
{
  class SourceCatalog
  {
  public:
    SourceCatalog(std::string name);
    virtual ~SourceCatalog();

    /**
     * Get a reference to the source catalog vector.
     * @return A reference to the vector of sources.
     */
    const std::vector<Source>& getCatalog() const { return m_sources; }

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
     * Get the name of the source catalog.
     */
    std::string getName() const { return m_name; }

    
  private:

    friend class SourceCatalogLoader;

    std::string         m_name;
    std::vector<Source> m_sources;
  };

  class SourceCatalogLoader
  {
  public:
    
    /**
     * Load source catalog from file.
     *
     * File format: comma separated, one source per line (type in square brackets).
     * 
     * source name [string], RA (rad) [double], DEC (rad) [double], [freq (MHz) [double], flux (Jy) [double]]*
     *
     * The frequency-flux pairs must be in order increasing frequency
     *
     * @param filename Name of the input source catalog file.
     * @return The newly allocated source catalog.
     */
    static const SourceCatalog* loadFromFile(std::string filename);
  };

};

#endif /* SOURCECATALOG_H_ */

