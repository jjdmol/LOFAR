/*
 * PlotController.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter;

import java.awt.Image;
import java.util.HashMap;
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
         * @param type A plot type (as defined in PlotConstants.PLOT_*)
	 * @param constraints A data access identifier array
	 * 
	 */
	public JComponent createPlot(int type, String[] constraints) throws PlotterException{
        
            Object aPlotter = null;
            IPlot aNewPlot = null;
            try {

                Class implementator = PlotController.class.forName(PlotConstants.FRAMEWORK);
                aPlotter = implementator.newInstance();
                aNewPlot = (IPlot)aPlotter;
            } catch (IllegalAccessException ex) {
                //TODO Log!
                throw new PlotterFrameworkInitializationException();
            } catch (ClassNotFoundException ex) {
                //TODO Log!
                throw new PlotterFrameworkNotFoundException("(used:"+ PlotConstants.FRAMEWORK + " )");
            } catch (InstantiationException ex) {
                //TODO Log!
                throw new PlotterFrameworkInitializationException();
            } catch (ClassCastException ex) {
                //TODO LOG!
                throw new PlotterFrameworkNotCompatibleException();
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
                        return m_IPlot.createPlot(type,constraints[0],retrieveableData,PlotConstants.PLOT_SEPARATE_LEGEND);
                
                }else{
                    throw new NotSupportedException("The requested plot type ("+type+") is not supported by the plotter at this time.");
                }   
            }
            return null;
	}
        public JComponent getLegendForPlot(JComponent aPlot) throws PlotterException{
            return m_IPlot.getLegend(aPlot);
        }

	/**
	 * @param constraint
	 * 
	 */
	public Image createPlotImage(String constraint) throws PlotterException{
            throw new NotImplementedException("Image exportation is not yet implemented in this release."); 
	}

	/**
	 * @param data
	 * 
	 */
	public void exportData(HashMap data) throws PlotterException{
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
