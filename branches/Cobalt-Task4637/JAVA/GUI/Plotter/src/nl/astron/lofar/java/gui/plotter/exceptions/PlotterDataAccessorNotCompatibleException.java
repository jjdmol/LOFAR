/*
 * PlotterDataAccessorNotCompatibleException.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package nl.astron.lofar.java.gui.plotter.exceptions;

import nl.astron.lofar.java.gui.plotter.PlotConstants;

/**
 * This exception can be thrown if a Data Access Layer class, 
 * as configured in the plotter_config.properties file,
 * is not compatible with the IPlotDataAccess interface.
 * @version $Id$
 * @created April 19, 2006, 11:02 AM
 * @author pompert
 * @see nl.astron.lofar.java.gui.plotter.PlotDataManager
 * @see nl.astron.lofar.java.gui.plotter.IPlotDataAccess
 * @see PlotConstants
 */
public class PlotterDataAccessorNotCompatibleException extends PlotterException{
    
    /** 
     * Creates a new instance of PlotterDataAccessorNotCompatibleException 
     */
    public PlotterDataAccessorNotCompatibleException() {
        super();
    }
    /** 
     * Returns the exception message defined in PlotConstants.EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE
     * @return the Exception message
     * @see PlotConstants
     */
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE;
    }
    
    
}
