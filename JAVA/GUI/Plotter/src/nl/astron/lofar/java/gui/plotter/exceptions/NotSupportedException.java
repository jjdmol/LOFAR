/*
 * NotSupportedException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.exceptions;

import nl.astron.lofar.sas.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 19, 2006, 11:00 AM
 * @author pompert
 */
public class NotSupportedException extends PlotterException{
    
    private String message;
    /** Creates a new instance of NotSupportedException */
    public NotSupportedException(String message) {
        super();
        this.message = message;
    }
    public String getMessage(){
        return PlotConstants.EXCEPTION_OPERATION_NOT_SUPPORTED + message;
    }
    
    
}
