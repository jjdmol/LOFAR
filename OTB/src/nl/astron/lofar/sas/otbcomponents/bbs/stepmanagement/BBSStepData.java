/*
 * BBSStepData.java
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

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import java.util.HashMap;
import java.util.ArrayList;

/**
 * BBS Step Data object, which serves as a data structure for a given BBS Step with a unique name.
 *
 * All the attributes of a BBS Step are present, as well as a generic collection of operation 
 * attributes which can be filled for any given operation type.
 *
 * @version $Id$
 * @created August 9, 2006, 14:32 PM
 * @author pompert
 */
public class BBSStepData{
    
    //Step Baseline Selection
    private ArrayList<String> station1Selection = null;
    private ArrayList<String> station2Selection = null;
    //Step Sources
    private ArrayList<String> sources = null;
    private ArrayList<String> extraSources = null;
    //Step Instrument Model
    private ArrayList<String> instrumentModel = null;
    //Step Integration
    private double integrationFrequency = -1.0;
    private double integrationTime = -1.0;
    //Step Correlation
    private String correlationSelection = null;
    private ArrayList<String> correlationType = null;
    //Step Output Data Column
    private String outputDataColumn = null;
    //Step Operation name
    private String operationName = null;
    //Step Operation attributes
    private HashMap<String,String> operationAttributes = null;
    
    
    
    /** Creates a new instance of BBSStep */
    public BBSStepData() {
    }
   
    public ArrayList<String> getStation1Selection(){
        return station1Selection;
    }
    
    public void setStation1Selection(ArrayList<String> station1Selection){
        this.station1Selection = station1Selection;
    }
    public ArrayList<String> getStation2Selection(){
        return station2Selection;
    }
    
    public void setStation2Selection(ArrayList<String> station2Selection){
        this.station2Selection = station2Selection;
    }
    public ArrayList<String> getSources(){
        return sources;
    }
    
    public void setSources(ArrayList<String> sources){
        this.sources = sources;
    }
    
    public ArrayList<String> getExtraSources(){
        return extraSources;
    }
    
    public void setExtraSources(ArrayList<String> extraSources){
        this.extraSources = extraSources;
    }
    
    public ArrayList<String> getInstrumentModel(){
        return instrumentModel;
    }
    
    public void setInstrumentModel(ArrayList<String> instrumentModel){
        this.instrumentModel = instrumentModel;
    }
    
    public double getIntegrationFrequency(){
        return integrationFrequency;
    }
    
    public void setIntegrationFrequency(double integrationFrequency){
        this.integrationFrequency = integrationFrequency;
    }
    
    public double getIntegrationTime(){
        return integrationTime;
    }
    
    public void setIntegrationTime(double integrationTime){
        this.integrationTime = integrationTime;
    }
    
    public String getCorrelationSelection(){
        return correlationSelection;
    }
    
    public void setCorrelationSelection(String correlationSelection){
        this.correlationSelection = correlationSelection;
    }
    
    public ArrayList<String> getCorrelationType(){
        return correlationType;
    }
    
    public void setCorrelationType(ArrayList<String> correlationType){
        this.correlationType = correlationType;
    }
    
    public String getOutputDataColumn(){
        return outputDataColumn;
    }
    
    public void setOutputDataColumn(String outputDataColumn){
        this.outputDataColumn = outputDataColumn;
    }
    
    public String getOperationName(){
        return operationName;
    }
    
    public void setOperationName(String operationName){
        this.operationName = operationName;
    }
    
    public synchronized HashMap<String,String> getOperationAttributes(){
        return operationAttributes;
    }
    
    public synchronized String getOperationAttribute(String key){
        String returnString = null;
        if(operationAttributes!=null){
            returnString=operationAttributes.get(key);
        }
        return returnString;
    }
    
    public synchronized void setOperationAttributes(HashMap<String,String> operationAttributes){
        this.operationAttributes = operationAttributes;
    }
    
    public synchronized void addOperationAttribute(String key, String value){
        if(operationAttributes==null){
            operationAttributes=new HashMap<>();
        }
        this.operationAttributes.put(key,value);
    }
    public synchronized void removeOperationAttribute(String key){
        if(operationAttributes!=null){
            this.operationAttributes.remove(key);
        }
    }
    public synchronized void removeAllOperationAttributes(){
        if(operationAttributes!=null){
            this.operationAttributes.clear();
        }
        operationAttributes = null;
    }
    public synchronized boolean containsOperationAttribute(String key){
        boolean returnBool = false;
        if(operationAttributes!=null){
            returnBool = this.operationAttributes.get(key)!=null;
        }
        return returnBool;
    }
}
