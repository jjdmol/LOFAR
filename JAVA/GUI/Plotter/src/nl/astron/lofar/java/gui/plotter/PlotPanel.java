/**
 * PlotPanel.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl

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

package nl.astron.lofar.java.gui.plotter;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.PrintJob;
import javax.swing.JComponent;
import javax.swing.JPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:47
 */
public class PlotPanel extends JPanel{

	private PlotController m_PlotController;
        private JComponent plot;
        private JComponent legend;
        private String[] currentDataConstraint;
        
        /**
         * - Constructs a new PlotPanel
         * - Gets a new PlotController to work with
         */
	public PlotPanel(){
            m_PlotController = new PlotController();
            this.setBackground(Color.WHITE);
            this.setLayout(new BorderLayout());
        }
        /**
         * Finalize method to cleanup instance variables.
         */
	public void finalize() throws Throwable {
            plot = null;
            legend = null;
            m_PlotController = null;
            currentDataConstraint = null;
	}
        /**
         * This method will attempt to generate a plot using several key arguments:
         * -int type (this is defined by PlotConstants.PLOT_*)
         * -boolean separateLegend (this will tell the plotter that you would like a separate JComponent legend,
         *                          otherwise you will see a legend where the plotter puts it by default.
         * -String[] constraints (These are the arguments you need for the data access layer to get the data
         *                        you want, which can be anything, as long as your configured data access layer
         *                        supports it!)
         * The plot will be added to this class' ContentPane so you can view it directly. It can also be retrieved separately
         * by calling the getPlot() method. 
         *                         
         * @param type The type of plot to be generated (these types are defined in PlotConstants.PLOT_* )
         * @param separateLegend Tells the plotter if you like a separate legend JComponent, or leave it embedded in the plot itself. Dont forget to call getLegendForPlot() to get the separate legend!
         * @param constraints The arguments to be passed to the configured data access layer
         */
	public void createPlot(int type, boolean separateLegend, String[] constraints) throws PlotterException{
            this.removeAll();
            plot = null;
            legend = null;
            currentDataConstraint = constraints;
            plot = m_PlotController.createPlot(type, separateLegend, constraints);
            this.add(plot,BorderLayout.CENTER);
        }
        /**
         * This method will attempt to generate a plot image using several key arguments:
         * -int type (this is defined by PlotConstants.PLOT_*)
         * -String[] constraints (These are the arguments you need for the data access layer to get the data
         *                        you want, which can be anything, as long as your configured data access layer
         *                        supports it!)
         *                         
         * @param type The type of plot to be generated (these types are defined in PlotConstants.PLOT_* )
         * @param constraints The arguments to be passed to the configured data access layer
         * @return The image file which can be saved to a file or used in different software.
         */
	public Image exportImage(int type, String[] constraints) throws PlotterException{
            return m_PlotController.createPlotImage(constraints);
	}
        /**
         * This method will attempt to export the data currently in the plot to your configured data export layer using a single argument:
         * -String[] arguments (These are the arguments you need for the data export layer to export the data
         *                        you currently have in the plot, and these strings can mean anything, as long as your configured data export layer
         *                        supports them!)
         */
	public void exportData(String[] arguments) throws PlotterException{
            m_PlotController.exportData(arguments);
        }
         /**
          * TODO: JavaDoc!
          */
	public void modifyDataSelection() throws PlotterException{

	}
        /**
         * This method will attempt to print the current plot
         */
	public PrintJob printPlot() throws PlotterException{
            
            return null;
	}
        /**
         * This method will return the plot currently in memory
         * @return The plot currently in memory
         */
        public JComponent getPlot(){
            return plot;
        }
        /**
         * This method will return the legend for the plot currently in memory.
         * It will throw a NotSupportedException should the plot not have a separate legend available.
         * @return The plot legend currently in memory
         */
        public JComponent getLegendForPlot() throws PlotterException{
            if(legend == null && plot != null){
               legend = m_PlotController.getLegendForPlot(plot);
            }
            return legend;
            
        }
        
 }
