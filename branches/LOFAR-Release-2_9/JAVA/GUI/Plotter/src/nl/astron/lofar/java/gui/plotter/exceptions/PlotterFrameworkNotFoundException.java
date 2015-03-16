/*
 * PlotterFrameworkNotFoundException.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
 * This exception can be thrown if the Plotter Framework Class, 
 * as configured in the plotter_config.properties file variable 'FRAMEWORK',
 * could not be located anywhere in the Java CLASSPATH.
 * @version $Id$
 * @created April 19, 2006, 11:02 AM
 * @author pompert
 * @see nl.astron.lofar.java.gui.plotter.PlotController
 * @see PlotConstants
 * @see plotter_config.properties
 */
public class PlotterFrameworkNotFoundException extends PlotterException{
    
    private String message;
    /** 
     * Creates a new instance of PlotterFrameworkNotFoundException, using the supplied message
     * @param message A custom string to be added to the exception message
     */
    public PlotterFrameworkNotFoundException(String message) {
        super();
        this.message = message;
    }
    /** 
     * Returns the exception message defined in PlotConstants.EXCEPTION_FRAMEWORK_NOT_FOUND
     * @return the Exception message
     * @see PlotConstants
     */
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_FRAMEWORK_NOT_FOUND + " " +message;
    }
    
    
}
