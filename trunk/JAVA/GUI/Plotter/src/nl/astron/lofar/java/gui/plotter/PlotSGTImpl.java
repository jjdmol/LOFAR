/*
 * PlotSGTImpl.java
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
 * NOTICE :Although this class works with the standard SGT release,
 *         it is recommended you use the supplied SGT jar file to
 *         prevent as much bugs as possible.
 */

package nl.astron.lofar.java.gui.plotter;

import gov.noaa.pmel.sgt.DataNotFoundException;
import gov.noaa.pmel.sgt.JPane;
import gov.noaa.pmel.sgt.LineAttribute;
import gov.noaa.pmel.sgt.LineCartesianRenderer;
import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleGrid;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import gov.noaa.pmel.sgt.swing.prop.LineAttributeDialog;
import gov.noaa.pmel.util.Dimension2D;
import gov.noaa.pmel.util.Rectangle2D;
import gov.noaa.pmel.util.SoTDomain;
import gov.noaa.pmel.util.SoTRange;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyVetoException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import javax.swing.JComponent;
import nl.astron.lofar.java.gui.plotter.exceptions.EmptyDataSetException;
import nl.astron.lofar.java.gui.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.java.gui.plotter.exceptions.NotSupportedException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:46
 */
public class PlotSGTImpl implements IPlot{
    
        private HashMap data;
        private JPlotLayout aLayout;
        private LineAttributeDialog lad;
        private plotMouseAdapter pma;
        
	public PlotSGTImpl(){
        
	}
        
	public JComponent createPlot(int type, String name, HashMap data, boolean separateLegend) throws PlotterException{
	    
            JPlotLayout aNewPlot = null; 
            
            if(type==PlotConstants.PLOT_XYLINE){
                aNewPlot = linePlot(name,data, separateLegend,false);
               
                pma = new plotMouseAdapter();
                if(aNewPlot.getKeyPane()!= null) aNewPlot.getKeyPane().addMouseListener(pma);
                aNewPlot.addMouseListener(pma);
            }
            else if(type==PlotConstants.PLOT_POINTS){
                aNewPlot = linePlot(name,data, separateLegend,true);
                 pma = new plotMouseAdapter();
                  if(aNewPlot.getKeyPane()!= null) aNewPlot.getKeyPane().addMouseListener(pma);
                 aNewPlot.addMouseListener(pma);
            }
            else if(type==PlotConstants.PLOT_GRID){
                aNewPlot = gridPlot(name,data, separateLegend);
                 
            }
            else if(type==PlotConstants.PLOT_SCATTER){
                aNewPlot = scatterPlot(name,data, separateLegend);
            }
            //JPanel keyAndPlotPanel = new JPanel();
            //keyAndPlotPanel.add(aNewPlot);
            //keyAndPlotPanel.add(aNewPlot,BorderLayout.CENTER);
            //JPane keyPane = aNewPlot.getKeyPane();
            
            //keyAndPlotPanel.add(keyPane,BorderLayout.SOUTH);
            aLayout = aNewPlot;
            
            return aNewPlot;
	}
        
        private JPlotLayout linePlot(String name, HashMap data, boolean separateLegend, boolean showPointsOnly) throws PlotterException{
            JPlotLayout layout = new JPlotLayout(JPlotLayout.LINE, false, false,
                    name,null,separateLegend);
            layout.setSize(640,480);
            
            if(separateLegend){
               layout.setKeyLayerSizeP(new Dimension2D(6.0, 1.0));
               layout.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0, 6.0, 1.0));  
            }
            layout.setBatch(true);
            layout.setId(name);
            layout.setName(name);
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";                       
            String yAxisUnits = "no unit specified";
            double xstart = -1;
            double xend = -1;
            double ystart = -1;
            double yend = -1;
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XY values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                
                while(it.hasNext()){
                    String key = (String)it.next();
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_START)){
                        xstart = Double.parseDouble((String)data.get(key));
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_END)){
                        xend = Double.parseDouble((String)data.get(key));
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_START)){
                        ystart = Double.parseDouble((String)data.get(key));
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_END)){
                        yend = Double.parseDouble((String)data.get(key));
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
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
                        
                        double[] xArray = null;
                        double[] yArray = null;
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                                   xAxisUnits,
                                                   false,
                                                   false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                                   yAxisUnits,
                                                   false,
                                                   false);
                        String lineLabel = "Unknown value";
                        HashMap line = (HashMap)linesIterator.next();
                        Iterator lineIterator = line.keySet().iterator();
                        
                        //Retrieve XY pair label and xy values
                        while(lineIterator.hasNext()){ 
                            String key = (String)lineIterator.next();
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                            lineLabel = (String)line.get(key);
                            
                            }  
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                               xArray = (double[])line.get(key);
                            }
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])line.get(key);
                            }                   
                        }
                        
                        SimpleLine lineData = new SimpleLine(
                                xArray,yArray,lineLabel);
                        
                        lineData.setXMetaData(meta);
                        lineData.setYMetaData(ymeta);
                        //Add line to plot
                        layout.addData(lineData, lineLabel);
                       
                        if(showPointsOnly){
                            try{
                                LineAttribute marks;
                                marks = (LineAttribute) layout.getAttribute(lineData);
                                marks.setStyle(LineAttribute.MARK);
                            } catch (DataNotFoundException ex) {
                                //LOG! ex.printStackTrace();
                            }
                        }
                    }
                }
            }else{
                //TODO LOG!
                throw new EmptyDataSetException();
                
            }
            if(xstart!= -1 && xend != -1 && ystart != -1 && yend != -1){
               
                layout.setAutoRange(false,false);
                try {
                    
                    layout.setRange(new SoTDomain(new SoTRange.Double(xstart,xend),new SoTRange.Double(ystart,yend)));
                } catch (PropertyVetoException ex) {
                    ex.printStackTrace();
                }
            }
            layout.setName(plotTitle);
            layout.setBatch(false);
            return layout;
        }
        
        private JPlotLayout gridPlot(String name, HashMap data, boolean separateLegend) throws PlotterException{
            
            
            JPlotLayout layout = new JPlotLayout(JPlotLayout.GRID, false, false,
                    name,null,separateLegend);
            layout.setBatch(true);
            layout.setSize(640,480);
            if(separateLegend){
               layout.setKeyLayerSizeP(new Dimension2D(6.0, 1.0));
               layout.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0, 6.0, 1.0));  
            }else{
               //layout.setKeyLayerSizeP(new Dimension2D(4.0,1.0));
               //layout.setKeyBoundsP(new Rectangle2D.Double(0.1, 0.1, 4.0, 1.0));
               //layout.setKeyAlignment(layout.LEFT,layout.TOP);
            }
           
            layout.setId(name);
            
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";                       
            String yAxisUnits = "no unit specified";
            String zAxisTitle = "Z";                       
            String zAxisUnits = "no unit specified";
            
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XYZ values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                while(it.hasNext()){
                    String key = (String)it.next();
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZAXISLABEL)){
                        zAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZAXISUNIT)){
                        zAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
                       values = (HashSet<HashMap>)data.get(key); 
                    }
                }
                //Set titles to plot

                layout.setTitles(plotTitle,plotSubTitle,"");
               
                //Loop through XYZ Value data
                if(values != null && values.size()> 0){

                    Iterator linesIterator = values.iterator();
                    //Loop through all XY pairs
                    int lineNumber = 0;
                    while(linesIterator.hasNext()){
                        lineNumber++;
                        double[] xArray = null;
                        double[] yArray = null;
                        double[] zArray = null;
                        
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                                   xAxisUnits,
                                                   false,
                                                   false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                                   yAxisUnits,
                                                   false,
                                                   false);
                        
                        SGTMetaData zmeta = new SGTMetaData(zAxisTitle,
                                                   zAxisUnits,
                                                   false,
                                                   false);
                        
                        String lineLabel = "Unspecified value "+ lineNumber;
                        HashMap grid = (HashMap)linesIterator.next();
                        Iterator lineIterator = grid.keySet().iterator();
                        
                        //Retrieve XYZ pair label and xyz values
                        while(lineIterator.hasNext()){ 
                            String key = (String)lineIterator.next();
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                            lineLabel = (String)grid.get(key);
                            
                            }  
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                               xArray = (double[])grid.get(key);
                            }
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])grid.get(key);
                            }
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZVALUES)){
                                zArray = (double[])grid.get(key);
                            } 
                        }
                        SimpleGrid gridData = new SimpleGrid(
                                zArray,xArray,yArray,lineLabel);
                        gridData.setXMetaData(meta);
                        gridData.setYMetaData(ymeta);
                        gridData.setZMetaData(zmeta);
                        //Add line to plot
                        layout.addData(gridData, lineLabel);
                    }
                }
            }else{
                //TODO LOG!
                throw new EmptyDataSetException();
                
            }
            layout.setName(plotTitle);
            layout.setBatch(false);
            return layout; 
        }
        
        private JPlotLayout scatterPlot(String name, HashMap data, boolean separateLegend) throws PlotterException{
             throw new NotImplementedException("Scatter plots are not yet implemented in the plotter's SGT plugin."); 
        }
        
        public HashMap getData(){
            return data;
        }
        public void setData(HashMap newData){
            if(newData!=null){
                this.data = newData;
            }
        }
        public JComponent getLegend(JComponent aPlot) throws PlotterException{
        JPlotLayout parentPlot = null;
        JPane keyPane = null;
            try {
                parentPlot = (JPlotLayout) aPlot;
                keyPane = parentPlot.getKeyPane();
                
                keyPane.setSize(new Dimension(600,100));
            } catch (ClassCastException e) {
                throw new NotSupportedException("The plot ("+aPlot.getName()+") is not recognized by the plotter's configured framework (SGT).");
            } catch (NullPointerException e) {
                throw new NotSupportedException("The plot ("+aPlot.getName()+") does not have a legend available.");
            }
        
            return keyPane;
        }
        
        class plotMouseAdapter extends MouseAdapter{
            public void mouseReleased(MouseEvent e){
                Object object = e.getSource();
                if(object == aLayout.getKeyPane()){
                    if(e.isPopupTrigger() || e.getClickCount() == 2){
                        Object obj = aLayout.getKeyPane().getObjectAt(e.getX(),e.getY());
                        
                        aLayout.getKeyPane().setSelectedObject(obj);
                        if(obj instanceof LineCartesianRenderer){
                            LineAttribute attr = ((LineCartesianRenderer)obj).getLineAttribute();
                            if(lad == null){
                                lad = new LineAttributeDialog();
                            }
                            SimpleLine aLine = (SimpleLine)((LineCartesianRenderer)obj).getLine();
                            lad.setTitle("Line Attribute Configuration for " + aLine.getTitle());
                            aLayout.getKeyPane().setBatch(true);
                            aLayout.getKeyPane().setBatch(false);
                            lad.setLineAttribute(attr);
                            if(!lad.isShowing()){
                                lad.setVisible(true);
                            }
                        }
                    }
                }
                else if(object == aLayout){
                    if(e.isPopupTrigger() || e.getClickCount() == 2){
                        Object obj = aLayout.getObjectAt(e.getX(),e.getY());
                        
                        aLayout.setSelectedObject(obj);
                        if(obj instanceof LineCartesianRenderer){
                            LineAttribute attr = ((LineCartesianRenderer)obj).getLineAttribute();
                            if(lad == null){
                                lad = new LineAttributeDialog();
                            }
                            SimpleLine aLine = (SimpleLine)((LineCartesianRenderer)obj).getLine();
                            lad.setTitle("Line Attribute Configuration for " + aLine.getTitle());
                            lad.setLineAttribute(attr);
                            if(!lad.isShowing()){
                                lad.setVisible(true);
                            }
                        }
                    }
                }
            }
           
        }
        
}
