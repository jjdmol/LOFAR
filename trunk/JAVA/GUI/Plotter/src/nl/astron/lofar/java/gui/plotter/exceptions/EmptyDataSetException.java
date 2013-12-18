/*
 * EmptyDataSetException.java
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
 * This exception can be thrown when a IPlot implementing class encounters a 
 * data set being passed to it that doesn't contain any information.
 * @version $Id$
 * @created April 18, 2006, 11:02 AM
 * @author pompert
 */
public class EmptyDataSetException extends PlotterException{
    
    /** Creates a new instance of EmptyDataSetException */
    public EmptyDataSetException() {
        super();
    }
    /** 
     * Returns the exception message defined in PlotConstants.EXCEPTION_EMPTY_DATASET
     * @return the Exception message
     * @see PlotConstants
     */
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_EMPTY_DATASET;
    }
    
    
}
