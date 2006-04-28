/*
 * IPlot.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import javax.swing.JComponent;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:45
 */
public interface IPlot{

	/**
         * Creates a plot using several key arguments
	 * @param type Type of plot as dictated by PlotConstants.PLOT_*
	 * @param name Name to be given to the plot
	 * @param data The dataset to be used to create the plot
	 * @param separateLegend Indicates the user's need for a separate legend 
	 */
	public JComponent createPlot(int type, String name, HashMap data, boolean separateLegend) throws PlotterException;
        
        /**
	 * Returns the current dataset used in the plot 
	 */
        public HashMap getData();
        /**
         * Sets the dataset used in the plot 
         * @param newData A new set of data
	 */
        public void setData(HashMap newData);
        /**
         * Create a legend/key using the plot specified
         * @param aPlot A plot JComponent
	 */
        public JComponent getLegend(JComponent aPlot) throws PlotterException;
}

