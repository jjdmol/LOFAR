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
import java.util.Map;
import javax.swing.JComponent;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 12-apr-2006 15:15:03
 */
public class PlotController{

	public PlotDataManager m_LOFARDataManager;
	public PlotGroup m_PlotGroup;
	public IPlot m_IPlot;
	
	public PlotController(){

	}

	public void finalize() throws Throwable {

	}

	/**
	 * @param constraint
	 * 
	 */
	public JComponent createPlot(String constraint){
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