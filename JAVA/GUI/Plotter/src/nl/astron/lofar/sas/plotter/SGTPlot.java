/*
 * SGTPlot.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This software uses (parts of) the SGT graphics library. 
 * SGT was developed and is maintained by Donald W. Denbo,
 * National Oceanic and Atmospheric Administration.
 * E-Mail : Donald.W.Denbo@noaa.gov
 * Website: http://www.epic.noaa.gov/java/sgt
 *
 */

package nl.astron.lofar.sas.plotter;

import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import javax.swing.JComponent;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:46
 */
public class SGTPlot implements IPlot{
    
        private HashMap data;
        
	public SGTPlot(){
        
	}
        
	public JComponent createPlot(int type, String name, HashMap data){
	    JPlotLayout aNewPlot = null; 
            if(type==this.XYLINE){
                aNewPlot = linePlot(name,data);
            }
            else if(type==this.GRID){
                aNewPlot = gridPlot(name,data);
            }
            else if(type==this.SCATTER){
                aNewPlot = scatterPlot(name,data);
            }
            return aNewPlot;
	}
        
        private JPlotLayout linePlot(String name, HashMap data){
            JPlotLayout layout = new JPlotLayout(JPlotLayout.LINE, false, false,
                    name,null,true);
            layout.setEditClasses(false);
            layout.setBatch(true);
            layout.setId(name);
            
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";                       
            String yAxisUnits = "no unit specified";
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XY values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                while(it.hasNext()){
                    String key = (String)it.next();
                    if(key.equalsIgnoreCase("DataSetName")){
                        plotTitle = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetSubName")){
                        plotSubTitle = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetXAxisLabel")){
                        xAxisTitle = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetXAxisUnit")){
                        xAxisUnits = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetYAxisLabel")){
                        yAxisTitle = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetYAxisUnit")){
                        yAxisUnits = (String)data.get(key);
                    }
                    if(key.equalsIgnoreCase("DataSetValues")){
                        values = (HashSet<HashMap>)data.get(key);
                    }
                }
                //Set titles to plot

                layout.setTitles(plotTitle,plotSubTitle,"");
            
                //Loop through X and Y Value data
                if(values != null && values.size()> 0){

                    Iterator linesIterator = values.iterator();
                    //Loop through all XY pairs
                    while(linesIterator.hasNext()){ 
                        SimpleLine lineData = new SimpleLine();
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                                   xAxisUnits,
                                                   false,
                                                   false);
                        lineData.setXMetaData(meta);
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                                   yAxisUnits,
                                                   false,
                                                   false);
                        lineData.setYMetaData(ymeta);
                        String lineLabel = "Unknown value";
                        HashMap line = (HashMap)linesIterator.next();
                        Iterator lineIterator = line.keySet().iterator();
                        //Retrieve XY pair label and xy values
                        while(lineIterator.hasNext()){ 
                            String key = (String)lineIterator.next();
                            if(key.equalsIgnoreCase("Label")){
                            lineLabel = (String)data.get(key);
                            lineData.setTitle(lineLabel);
                            }  
                            if(key.equalsIgnoreCase("XValues")){
                                lineData.setXArray((double[])data.get(key));
                            }
                            if(key.equalsIgnoreCase("YValues")){
                                lineData.setXArray((double[])data.get(key));
                            }                   
                        }
                        //Add line to plot
                        layout.addData(lineData);
                    }
                }
            }else{
                //throw exception
                System.out.println("No data in set");
                System.exit(0);
            }
            
            layout.setBatch(false);
            return layout;
        }
        private JPlotLayout gridPlot(String name, HashMap data){
            return null;   
        }
        private JPlotLayout scatterPlot(String name, HashMap data){
            return null;   
        }       
        public HashMap getData(){
            return data;
        }
        public void setData(HashMap newData){
            if(newData!=null){
                this.data = newData;
            }
        }
        
        
}