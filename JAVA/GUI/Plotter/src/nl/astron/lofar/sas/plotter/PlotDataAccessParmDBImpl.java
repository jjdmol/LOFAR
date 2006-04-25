/*
 * PlotDataAccessParmDBImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimeZone;
import java.util.Vector;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacade;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessParmDBImpl implements IPlotDataAccess{

        private jParmFacade parmDB = null;
       
        static {
           System.loadLibrary("jparmfacade");
        }
        
        
        public PlotDataAccessParmDBImpl(){
            parmDB = new jParmFacade(PlotConstants.DATA_INMEP_FILE_PATH);
        }
        

	public void finalize() throws Throwable {
           parmDB = null;
	}

	/**
	 * @param constraint
	 * 
	 */
	public HashMap retrieveData(String constraint) throws PlotterException{
           
            HashMap<String,Object> returnMap = new HashMap<String, Object>();
            HashSet<HashMap> returnValues = new HashSet<HashMap>();
            
            Vector names = parmDB.getNames(constraint);
            //System.out.println("Parameter Names found for constraint "+constraint+ ": " +names.size()+"x"); 
            
            
            if(names.size()>=1){
                returnMap.put(PlotConstants.DATASET_NAME,"Parameter Database Filter : "+constraint);
                TimeZone utcZone = TimeZone.getTimeZone("UTC");
                utcZone.setDefault(utcZone);
                Calendar aCalendar = Calendar.getInstance(utcZone);
                Date utcDate = aCalendar.getTime();
                
                returnMap.put(PlotConstants.DATASET_SUBNAME,"Generated at "+ utcDate.toString());

                for(int n = 0; n < names.size();n++){
                     
                     Vector paramValues = parmDB.getRange(names.get(n).toString());
                     System.out.println("Parameter Range Values found: "+paramValues.size()+"x"); 
                     double startx = 0;
                     double endx = 5;
                     double starty = 0;
                     double endy = 5;
                     int numx = 10;
                     int numy = 10;
                     /*
                     if(paramValues.size()==4){
                         startx = Double.parseDouble(paramValues.get(0).toString());
                         endx =Double.parseDouble(paramValues.get(1).toString());
                         starty = Double.parseDouble(paramValues.get(2).toString());
                         endy = Double.parseDouble(paramValues.get(3).toString());
                         System.out.println("Parameter Range : startx:"+startx+" endx:" +endx+ " starty: "+starty+" endy:"+endy ); 
                     }*/
                                  
                     returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_START,Double.toString(startx));
                     returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_END,Double.toString(endx));
                     returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_START,Double.toString(starty));
                     returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_END,Double.toString(endy));
                     
                     HashMap<String,Vector<Double>> values = parmDB.getValues((String)names.get(n),startx,endx,numx,starty,endy,numy);
                     //HashMap<String,Vector<Double>> values = parmDB.getValues((String)names.get(n),1,5,numx,1,5,numy);

                     Iterator anIterator = values.keySet().iterator();
                     while(anIterator.hasNext()){
                         HashMap<String,Object> aValueMap = new HashMap<String,Object>();

                         String aValue = (String)anIterator.next();
                         System.out.println("Parameter Value Found: "+aValue);

                         aValueMap.put(PlotConstants.DATASET_VALUELABEL,aValue);

                         Vector<Double> valueDoubles = (Vector<Double>)values.get(aValue);
                         System.out.println("Parameter doubles inside " +aValue+": "+valueDoubles.size()+"x");
                         double[] xArray = new double[valueDoubles.size()];
                         double[] yArray = new double[valueDoubles.size()];

                         for(int i = 0;(i<valueDoubles.size());i++){
                               if(numx > 1){
                                   xArray[i] = startx + ( (endx-startx) / (numx*i+1) );
                                   yArray[i] = valueDoubles.get(i);
                                   System.out.println("xArray["+xArray[i]+"] yArray["+yArray[i]+"]");  
                               }
                               else if(numy > 1){
                                   yArray[i] = starty + ( (endy-starty) / (numy*i+1) );
                                   xArray[i] = valueDoubles.get(i);
                                   System.out.println("xArray["+xArray[i]+"] yArray["+yArray[i]+"]");
                               }
                               
                             
                         }
                         aValueMap.put(PlotConstants.DATASET_XVALUES,xArray);
                         aValueMap.put(PlotConstants.DATASET_YVALUES,yArray);
                         returnValues.add(aValueMap);
                     }
                     
                }
                returnMap.put(PlotConstants.DATASET_VALUES,returnValues);
            }
            return returnMap; 
	}
}