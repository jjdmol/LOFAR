/*
 * PlotController.java
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

import java.awt.Image;
import java.util.HashMap;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import javax.swing.JComponent;
import nl.astron.lofar.java.gui.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.java.gui.plotter.exceptions.NotSupportedException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterFrameworkInitializationException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterFrameworkNotCompatibleException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterFrameworkNotFoundException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:43
 */
public class PlotController{

	private PlotDataManager m_PlotDataManager;
	private PlotGroup m_PlotGroup;
	private IPlot m_IPlot;
        	
	public PlotController(){
            m_PlotDataManager = PlotDataManager.getInstance();
            //Initialise and load plotter classes and data interfaces
        }

	public void finalize() throws Throwable {
            m_PlotDataManager = null;
            m_PlotGroup = null;
            m_IPlot = null;
	}

	/**
         * This method will attempt to create a JComponent plot using the framework in the plotter_config.properties file.
         *
         * @param type A plot type (as defined in PlotConstants.PLOT_*)
         * @param separateLegend True to generate a separate legend JComponent, false to have a legend embedded in the plot
	 * @param constraints A data access identifier array
	 * 
	 */
	public JComponent createPlot(int type, boolean separateLegend, String[] constraints) throws PlotterException{
            String plotFrameworkClass = "";
            Object aPlotter = null;
            IPlot aNewPlot = null;
            ResourceBundle resources = PlotDataManager.getPlotterConfigurationFile();
            try {
                plotFrameworkClass = resources.getString(("FRAMEWORK"));
                Class implementator = PlotController.class.forName(plotFrameworkClass);
                aPlotter = implementator.newInstance();
                aNewPlot = (IPlot)aPlotter;
            } catch (IllegalAccessException ex) {
                //TODO Log!
                PlotterFrameworkInitializationException exx = new PlotterFrameworkInitializationException();
                exx.initCause(ex);
                throw exx; 
            } catch (ClassNotFoundException ex) {
                //TODO Log!
                PlotterFrameworkNotFoundException exx = new PlotterFrameworkNotFoundException("(used:"+ plotFrameworkClass + " )");
                exx.initCause(ex);
                throw exx; 
            } catch (MissingResourceException ex) {
                //TODO Log!
                PlotterFrameworkNotFoundException exx = new PlotterFrameworkNotFoundException("(The property 'FRAMEWORK' was not found in the plotter_config.properties file)");
                exx.initCause(ex);
                throw exx;  
            } catch (InstantiationException ex) {
                //TODO Log!
                PlotterFrameworkInitializationException exx = new PlotterFrameworkInitializationException();
                exx.initCause(ex);
                throw exx;
                
            } catch (ClassCastException ex) {
                //TODO LOG!
                PlotterFrameworkNotCompatibleException exx = new PlotterFrameworkNotCompatibleException();
                exx.initCause(ex);
                throw exx;
            } catch (Exception ex) {
                //TODO Log!
                PlotterFrameworkInitializationException exx = new PlotterFrameworkInitializationException();
                exx.initCause(ex);
                throw exx;
            }
            
            if(aPlotter != null){
                if(type == PlotConstants.PLOT_BAR
                        || type == PlotConstants.PLOT_GRID
                        || type == PlotConstants.PLOT_SCATTER
                        || type == PlotConstants.PLOT_XYLINE
                        || type == PlotConstants.PLOT_POINTS){

                        HashMap retrieveableData = 
                            m_PlotDataManager.retrieveData(constraints);                               
                        m_IPlot = aNewPlot;
                        return m_IPlot.createPlot(type,constraints[0],retrieveableData,separateLegend);
                
                }else{
                    throw new NotSupportedException("The requested plot type ("+type+") is not supported by the plotter at this time.");
                }   
            }
            return null;
	}
        /**
         * This method will attempt to retrieve a JComponent legend for the supplied JComponent plot.
         *
         * Important: The JComponent plot you provide must have been generated by the plotter framework
         *            implementation class (for example PlotSGTImpl). An exception can be thrown if you attempt
         *            to retrieve a Legend for say a JPanel or JButton.
         *
         * @param aPlot The JComponent plot that needs to have a separate legend
	 * 
	 */
        public JComponent getLegendForPlot(JComponent aPlot) throws PlotterException{
            return m_IPlot.getLegend(aPlot);
        }

	/**
	 * @param constraints The array of string constraints to be passed to the data access layer
	 * 
	 */
	public Image createPlotImage(String[] constraints) throws PlotterException{
            throw new NotImplementedException("Image exportation is not yet implemented in this release."); 
	}

	/**
         * @param arguments The array of string arguments to be passed to the data export layer
	 * 
	 */
	public void exportData(String[] arguments) throws PlotterException{
            throw new NotImplementedException("Export of data is not yet implemented in this release."); 
	}

	/**
	 * @param data
	 * 
	 */
	public JComponent modifyPlot(HashMap data){
		return null;
	}

}