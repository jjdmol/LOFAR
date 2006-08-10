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

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;

/**
 * @version $Id$
 * @created August 9, 2006, 14:32 PM
 * @author pompert
 */
public class BBSStepData{
    
    //Step Baseline Selection
    private Vector<String> station1Selection = null;
    private Vector<String> station2Selection = null;
    //Step Sources
    private Vector<String> sources = null;
    private Vector<String> extraSources = null;
    //Step Instrument Model
    private Vector<String> instrumentModel = null;
    //Step Integration
    private double integrationFrequency = -1;
    private double integrationTime = -1;
    //Step Correlation
    private String correlationSelection = null;
    private Vector<String> correlationType = null;
    //Step Output Data Column
    private String outputDataColumn = null;
    
    //TODO: Step Operation Types
    
    
    /** Creates a new instance of BBSStep */
    public BBSStepData() {
        
    }
   
    public Vector<String> getStation1Selection(){
        return station1Selection;
    }
    
    public void setStation1Selection(Vector<String> station1Selection){
        this.station1Selection = station1Selection;
    }
    public Vector<String> getStation2Selection(){
        return station2Selection;
    }
    
    public void setStation2Selection(Vector<String> station2Selection){
        this.station2Selection = station2Selection;
    }
    public Vector<String> getSources(){
        return sources;
    }
    
    public void setSources(Vector<String> sources){
        this.sources = sources;
    }
    
    public Vector<String> getExtraSources(){
        return extraSources;
    }
    
    public void setExtraSources(Vector<String> extraSources){
        this.extraSources = extraSources;
    }
    
    public Vector<String> getInstrumentModel(){
        return instrumentModel;
    }
    
    public void setInstrumentModel(Vector<String> instrumentModel){
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
    
    public Vector<String> getCorrelationType(){
        return correlationType;
    }
    
    public void setCorrelationType(Vector<String> correlationType){
        this.correlationType = correlationType;
    }
    
    public String getOutputDataColumn(){
        return outputDataColumn;
    }
    
    public void setOutputDataColumn(String outputDataColumn){
        this.outputDataColumn = outputDataColumn;
    }

    public BBSStepData clone(){
        BBSStepData newStep = new BBSStepData();
        newStep.setStation1Selection(this.getStation1Selection());
        newStep.setStation2Selection(this.getStation2Selection());
        newStep.setSources(this.getSources());
        newStep.setExtraSources(this.getExtraSources());
        newStep.setInstrumentModel(this.getInstrumentModel());
        newStep.setIntegrationFrequency(this.getIntegrationFrequency());
        newStep.setIntegrationTime(this.getIntegrationTime());
        newStep.setCorrelationSelection(this.getCorrelationSelection());
        newStep.setCorrelationType(this.getCorrelationType());
        newStep.setOutputDataColumn(this.getOutputDataColumn());
        newStep.setCorrelationType(this.getCorrelationType());
        
        return newStep;
    }
}
