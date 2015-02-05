/*
 * PlotterConfigurationNotFoundException.java
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
 * This exception can be thrown when the plotter_config.properties file can not
 * be found anywhere in the Java CLASSPATH.
 * <br><br>
 * Note: The PlotConstants.RESOURCE_FILE variable tells the plotter where to
 * look for the plotter_config.properties file
 * 
 * @version $Id$
 * @created May 04, 2006, 12:00
 * @author pompert
 * @see PlotConstants
 * @see plotter_config.properties
 */
public class PlotterConfigurationNotFoundException extends PlotterException{
    
   
    /** 
     * Creates a new instance of PlotterConfigurationNotFoundException
     */
    public PlotterConfigurationNotFoundException() {
        super();
      
    }
    /** 
     * Returns the exception message defined in PlotConstants.EXCEPTION_RESOURCE_FILE_NOT_FOUND
     * @return the Exception message
     * @see PlotConstants
     */
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_RESOURCE_FILE_NOT_FOUND;
    }
    
    
}
