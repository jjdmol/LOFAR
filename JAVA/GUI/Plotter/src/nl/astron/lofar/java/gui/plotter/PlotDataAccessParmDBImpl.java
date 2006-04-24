/*
 * PlotDataAccessParmDBImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacade;
import nl.astron.lofar.sas.plotter.exceptions.NotImplementedException;
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
            
            Vector names = parmDB.getNames(constraint);
            System.out.println("Parameter Names found for constraint "+constraint+ ": " +names.size()+"x"); 
            
            if(names.size()==1){
                 Vector paramValues = parmDB.getRange(names.get(0).toString());
                 System.out.println("Parameter Range Values found: "+paramValues.size()+"x"); 
                 double startx = 0.0;
                 double endx = 0.0;
                 double starty = 0.0;
                 double endy = 0.0;
                 
                 if(paramValues.size()==4){
                     startx = Double.parseDouble(paramValues.get(0).toString());
                     endx =Double.parseDouble(paramValues.get(1).toString());
                     starty = Double.parseDouble(paramValues.get(2).toString());
                     endy = Double.parseDouble(paramValues.get(3).toString());
                     System.out.println("Parameter Range : startx:"+startx+" endx:" +endx+ " starty: "+starty+" endy:"+endy ); 
                 }
                 HashMap<String,Vector<Double>> values = parmDB.getValues(constraint,startx,endx,5,starty,endy,10);
                 
                 Iterator anIterator = values.keySet().iterator();
                 while(anIterator.hasNext()){
                     String aValue = (String)anIterator.next();
                     System.out.println("Parameter Value Found: "+aValue);
                     Vector<Double> valueDoubles = (Vector<Double>)values.get(aValue);
                     System.out.println("Parameter doubles inside " +aValue+": "+valueDoubles.size()+"x");
                     
                     for(int i = 0;(i<valueDoubles.size());i++){
                        System.out.println("Parameter double inside " +aValue+": "+valueDoubles.get(i)+" ");
                       
                     }
                 }
                 
            }
            
            throw new NotImplementedException("WARNING! ParmDB Implementation not fully completed"); 
	}
}
