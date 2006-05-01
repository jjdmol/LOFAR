/*
 * PlotDataAccessParmDBImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimeZone;
import java.util.Vector;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacade;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessParmDBImpl implements IPlotDataAccess{
    
    public static final int requiredDataConstraints = 7;
    //Location of ParmDB table file(s)
    public static final String DATA_INMEP_FILE_PATH = "/home/pompert/transfer/tParmFacade.in_mep";
      
    private jParmFacade parmDB = null;
    
    static {
        System.loadLibrary("jparmfacade");
    }
    
    
    public PlotDataAccessParmDBImpl(){
        parmDB = new jParmFacade(this.DATA_INMEP_FILE_PATH);
    }
    
    
    public void finalize() throws Throwable {
        parmDB = null;
    }
    
    /**
     * @param constraint
     *
     */
    public HashMap retrieveData(String[] constraints) throws PlotterDataAccessException{
        
        HashMap<String,Object> returnMap = new HashMap<String, Object>();
        HashSet<HashMap> returnValues = new HashSet<HashMap>();
        
        Vector names = parmDB.getNames(constraints[0]);
        //System.out.println("Parameter Names found for constraint "+constraint+ ": " +names.size()+"x");
        
        
        if(names.size()>=1 && constraints.length == this.requiredDataConstraints){
            returnMap.put(PlotConstants.DATASET_NAME,"Parameter Database Filter : "+constraints[0]);
            TimeZone utcZone = TimeZone.getTimeZone("UTC");
            utcZone.setDefault(utcZone);
            Calendar aCalendar = Calendar.getInstance(utcZone);
            Date utcDate = aCalendar.getTime();
            
            returnMap.put(PlotConstants.DATASET_SUBNAME,"Generated at "+ utcDate.toString());
            
            for(int n = 0; n < names.size();n++){
                
                Vector paramValues = parmDB.getRange(names.get(n).toString());
                //System.out.println("Parameter Range Values found: "+paramValues.size()+"x");
                double startx = Double.parseDouble(constraints[1]);
                double endx = Double.parseDouble(constraints[2]);
                double starty = Double.parseDouble(constraints[4]);
                double endy = Double.parseDouble(constraints[5]);
                int numx = Integer.parseInt(constraints[3]);
                int numy = Integer.parseInt(constraints[6]);
                
                if(paramValues.size()==4){
                    //startx = Double.parseDouble(paramValues.get(0).toString());
                    //endx =Double.parseDouble(paramValues.get(1).toString());
                    //starty = Double.parseDouble(paramValues.get(2).toString());
                    //endy = Double.parseDouble(paramValues.get(3).toString());
                    //System.out.println("Parameter Range : startx:"+startx+" endx:" +endx+ " starty: "+starty+" endy:"+endy );
                }
                
                returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_START,Double.toString(startx));
                returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_END,Double.toString(endx));
                returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_START,Double.toString(starty));
                returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_END,Double.toString(endy));
                HashMap<String, Vector<Double>> values = new HashMap<String,Vector<Double>>();
                try {
                    values = parmDB.getValues((String) names.get(n), startx, endx, numx, starty, endy, numy);
                } catch (Exception ex) {
                    throw new PlotterDataAccessException("An invalid call was made to the ParmDB interface. Please check that all variables seem OK. If so, contact support for details.");
                }
                
                Iterator anIterator = values.keySet().iterator();
                while(anIterator.hasNext()){
                    HashMap<String,Object> aValueMap = new HashMap<String,Object>();
                    
                    String aValue = (String)anIterator.next();
                    //System.out.println("Parameter Value Found: "+aValue);
                    
                    aValueMap.put(PlotConstants.DATASET_VALUELABEL,aValue);
                    
                    Vector<Double> valueDoubles = (Vector<Double>)values.get(aValue);
                    //System.out.println("Parameter doubles inside " +aValue+": "+valueDoubles.size()+"x");
                    double[] xArray = new double[valueDoubles.size()];
                    double[] yArray = new double[valueDoubles.size()];
                    
                    for(int i = 0;(i<valueDoubles.size());i++){
                        xArray[i] = startx + (endx-startx) / valueDoubles.size()*(i+0.5);
                        yArray[i] = valueDoubles.get(i);
                    }
                    aValueMap.put(PlotConstants.DATASET_XVALUES,xArray);
                    aValueMap.put(PlotConstants.DATASET_YVALUES,yArray);
                    returnValues.add(aValueMap);
                }
                
            }
            returnMap.put(PlotConstants.DATASET_VALUES,returnValues);
        
        }else if (constraints.length != this.requiredDataConstraints){
            throw new PlotterDataAccessException("An invalid amount of parameters (" +constraints.length+" instead of " +this.requiredDataConstraints+") were passed to the ParmDB Data Access Interface");
        }else if (names.size() < 1){
            throw new PlotterDataAccessException("No results were found in the ParmDB table(s) using the given parameter name filter ( "+constraints[0]+" )");
        }
        return returnMap;
    }
}