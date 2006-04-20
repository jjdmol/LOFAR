/*
 * IPlotDataExport.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006 11:00:00
 */
public interface IPlotDataExport{
    
    /**
     * @param params
     * @param data
     */
    public void exportData(String[] params, HashMap data) throws PlotterException;
    
}

