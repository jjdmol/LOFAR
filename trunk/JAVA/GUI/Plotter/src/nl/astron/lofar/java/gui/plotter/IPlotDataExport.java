/*
 * IPlotDataExport.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter;

import java.util.HashMap;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006 11:00:00
 */
public interface IPlotDataExport{
    
    /**
     * Exports a dataset using a set of parameters
     * @param params String array containing zero or more arguments for the export class
     * @param data the dataset to be exported
     */
    public void exportData(String[] params, HashMap data) throws PlotterException;
    
}

