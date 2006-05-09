/*
 * PlotDataAccessParmDBImpl.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl

 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package nl.astron.lofar.java.cep.jparmfacade;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimeZone;
import java.util.Vector;
import nl.astron.lofar.java.gui.plotter.IPlotDataAccess;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessException;

/**
 * This class provides an implementation of IPlotDataAccess for use with LOFAR's
 *  jParmFacade interface. It manages connections to that interface, and allows
 * the plotter framework to generate plots of data present in the ParmDB. 
 *
 * @see nl.astron.lofar.java.cep.jparmfacade.jParmFacade
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessParmDBImpl implements IPlotDataAccess{
    
    private static final int requiredDataConstraints = 7;
    //Location of ParmDB table file(s)
    private static final String DATA_INMEP_FILE_PATH = "/home/pompert/transfer/tParmFacade.in_mep";
      
    private static jParmFacade parmDB = null;
    /**
     *Creates a new instance of PlotDataAccessParmDBImpl
     */
    public PlotDataAccessParmDBImpl(){
    
    }
    /**
     *Cleans up the instance variables
     */
    public void finalize() throws Throwable {
        parmDB = null;

    }
    
    /**
     * This method, the most important one, makes a Plotter compliant dataset using
     * the constraints provided by the PlotPanel, and to do that, it uses the 
     * jParmFacade interface to get the data.
     * 
     *
     * @param constraints The constraints provided by the PlotPanel. 
     * These must be modelled as follows:<br><br>
     * -constraints[0]= the parameter name filter (for example parm*) (String)<br>
     * -constraints[1]= the startx variable (for example 0) (double)<br>
     * -constraints[2]= the endx variable (for example 5) (double)<br>
     * -constraints[3]= the numx variable (for example 5) (int)<br>
     * -constraints[4]= the starty variable (for example 0) (double)<br>
     * -constraints[5]= the endy variable (for example 5) (double)<br>
     * -constraints[6]= the numy variable (for example 5) (int)<br><br>
     * @return the data set generated
     * @throws PlotterDataAccessException will be thrown if anything goes wrong 
     * with the ParmDB interface and calls to it.
     *
     */
    public HashMap retrieveData(String[] constraints) throws PlotterDataAccessException{
        if(parmDB == null){
            this.initiateConnectionToParmDB();
        }
        HashMap<String,Object> returnMap = new HashMap<String, Object>();
        HashSet<HashMap> returnValues = new HashSet<HashMap>();
        if(parmDB != null){
            Vector names;
            try{
                names = parmDB.getNames(constraints[0]);
            } catch (Exception ex) {
                //TODO LOG!
                PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getNames() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                exx.initCause(ex);
                throw exx;
            }
            if(names != null && names.size()>=1 && constraints.length == this.requiredDataConstraints){
                returnMap.put(PlotConstants.DATASET_NAME,"Parameter Database Filter : "+constraints[0]);
                TimeZone utcZone = TimeZone.getTimeZone("UTC");
                utcZone.setDefault(utcZone);
                Calendar aCalendar = Calendar.getInstance(utcZone);
                Date utcDate = aCalendar.getTime();
                //Test code to accomplish MJD notation
                int epoch_unix_to_julian = 2440588;
                double curDate = utcDate.getTime();
                double millis = (1000*60*60*24);
                double floorDivide = (curDate >= 0) ? curDate / millis : ((curDate+1)/millis) -1; 
                double julianDay = epoch_unix_to_julian + floorDivide;
                double modifiedJulianDay = julianDay - 2400000.5;
                returnMap.put(PlotConstants.DATASET_SUBNAME,"Generated at "+ utcDate.toString() + " MJD: "+modifiedJulianDay);
                
                //Every parameter
                for(int n = 0; n < names.size();n++){
                    Vector paramValues;
                    try{
                        paramValues = parmDB.getRange(names.get(n).toString());
                    } catch (Exception ex) {
                        //TODO LOG!
                        PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getRange() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                        exx.initCause(ex);
                        throw exx;
                    }
                    double startx = Double.parseDouble(constraints[1]);
                    double endx = Double.parseDouble(constraints[2]);
                    double starty = Double.parseDouble(constraints[4]);
                    double endy = Double.parseDouble(constraints[5]);
                    int numx = Integer.parseInt(constraints[3]);
                    int numy = Integer.parseInt(constraints[6]);

                    if(paramValues != null && paramValues.size()==4){
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
                        //TODO LOG!
                        PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getValues() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                        exx.initCause(ex);
                        throw exx;
                    }
                    //Every parameter value
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
                        
                        //Every parameter value double inside the vector
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
        }
        
        return returnMap;
    }
    
    /**
     * This method will check if the jParmFacade ParmDB interface is ready, and if not, it will
     * create a new instance of it.
     * @throws PlotterDataAccessException will be thrown if the jParmFacade could
     * not be started, due to missing native libraries or other errors.
     */
    private static void initiateConnectionToParmDB() throws PlotterDataAccessException{
         if(parmDB == null){
             try {
                System.loadLibrary("jparmfacade");
                parmDB = new jParmFacade(PlotDataAccessParmDBImpl.DATA_INMEP_FILE_PATH);
             } catch (Throwable e) {
                PlotterDataAccessException exx = new PlotterDataAccessException("The jParmFacade interface could not be initiated. Please check that you have the jParmFacade/ParmDB library somewhere in the java library path");
                exx.initCause(e);
                throw exx;
             } 
         }
    }

}