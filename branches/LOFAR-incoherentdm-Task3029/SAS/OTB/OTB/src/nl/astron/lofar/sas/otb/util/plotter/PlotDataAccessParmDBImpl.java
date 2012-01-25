/*
 * PlotDataAccessParmDBImpl.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
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

package nl.astron.lofar.sas.otb.util.plotter;

import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.TimeZone;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface;
import nl.astron.lofar.java.gui.plotter.IPlotDataAccess;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessException;
import org.apache.log4j.Logger;

/**
 * This class provides an implementation of IPlotDataAccess for use with LOFAR's
 * jParmFacade interface. It manages connections to that interface, and allows
 * the plotter framework to generate plots of data present in the ParmDB.
 *
 * @see nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessParmDBImpl implements IPlotDataAccess{
    
    private static final int requiredDataConstraints = 8;
    private static Logger logger = Logger.getLogger(PlotDataAccessParmDBImpl.class);
    
    private static jParmFacadeInterface parmDB = null;
    
    /**
     *
     *Creates a new instance of PlotDataAccessParmDBImpl
     *
     */
    
    public PlotDataAccessParmDBImpl(){
    }
    
    /**
     *Cleans up the instance variables
     *
     */
    
    @Override
    public void finalize() throws Throwable {
        parmDB = null;
    }
    
    /**
     *
     * This method, the most important one, makes a Plotter compliant dataset using
     * the constraints provided by the PlotPanel, and to do that, it uses the
     * jParmFacade interface to get the data.
     *
     * @param constraints The constraints provided by the PlotPanel.<br><br>
     *
     * This object must be modelled as follows:<br><br>
     *
     * HashMap<String,Object> constraints (This is the object you pass here)<br>
     * --(key,value) ("PARMDBINTERFACE",a jParmFacadeInterface instance object)<br>
     * --(key,value) ("PARMDBCONSTRAINTS",String[] constraints) See below on how to fill the String[]<br>
     * ----+constraints[0]= the parameter name filter (for example parm*) (String)<br>
     * ----+constraints[1]= the startx variable (for example 0) (double)<br>
     * ----+constraints[2]= the endx variable (for example 5) (double)<br>
     * ----+constraints[3]= the numx variable (for example 5) (int)<br>
     * ----+constraints[4]= the starty variable (for example 0) (double)<br>
     * ----+constraints[5]= the endy variable (for example 5) (double)<br>
     * ----+constraints[6]= the numy variable (for example 5) (int)<br>
     * ----+constraints[7]= A string that will be put in front of every value. Empty string or null at least!<br>
     * @return the data set generated
     * @see nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface
     * @see nl.astron.lofar.java.gui.plotter.PlotConstants
     * @throws PlotterDataAccessException will be thrown if anything goes wrong
     * with the ParmDB interface and calls to it.
     */
    @SuppressWarnings("unchecked")
    public HashMap<String,Object> retrieveData(Object constraints) throws PlotterDataAccessException{
        if(parmDB == null){
            PlotDataAccessParmDBImpl.initiateConnectionToParmDB(constraints);
        }
        
        HashMap<String,Object> parameterConstraints = (HashMap<String,Object>)constraints;
        String[] constraintsArray = (String[])parameterConstraints.get(new String("PARMDBCONSTRAINTS"));

        HashMap<String,Object> returnMap = new HashMap<String, Object>();

        if(parmDB != null){
            if(constraintsArray.length == PlotDataAccessParmDBImpl.requiredDataConstraints){
                String tableName = constraintsArray[7];

                LinkedList<HashMap<String,Object>> values = null;
                Vector<String> nameFilter = new Vector();
                nameFilter.add(constraintsArray[0]);
                if(tableName.equalsIgnoreCase("History")){
                    values = getParmHistoryValues(nameFilter,constraintsArray);
                }else{
                    values = getParmValues(nameFilter,constraintsArray);
                }
                
                if(values != null && values.size() > 0){
                    returnMap.put(PlotConstants.DATASET_NAME,"ParmDB dataset '"+constraintsArray[0]+"'");

                    TimeZone utcZone = TimeZone.getTimeZone("UTC");
                    TimeZone.setDefault(utcZone);
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

                    if(tableName.equalsIgnoreCase("History")){
                        returnMap.put(PlotConstants.DATASET_XAXISLABEL,"Iteration");
                        returnMap.put(PlotConstants.DATASET_XAXISUNIT,"");
                        returnMap.put(PlotConstants.DATASET_XAXISTYPE,"SPATIAL");
                        returnMap.put(PlotConstants.DATASET_YAXISLABEL,"Value");
                        returnMap.put(PlotConstants.DATASET_YAXISUNIT,"");
                        returnMap.put(PlotConstants.DATASET_YAXISTYPE,"SPATIAL");
                        returnMap.put(PlotConstants.DATASET_VALUES,values);
                    }else{
                        returnMap.put(PlotConstants.DATASET_XAXISLABEL,"Frequency");
                        returnMap.put(PlotConstants.DATASET_XAXISUNIT,"(Hz)");
                        returnMap.put(PlotConstants.DATASET_XAXISTYPE,"SPATIAL");
                        returnMap.put(PlotConstants.DATASET_YAXISLABEL,"Bandpass Gain");
                        returnMap.put(PlotConstants.DATASET_YAXISUNIT,"");
                        returnMap.put(PlotConstants.DATASET_YAXISTYPE,"SPATIAL");
                        returnMap.put(PlotConstants.DATASET_VALUES,values);
                    }
                    
                }else{
                    throw new PlotterDataAccessException("No results were found in the ParmDB table(s) using the given parameter name filter ( " + nameFilter + " )");
                }
                    
            }else{
                throw new PlotterDataAccessException("An invalid amount of parameters (" +constraintsArray.length+" instead of " +PlotDataAccessParmDBImpl.requiredDataConstraints+") were passed to the ParmDB Data Access Interface");
            }
        }
        return returnMap;
    }
    /**
     * This method updates an already existing Plotter compliant dataset using
     * the constraints provided by the PlotPanel, and to do that, it uses the
     * jParmFacade interface to get the data.
     * @param currentData The current data set to be updated.
     * @param constraints The update constraints provided by the PlotPanel.<br>
     * This object must be modelled as follows:<br><br>
     *
     * HashMap<String,Object> constraints (This is the object you pass here)<br>
     * --(key,value) ("PARMDBINTERFACE",a jParmFacadeInterface instance object)<br><br>
     * --One of the following operators:<br><br>
     * --(key,value) (PlotConstants.DATASET_OPERATOR_ADD,HashMap<String,Object> addDataSet)<br>
     * ----addDataSet(key,value) ("PARMDBCONSTRAINTS",String[] constraints) See below on how to fill the String[]<br>
     * ------+constraints[0]= the parameter name filter (for example parm*) (String)<br>
     * ------+constraints[1]= the startx variable (for example 0) (double)<br>
     * ------+constraints[2]= the endx variable (for example 5) (double)<br>
     * ------+constraints[3]= the numx variable (for example 5) (int)<br>
     * ------+constraints[4]= the starty variable (for example 0) (double)<br>
     * ------+constraints[5]= the endy variable (for example 5) (double)<br>
     * ------+constraints[6]= the numy variable (for example 5) (int)<br>
     * ------+constraints[7]= A string that will be put in front of every value. Empty string at least!<br>
     * --(key,value) (PlotConstants.DATASET_OPERATOR_DELETE,String[] dataIdentifiers) See below on how to fill the String[]<br>
     * ----+dataIdentifiers[0-n]= the parameter name to delete from the plot (String)<br>
     * --(key,value) ("DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_ALL_LINES",null)<br>
     * --(key,value) ("DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_LINE",String[] dataIdentifiers) See below on how to fill the String[]<br>
     * ----+dataIdentifiers[0]= the parameter value from which to subtract the mean from all values(String)<br>
     * --(key,value) ("DATASET_OPERATOR_SUBTRACT_LINE",String[] dataIdentifiers) See below on how to fill the String[]<br>
     * ----+dataIdentifiers[0]= the parameter value which you would like to subtract from all other values(String)<br>
     * --(key,value) ("DATASET_OPERATOR_ADD_Y_OFFSET",String[] offset) See below on how to fill the String[]<br>
     * ----+offset[0]= the offset value which you would like to add to all values(String representation of double)<br>
     * --(key,value) ("DATASET_OPERATOR_REMOVE_Y_OFFSET",String[] offset) See below on how to fill the String[]<br>
     * ----+offset[0]= the offset value which you would like to remove from all values(String representation of double)<br>
     * <br>
     * @return the data set generated
     * @see nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface
     * @see nl.astron.lofar.java.gui.plotter.PlotConstants
     * @throws PlotterDataAccessException will be thrown if anything goes wrong
     * with the ParmDB interface and calls to it.
     *
     */
    @SuppressWarnings("unchecked")
    public HashMap<String,Object> updateData(HashMap<String,Object> currentData, Object constraints) throws PlotterDataAccessException{
        
        if(parmDB == null){
            PlotDataAccessParmDBImpl.initiateConnectionToParmDB(constraints);
        }

        try{
            LinkedList<HashMap<String,Object>> currentValuesInPlot = (LinkedList<HashMap<String,Object>>)currentData.get(PlotConstants.DATASET_VALUES);
            HashMap<String,Object> operatorsOnDataset = (HashMap<String,Object>)constraints;
            
            if(operatorsOnDataset.containsKey(PlotConstants.DATASET_OPERATOR_ADD)){
                HashMap<String,Object> addDataSet = (HashMap<String,Object>)operatorsOnDataset.get(PlotConstants.DATASET_OPERATOR_ADD);
                String[] constraintsArray = (String[])addDataSet.get("PARMDBCONSTRAINTS");

                if(constraintsArray.length == PlotDataAccessParmDBImpl.requiredDataConstraints){
                    String tableName = constraintsArray[7];

                    LinkedList<HashMap<String,Object>> newParmValues = null;
                    Vector<String> nameFilter = new Vector();
                    nameFilter.add(constraintsArray[0]);
                    if(tableName.equalsIgnoreCase("History")){
                        newParmValues = getParmHistoryValues(nameFilter,constraintsArray);
                    }else{
                        newParmValues = getParmValues(nameFilter,constraintsArray);
                    }
                
                    if(newParmValues != null && newParmValues.size() > 0){
                        HashSet<HashMap<String,Object>> toBeAddedValueObjects = new HashSet<HashMap<String,Object>>();
                        
                        for(HashMap<String,Object> parmValue : newParmValues){
                            boolean addData = true;
                            for(HashMap<String,Object> parmValue2 : currentValuesInPlot){
                                String compareValue = (String)parmValue2.get(PlotConstants.DATASET_VALUELABEL);
                                if(parmValue.get(PlotConstants.DATASET_VALUELABEL).equals(compareValue)){
                                    addData = false;
                                }
                            }
                            if(addData){
                                toBeAddedValueObjects.add(parmValue);
                            }
                        }
                        currentValuesInPlot.addAll(toBeAddedValueObjects);
                    }else{
                        throw new PlotterDataAccessException("No results were found in the ParmDB table(s) using the given parameter name filter ( " + nameFilter + " )");
                    }
                    
                }else{
                    throw new PlotterDataAccessException("An invalid amount of parameters (" +constraintsArray.length+" instead of " +PlotDataAccessParmDBImpl.requiredDataConstraints+") were passed to the ParmDB Data Access Interface");
                }
            } else if(operatorsOnDataset.containsKey(PlotConstants.DATASET_OPERATOR_MODIFY)){
                
                String[] constraintsArray = (String[])operatorsOnDataset.get(PlotConstants.DATASET_OPERATOR_MODIFY);
                
            } else if(operatorsOnDataset.containsKey(PlotConstants.DATASET_OPERATOR_DELETE)){
                HashSet<HashMap<String,Object>> toBeDeletedValueObjects = new HashSet<HashMap<String,Object>>();
                String[] toBeDeletedValues = (String[])operatorsOnDataset.get(PlotConstants.DATASET_OPERATOR_DELETE);
                for(int i = 0; i < toBeDeletedValues.length; i++){
                    String aValueToBeDeleted = toBeDeletedValues[i];
                    for(HashMap<String,Object> dataObject : currentValuesInPlot){
                        if(dataObject.get(PlotConstants.DATASET_VALUELABEL).equals(aValueToBeDeleted)){
                            toBeDeletedValueObjects.add(dataObject);
                        }
                    }
                }
                for(HashMap<String,Object> deleteObject : toBeDeletedValueObjects){
                    currentValuesInPlot.remove(deleteObject);
                }
            } else if(operatorsOnDataset.containsKey("DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_ALL_LINES")){
                
                HashMap<String,Object> firstValue = currentValuesInPlot.getFirst();
                double[] firstValueYArray =  (double[])firstValue.get(PlotConstants.DATASET_YVALUES);
                int totalNumberOfYIterations = firstValueYArray.length;
                //for each Y value
                for(int yIteration = 0; yIteration < totalNumberOfYIterations; yIteration++){
                    
                    double yMeanForIteration = 0.0;
                    //add value of a line to the total for this Y iteration
                    for(HashMap<String,Object> aValue : currentValuesInPlot){
                        
                        double[] yArray =  (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                        yMeanForIteration +=  yArray[yIteration];
                        
                    }
                    //devide the total value for this Y iteration by total amount of values
                    
                    yMeanForIteration = yMeanForIteration / currentValuesInPlot.size();
                    //subtract the mean amount for this Y iteration from all values
                    for(HashMap<String,Object> aValue : currentValuesInPlot){
                        
                        double[] yArray =  (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                        yArray[yIteration] = yArray[yIteration] - yMeanForIteration;
                        
                    }
                }
                //update the titles of every value
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    String title = ((String)aValue.get(PlotConstants.DATASET_VALUELABEL));
                    title += " MINUS mean(all values)";
                    aValue.put(PlotConstants.DATASET_VALUELABEL,title);
                }
            } else if(operatorsOnDataset.containsKey("DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_LINE")){
                String[] toBeSubtractedValues = (String[])operatorsOnDataset.get("DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_LINE");
                HashMap<String,Object> firstValue = currentValuesInPlot.getFirst();
                double[] firstValueYArray =  (double[])firstValue.get(PlotConstants.DATASET_YVALUES);
                int totalNumberOfYIterations = firstValueYArray.length;
                //for each Y value
                
                for(int yIteration = 0; yIteration < totalNumberOfYIterations; yIteration++){
                    
                    double yMeanForIteration = 0.0;
                    //add value of a line to the total for this Y iteration
                    
                    for(HashMap<String,Object> aValue : currentValuesInPlot){
                        
                        double[] yArray =  (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                        yMeanForIteration +=  yArray[yIteration];
                        
                    }
                    //devide the total value for this Y iteration by total amount of values
                    yMeanForIteration = yMeanForIteration / currentValuesInPlot.size();
                    //subtract the mean amount for this Y iteration from given value
                    for(HashMap<String,Object> aValue : currentValuesInPlot){
                        if(((String)aValue.get(PlotConstants.DATASET_VALUELABEL)).equalsIgnoreCase(toBeSubtractedValues[0])){
                            double[] yArray =  (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                            yArray[yIteration] = yArray[yIteration] - yMeanForIteration;
                        }
                    }
                }
                //only update the value given
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    if(((String)aValue.get(PlotConstants.DATASET_VALUELABEL)).equalsIgnoreCase(toBeSubtractedValues[0])){
                        String title = ((String)aValue.get(PlotConstants.DATASET_VALUELABEL));
                        title += " MINUS mean(all values)";
                        aValue.put(PlotConstants.DATASET_VALUELABEL,title);
                    }
                }
            } else if(operatorsOnDataset.containsKey("DATASET_OPERATOR_SUBTRACT_LINE")){
                String[] toBeSubtractedValues = (String[])operatorsOnDataset.get("DATASET_OPERATOR_SUBTRACT_LINE");
                double[] firstValueYArray =  null;
                
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    String label = (String)aValue.get(PlotConstants.DATASET_VALUELABEL);
                    if(label.equalsIgnoreCase(toBeSubtractedValues[0])){
                        double[] originValueYArray = (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                        //create copy of value array to prevent subtracting by zero
                        firstValueYArray = new double[originValueYArray.length];
                        for(int i = 0; i < firstValueYArray.length; i++){
                            firstValueYArray[i] = originValueYArray[i];
                        }
                    }
                }
                
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    
                    double[] yArray =  (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                    for(int subtractI = 0; subtractI < yArray.length; subtractI++){
                        yArray[subtractI] =  yArray[subtractI]-firstValueYArray[subtractI];
                    }
                    String title = ((String)aValue.get(PlotConstants.DATASET_VALUELABEL));
                    title += " MINUS ("+toBeSubtractedValues[0]+")";
                    aValue.put(PlotConstants.DATASET_VALUELABEL,title);
                    
                }
            } else if(operatorsOnDataset.containsKey("DATASET_OPERATOR_ADD_Y_OFFSET")){
                String[] toBeSubtractedValues = (String[])operatorsOnDataset.get("DATASET_OPERATOR_ADD_Y_OFFSET");
                double offset = Double.parseDouble(toBeSubtractedValues[0]);
                
                int valueDone = 0;
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    valueDone++;
                    double[] originValueYArray = (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                    for(int i = 0;i<originValueYArray.length;i++){
                        originValueYArray[i] = originValueYArray[i]+(offset*valueDone);
                    }
                    String title = ((String)aValue.get(PlotConstants.DATASET_VALUELABEL));
                    title += " OFFSET("+DecimalFormat.getInstance().format(offset*valueDone)+")";
                    aValue.put(PlotConstants.DATASET_VALUELABEL,title);
                }
            } else if(operatorsOnDataset.containsKey("DATASET_OPERATOR_REMOVE_Y_OFFSET")){
                String[] toBeSubtractedValues = (String[])operatorsOnDataset.get("DATASET_OPERATOR_REMOVE_Y_OFFSET");
                double offset = Double.parseDouble(toBeSubtractedValues[0]);
                
                int valueDone = 0;
                for(HashMap<String,Object> aValue : currentValuesInPlot){
                    valueDone++;
                    double[] originValueYArray = (double[])aValue.get(PlotConstants.DATASET_YVALUES);
                    for(int i = 0;i<originValueYArray.length;i++){
                        originValueYArray[i] = originValueYArray[i]-(offset*valueDone);
                    }
                    String title = ((String)aValue.get(PlotConstants.DATASET_VALUELABEL));
                    logger.trace("Old title with offset was :"+title);
                    String offsetString = " OFFSET\\("+DecimalFormat.getInstance().format(offset*valueDone)+"\\)";
                    logger.trace("New title will be stripped of : "+offsetString);
                    Pattern pat=Pattern.compile(offsetString);
                    
                    Matcher matcher=pat.matcher(title);
                    String newTitle = matcher.replaceAll("");
                    /*int lastIndexOfOffset = title.lastIndexOf("OFFSET ("+(offset*valueDone)+")");
                    String newTitle = title.substring(0,lastIndexOfOffset);
                     */
                    logger.trace("New title without offset is :"+newTitle);
                    aValue.put(PlotConstants.DATASET_VALUELABEL,newTitle);
                }
            }
        }catch(Exception e){
            
            PlotterDataAccessException ex = new PlotterDataAccessException("An error occurred while updating the dataset! : "+e.getMessage());
            ex.initCause(e);
            logger.error(ex);
            e.printStackTrace();
            throw ex;
            
        }
        
        return currentData;
        
    }
    
    /**
     *
     * This method will check if the jParmFacadeInterface is present<br>
     * in the constraints object passed to this class through the retrieveData<br>
     * and updateData methods.
     *
     * @throws PlotterDataAccessException will be thrown if the jParmFacadeInterface could not be accessed
     */
    @SuppressWarnings("unchecked")
    private static void initiateConnectionToParmDB(Object constraints) throws PlotterDataAccessException{
        
        if(parmDB == null){
            
            try {
                
                HashMap<String,Object> parameterConstraints = (HashMap<String,Object>)constraints;
                
                parmDB = (jParmFacadeInterface)parameterConstraints.get(new String("PARMDBINTERFACE"));
                
            } catch (Throwable e) {
                
                PlotterDataAccessException exx = new PlotterDataAccessException("The jParmFacade interface could not be initiated. Please check if you have a working jParmFacade server and interface");
                
                exx.initCause(e);
                
                logger.error(exx);
                throw exx;
                
            }
            
        }
        
    }
    /**
     *Helper method that gets a vector of names from the jParmFacade interface
     *@param namefilter Name filter to be sent to ParmDB.
     *@return vector of Names
     */
    private Vector getNames(String namefilter) throws PlotterDataAccessException{
        Vector names;
        
        try{
            
            names = parmDB.getNames(namefilter);
            
        } catch (Exception ex) {
            
            //TODO LOG!
            
            PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getNames() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
            
            exx.initCause(ex);
            
            logger.error(exx);
            throw exx;
            
        }
        return names;
    }
    /**
     * Helper method that generates a LinkedList with values from the jParmFacade interface
     * @param names filter to be sent to ParmDB.
     * @return vector of Names
     */
    private LinkedList<HashMap<String,Object>> getParmValues(Vector names, String[] constraintsArray) throws PlotterDataAccessException{
        LinkedList<HashMap<String,Object>> returnList = new LinkedList<HashMap<String,Object>>();
        
        for(int n = 0; n < names.size();n++){
            
            Vector paramValues;
            
            try{
                
                paramValues = parmDB.getRange(names.get(n).toString());
                
                
            } catch (Exception ex) {
                
                PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getRange() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                
                exx.initCause(ex);
                
                logger.error(exx);
                throw exx;
                
            }
            
            double startx = Double.parseDouble(constraintsArray[1]);
            
            double endx = Double.parseDouble(constraintsArray[2]);
            
            double starty = Double.parseDouble(constraintsArray[4]);
            
            double endy = Double.parseDouble(constraintsArray[5]);
            
            int numx = Integer.parseInt(constraintsArray[3]);
            
            int numy = Integer.parseInt(constraintsArray[6]);
            
            
            /*
            returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_START,Double.toString(startx));
             
            returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_END,Double.toString(endx));
             
            returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_START,Double.toString(starty));
             
            returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_END,Double.toString(endy));
             */
            
            
            HashMap<String, Vector<Double>> values = new HashMap<String,Vector<Double>>();
            
            try {
                
                values = parmDB.getValues((names.get(n)).toString(), startx, endx, numx, starty, endy, numy);
                
            } catch (Exception ex) {
                
                //TODO LOG!
                
                PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getValues() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                
                exx.initCause(ex);
                
                logger.error(exx);
                throw exx;
                
            }
            
            //Every parameter value
            for(String aValue : values.keySet()){
                
                HashMap<String,Object> aValueMap = new HashMap<String,Object>();
                
                logger.debug("Parameter Value Found: "+aValue);
                
                if(constraintsArray[7] == null || constraintsArray[7].equalsIgnoreCase("")){
                    aValueMap.put(PlotConstants.DATASET_VALUELABEL,aValue);
                }else{
                    aValueMap.put(PlotConstants.DATASET_VALUELABEL,constraintsArray[7]+" - "+aValue);
                }
                
                Vector<Double> valueDoubles = (Vector<Double>)values.get(aValue);
                
                logger.debug("Parameter doubles inside " +aValue+": "+valueDoubles.size()+"x");
                
                double[] xArray = new double[valueDoubles.size()];
                
                double[] yArray = new double[valueDoubles.size()];
                
                
                
                //Every parameter value double inside the vector
                
                for(int i = 0;(i<valueDoubles.size());i++){
                    
                    xArray[i] = startx + (endx-startx) / valueDoubles.size()*(i+0.5);
                    
                    yArray[i] = valueDoubles.get(i);
                    
                }
                
                aValueMap.put(PlotConstants.DATASET_XVALUES,xArray);
                
                aValueMap.put(PlotConstants.DATASET_YVALUES,yArray);
                
                returnList.add(aValueMap);
                
            }
            
        }
        return returnList;
    }
    
    /**
     * Helper method that generates a LinkedList with values from the jParmFacade interface
     * @param names filter to be sent to ParmDB.
     * @return vector of Names
     */
    private LinkedList<HashMap<String,Object>> getParmHistoryValues(Vector names, String[] constraintsArray) throws PlotterDataAccessException{
        LinkedList<HashMap<String,Object>> returnList = new LinkedList<HashMap<String,Object>>();
        
        for(int n = 0; n < names.size(); n++) {
            /*
            Vector paramValues;
             
            try
            {
                paramValues = parmDB.getRange(names.get(n).toString());
            }
            catch (Exception ex)
            {
                PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getRange() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                exx.initCause(ex);
                logger.error(exx);
                throw exx;
            }
             
             
            double startx = 0.0;
             
            double endx = 1.0e25;
             
            double starty = 0.0;
             
            double endy = 1.0e25;
             
             */
            double startx = Double.parseDouble(constraintsArray[1]);
            
            double endx = Double.parseDouble(constraintsArray[2]);
            
            double starty = Double.parseDouble(constraintsArray[4]);
            
            double endy = Double.parseDouble(constraintsArray[5]);
            
            /*
            double startSolveTime = Double.parseDouble(constraintsArray[5]);
             
            double endSolveTime = Double.parseDouble(constraintsArray[6]);
             */
            
            /*
            returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_START,Double.toString(startx));
             
            returnMap.put(PlotConstants.DATASET_XAXIS_RANGE_END,Double.toString(endx));
             
            returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_START,Double.toString(starty));
             
            returnMap.put(PlotConstants.DATASET_YAXIS_RANGE_END,Double.toString(endy));
             */
            
            
            HashMap<String, Vector<Double>> values = new HashMap<String,Vector<Double>>();
            try {
                values = parmDB.getHistory((names.get(n)).toString(), startx, endx, starty, endy, 0.0, 1e25);
            } catch (Exception ex) {
                //TODO LOG!
                PlotterDataAccessException exx = new PlotterDataAccessException("An invalid getHistory() call was made to the ParmDB interface. Please check that all variables seem OK. Root cause: "+ex.getMessage());
                exx.initCause(ex);
                logger.error(exx);
                throw exx;
            }
            
            //Every parameter value
            for(String aValue : values.keySet()) {
                Vector<Double> valueDoubles = (Vector<Double>)values.get(aValue);
                
                //int coefficientCount = valueDoubles.get(0).intValue();
                //int iterationCount = (valueDoubles.size() - 1) / coefficientCount;
                
                //logger.debug(aValue + ": #coefficients=" + coefficientCount + " #iterations=" + iterationCount);
                
                HashMap<String,Object> aValueMap = new HashMap<String,Object>();
                
                logger.debug("Parameter Value Found: "+ aValue);
                
                if(constraintsArray[7] == null || constraintsArray[7].equalsIgnoreCase("")) {
                    aValueMap.put(PlotConstants.DATASET_VALUELABEL, aValue);
                } else {
                    aValueMap.put(PlotConstants.DATASET_VALUELABEL, constraintsArray[7] + " - " + aValue);
                }
                
                double[] xArray = new double[valueDoubles.size()];
                double[] yArray = new double[valueDoubles.size()];
                
                for(int i = 0;(i<valueDoubles.size());i++){
                    xArray[i] = i;
                    yArray[i] = valueDoubles.get(i);
                }
                
                aValueMap.put(PlotConstants.DATASET_XVALUES,xArray);
                aValueMap.put(PlotConstants.DATASET_YVALUES,yArray);
                returnList.add(aValueMap);
            }
        }
        return returnList;
    }
}
