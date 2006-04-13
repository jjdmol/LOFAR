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
            m_PlotDataManager = new PlotDataManager();
            //Initialise and load plotter classes and data interfaces
	}

	public void finalize() throws Throwable {

	}

	/**
	 * @param constraint
	 * 
	 */
	public JComponent createPlot(String constraint){
		IPlot aNewPlot = (IPlot)new SGTPlot();
                HashMap retrieveableData = 
                        m_PlotDataManager.retrieveData(constraint);                               
                return aNewPlot.createPlot(
                        aNewPlot.XYLINE,"test",retrieveableData);
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