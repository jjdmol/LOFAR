/*
 * PlotterException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter.exceptions;

import nl.astron.lofar.java.gui.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 18, 2006, 11:03 AM
 * @author pompert
 */
public class PlotterException extends Exception{
    
    /** Creates a new instance of PlotterFrameworkInitializationException */
    public PlotterException() {
        super();
    }
    public String getMessage(){
        return PlotConstants.EXCEPTION_GENERIC;
    }
    
}
