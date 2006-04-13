/*
 * PlotDataManager.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import java.util.HashSet;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:42
 */
public class PlotDataManager{

	//private ParmDB m_ParmDB;

	public PlotDataManager(){

	}

	public void finalize() throws Throwable {

	}

	/**
	 * @param constraint
	 * 
	 */
	public HashMap retrieveData(String constraint){
            
            HashMap data = new HashMap();
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";
            String yAxisUnits = "no unit specified";
            HashSet<HashMap> values = new HashSet<HashMap>();
            
	    return null;
	}

	/**
	 * @param rootParams
	 * @param data
	 * 
	 */
	public void exportRoot(String[] rootParams, HashMap data){

	}

}