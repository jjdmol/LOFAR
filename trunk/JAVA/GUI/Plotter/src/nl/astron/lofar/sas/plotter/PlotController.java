/*
 * PlotController.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.awt.Image;
import java.util.HashMap;
import java.util.Map;
import javax.swing.JComponent;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterFrameworkInitializationException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterFrameworkNotCompatibleException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterFrameworkNotFoundException;

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
	 * @param constraint
	 * 
	 */
	public JComponent createPlot(String constraint) throws PlotterException{
        
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
                throw new PlotterFrameworkNotFoundException();
            } catch (InstantiationException ex) {
                //TODO Log!
                throw new PlotterFrameworkInitializationException();
            } catch (ClassCastException ex) {
                //TODO LOG!
                throw new PlotterFrameworkNotCompatibleException();
            }
            
            if(aPlotter != null){
            
                HashMap retrieveableData = 
                    m_PlotDataManager.retrieveData(constraint);                               
                return aNewPlot.createPlot(
                    aNewPlot.XYLINE,"test",retrieveableData);
                
            }
            return null;
	}

	/**
	 * @param constraint
	 * 
	 */
	public Image createPlotImage(String constraint){
		return null;
	}

	/**
	 * @param data
	 * 
	 */
	public void exportRoot(Map data){

	}

	/**
	 * @param data
	 * 
	 */
	public JComponent modifyPlot(Map data){
		return null;
	}

}
