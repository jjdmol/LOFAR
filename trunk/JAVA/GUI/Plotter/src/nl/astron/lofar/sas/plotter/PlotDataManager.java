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
            //create the hashmap to be returned
            HashMap data = new HashMap();
            
            String plotTitle = "Testset LOFAR Plotter";
            data.put(PlotConstants.DATASETNAME,plotTitle);
            String plotSubTitle = "Using dummy data";
            data.put(PlotConstants.DATASETSUBNAME,plotSubTitle);
            String xAxisTitle = "t";
            data.put(PlotConstants.DATASETXAXISLABEL,xAxisTitle);
            
            String xAxisUnits = "seconds";
            data.put(PlotConstants.DATASETXAXISUNIT,xAxisUnits);
            
            String yAxisTitle = "Frequency";
            data.put(PlotConstants.DATASETYAXISLABEL,yAxisTitle);
            
            String yAxisUnits = "MHz";
            data.put(PlotConstants.DATASETYAXISUNIT,yAxisUnits);
                        
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            HashMap aLine = new HashMap();
            
            aLine.put(PlotConstants.VALUELABEL,"Lijntje");
             
             double[] xArray = new double[10];
             double[] yArray = new double[10];
             
             for(int i = 0;i<10;i++){
                 xArray[i] = Math.random()*10;
                 yArray[i] = Math.random()*100;
             }
             
             aLine.put(PlotConstants.XVALUES,xArray);
             aLine.put(PlotConstants.YVALUES,yArray);            
             values.add(aLine);
             
             HashMap aLine2 = new HashMap();
            
             aLine2.put(PlotConstants.VALUELABEL,"Lijntje Twee");
             double[] xArray2 = new double[10];
             double[] yArray2 = new double[10];
             
             xArray2[0] = 1;
             xArray2[1] = 2;
             xArray2[2] = 3;
             xArray2[3] = 4;
             xArray2[4] = 5;
             xArray2[5] = 6;
             xArray2[6] = 7;
             xArray2[7] = 8;
             xArray2[8] = 9;
             xArray2[9] = 10;
             
             yArray2[0] = xArray2[0]*xArray2[0];
             yArray2[1] = xArray2[1]*xArray2[1];
             yArray2[2] = xArray2[2]*xArray2[2];
             yArray2[3] = xArray2[3]*xArray2[3];
             yArray2[4] = xArray2[4]*xArray2[4];
             yArray2[5] = xArray2[5]*xArray2[5];
             yArray2[6] = xArray2[6]*xArray2[6];
             yArray2[7] = xArray2[7]*xArray2[7];
             yArray2[8] = xArray2[8]*xArray2[8];
             yArray2[9] = xArray2[9]*xArray2[9];
                                       
             aLine2.put(PlotConstants.XVALUES,xArray2);
             aLine2.put(PlotConstants.YVALUES,yArray2);            
             values.add(aLine2);
                          
             data.put(PlotConstants.DATASETVALUES,values);
	    return data;
	}

	/**
	 * @param rootParams
	 * @param data
	 * 
	 */
	public void exportRoot(String[] rootParams, HashMap data){

	}

}