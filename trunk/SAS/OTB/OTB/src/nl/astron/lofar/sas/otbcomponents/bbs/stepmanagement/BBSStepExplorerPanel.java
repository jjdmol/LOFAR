/*
 *  BBSStepExplorerPanel.java
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
 */

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.util.HashMap;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.table.DefaultTableModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStep;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepDataManager;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations.IBBSStepOperationPanel;
import org.apache.log4j.Logger;

/**
 * Panel for BBS Step Explorer
 *
 * @created 11-07-2006, 13:37
 * @author  pompert
 * @version $Id$
 */
public class BBSStepExplorerPanel extends javax.swing.JPanel{
    
    static Logger logger = Logger.getLogger(BBSStepExplorerPanel.class);
    public final static Color INHERITED_FROM_PARENT = new Color(255,255,204);
    public final static Color NOT_DEFINED = new Color(255,204,204);
    public final static Color NOT_INHERITED_FROM_PARENT = new Color(204,255,204);
    public final static Color DEFAULT = Color.WHITE;
    
    private static HashMap<String,String> stepOperationPanels = new HashMap<String,String>();
    private IBBSStepOperationPanel currentStepOperationsPanel = null;
            
    private BBSStep itsBBSStep = null;
    private BBSStep itsParentBBSStep = null;
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    
    /** 
     * Creates new BBSStepExplorerPanel 
     */
    public BBSStepExplorerPanel() {
        initComponents();
        initialize();
    }
    /**
     * Sets the OTB MainFrame link.
     *
     * @parm aMainFrame the OTB MainFrame instance to set.
     */
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    /**
     * Fills the panel with a certain BBSStep object, and displays inherited data from a given parent BBSStep object
     *
     * @parm itsStep the BBSStep object to display in the panel and to modify, null if adding a new Step
     * @parm itsParentBBSStep when adding a new step, this parent BBSStep can be 
     * displayed to show default values, null if no parent BBSStep can be defined.
     */
    public void setBBSStepContent(BBSStep itsStep,BBSStep itsParentBBSStep) {
        
        if(itsStep != null && itsParentBBSStep == null){
            //modify a step
            fillBBSGui(itsStep);
            stepExplorerStepNameText.setEditable(false);
            itsBBSStep = itsStep;
            this.stepExplorerRevertButton.setEnabled(true);
            
        }else if(itsStep == null && itsParentBBSStep != null){
            //creating a new step under a given parent step
            this.itsParentBBSStep = itsParentBBSStep;
            //fill the GUI with the current values of the parent in which the
            //new step will be situated to ease data entry...
            fillBBSGui(itsParentBBSStep);
            stepExplorerStepNameText.setText("");
            
            this.stepExplorerRevertButton.setEnabled(false);
            
        }else if (itsStep ==null && itsParentBBSStep == null){
            //assume a new strategy step will be generated
            this.stepExplorerRevertButton.setEnabled(false);
        }
    }
    
    /** 
     * Helper method that restores original values in the panel, only when modifying a BBSStep
     */
    private void restoreBBSStepExplorerPanel() {
        
        // Global Settings parameters
        if(itsBBSStep != null){
            this.stepExplorerStepNameText.setText(itsBBSStep.getName());
            this.fillBBSGui(itsBBSStep);
        }
    }
    
    /**
     * Helper method that initializes the panel
     */
    private void initialize() {
        buttonPanel1.addButton("Apply step and close");
        buttonPanel1.addButton("Close");
        if(itsBBSStep == null){
            stepExplorerStepNameText.setEditable(true);
        }
        this.stepExplorerNSourcesList.setModel(new DefaultListModel());
        this.stepExplorerInstrumentModelList.setModel(new DefaultListModel());
//        this.stepExplorerESourcesList.setModel(new DefaultListModel());
        
        //fill the supported step operation panels
        stepOperationPanels.put("Predict",null);
        stepOperationPanels.put("Solve","nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations.BBSStepOperationPanelSolveImpl");
        stepOperationPanels.put("Subtract",null);
        stepOperationPanels.put("Correct",null);
        
        // Fill the comboboxes (will need to be done from Database later)
        stepExplorerModifyNSourceCombobox.removeAllItems();
        stepExplorerModifyNSourceCombobox.addItem("CygA");
        stepExplorerModifyNSourceCombobox.addItem("CasA");
        stepExplorerModifyNSourceCombobox.addItem("TauA");
        stepExplorerModifyNSourceCombobox.addItem("VirA");
        stepExplorerModifyNSourceCombobox.addItem("HerA");
        stepExplorerModifyNSourceCombobox.addItem("PerA");
        stepExplorerModifyNSourceCombobox.addItem("3C123");
        stepExplorerModifyNSourceCombobox.addItem("3C134");
        stepExplorerModifyNSourceCombobox.addItem("3C157");
        stepExplorerModifyNSourceCombobox.addItem("3C196");
        stepExplorerModifyNSourceCombobox.addItem("3C219");
        stepExplorerModifyNSourceCombobox.addItem("3C295");
        stepExplorerModifyNSourceCombobox.addItem("3C343");
        stepExplorerModifyNSourceCombobox.addItem("3C343.1");
        stepExplorerModifyNSourceCombobox.addItem("3C353");
        stepExplorerModifyNSourceCombobox.addItem("3C363.1");
        stepExplorerModifyNSourceCombobox.addItem("3C400");
        
        stepExplorerModifyInstrumentModelCombobox.removeAllItems();
        stepExplorerModifyInstrumentModelCombobox.addItem("TOTALGAIN");
        stepExplorerModifyInstrumentModelCombobox.addItem("PATCHGAIN");
        stepExplorerModifyInstrumentModelCombobox.addItem("BANDPASS");
    }
    
    /**
     * Helper method that sets all the different fields in the GUI using the data of a given BBSStep
     *
     * @parm theBBSStep the BBSStep object to show data of.
     */
    private void fillBBSGui(BBSStep theBBSStep) {
        
        this.stepExplorerStepNameText.setText(theBBSStep.getName());
        BBSStepData stepData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getName());
        BBSStepData inheritedData = BBSStepDataManager.getInstance().getInheritedStepData(theBBSStep);
        
        //sources
        stepExplorerNSources.setBackground(DEFAULT);
        this.fillList(this.stepExplorerNSourcesList,new Vector<String>());
        if(stepData.getSources() != null){
            if(stepData.getSources().size()>0){
                this.useAllSourcesCheckbox.setSelected(false);
                this.fillList(this.stepExplorerNSourcesList,stepData.getSources());
                
            }else{
                this.useAllSourcesCheckbox.setSelected(true);
            }
            stepExplorerNSources.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getSources() != null){
                
                if(inheritedData.getSources().size()>0){
                    this.useAllSourcesCheckbox.setSelected(false);
                    this.fillList(this.stepExplorerNSourcesList,inheritedData.getSources());
                    
                }else{
                    this.useAllSourcesCheckbox.setSelected(true);
                }
                stepExplorerNSources.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.useAllSourcesCheckbox.setSelected(false);
                stepExplorerNSources.setBackground(NOT_DEFINED);
            }
        }

        //instrument model 
        stepExplorerInstrumentModel.setBackground(DEFAULT);
        this.fillList(this.stepExplorerInstrumentModelList,new Vector<String>());
        if(stepData.getInstrumentModel() != null){
            if(stepData.getInstrumentModel().size()>0){
                this.noInstrumentModelCheckbox.setSelected(false);
                this.fillList(this.stepExplorerInstrumentModelList,stepData.getInstrumentModel());
                
            }else{
                this.noInstrumentModelCheckbox.setSelected(true);
            }
            stepExplorerInstrumentModel.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getInstrumentModel() != null){
                
                if(inheritedData.getInstrumentModel().size()>0){
                    this.noInstrumentModelCheckbox.setSelected(false);
                    this.fillList(this.stepExplorerInstrumentModelList,inheritedData.getInstrumentModel());
                    
                }else{
                    this.noInstrumentModelCheckbox.setSelected(true);
                }
                stepExplorerInstrumentModel.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.noInstrumentModelCheckbox.setSelected(false);
                stepExplorerInstrumentModel.setBackground(NOT_DEFINED);
            }
        }

//        //extra sources
//        stepExplorerESources.setBackground(DEFAULT);
//        this.stepExplorerModifyESourceText.setText("");
//        this.fillList(this.stepExplorerESourcesList,new Vector<String>());
//        if(stepData.getExtraSources() != null){
//            if(stepData.getExtraSources().size()>0){
//                this.useExtraSourcesCheckbox.setSelected(true);
//                this.fillList(this.stepExplorerESourcesList,stepData.getExtraSources());
               
//            }else{
//                this.useExtraSourcesCheckbox.setSelected(false);
//            }
//            stepExplorerESources.setBackground(NOT_INHERITED_FROM_PARENT);
            
//        }else{
//            if(inheritedData.getExtraSources() != null){
//                if(inheritedData.getExtraSources().size()>0){
//                    this.useExtraSourcesCheckbox.setSelected(true);
//                    this.fillList(this.stepExplorerESourcesList,inheritedData.getExtraSources());
                    
//                }else{
//                    this.useExtraSourcesCheckbox.setSelected(false);
//                }
//                stepExplorerESources.setBackground(INHERITED_FROM_PARENT);
//            }else{
//                this.useExtraSourcesCheckbox.setSelected(true);
//                stepExplorerESources.setBackground(NOT_DEFINED);
//            }
//        }
        //output data column
        
        this.stepExplorerOutputDataPanel.setBackground(DEFAULT);
        this.stepExplorerOutputDataText.setText("");
        if(stepData.getOutputDataColumn() != null){
            if(!stepData.getOutputDataColumn().equals("")){
                this.writeOutputCheckbox.setSelected(false);
                this.stepExplorerOutputDataText.setText(stepData.getOutputDataColumn());
                
            }else{
                this.writeOutputCheckbox.setSelected(true);
            }
            stepExplorerOutputDataPanel.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getOutputDataColumn() != null){
                if(!inheritedData.getOutputDataColumn().equals("")){
                    this.writeOutputCheckbox.setSelected(false);
                    this.stepExplorerOutputDataText.setText(inheritedData.getOutputDataColumn());
                    
                }else{
                    this.writeOutputCheckbox.setSelected(true);
                }
                stepExplorerOutputDataPanel.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.writeOutputCheckbox.setSelected(false);
                stepExplorerOutputDataPanel.setBackground(NOT_DEFINED);
            }
        }

        
        //time
        this.integrationTimeText.setBackground(DEFAULT);
        if(stepData.getIntegrationTime() != -1.0){
            this.integrationTimeText.setText(""+stepData.getIntegrationTime());
            this.integrationTimeText.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getIntegrationTime() != -1.0){
                this.integrationTimeText.setText(""+inheritedData.getIntegrationTime());
                integrationTimeText.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.integrationTimeText.setText("");
                integrationTimeText.setBackground(NOT_DEFINED);
            }
        }
        //frequency
        this.integrationFrequencyText.setBackground(DEFAULT);
        if(stepData.getIntegrationFrequency() != -1.0){
            this.integrationFrequencyText.setText(""+stepData.getIntegrationFrequency());
            this.integrationFrequencyText.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getIntegrationFrequency() != -1.0){
                this.integrationFrequencyText.setText(""+inheritedData.getIntegrationFrequency());
                integrationFrequencyText.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.integrationFrequencyText.setText("");
                integrationFrequencyText.setBackground(NOT_DEFINED);
            }
        }

        //correlation
        //type
        this.stepExplorerCorrelationTypeList.setBackground(DEFAULT);
        if(stepData.getCorrelationType() != null){
            this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,stepData.getCorrelationType());
            this.stepExplorerCorrelationTypeList.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getCorrelationType() != null){
                this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,inheritedData.getCorrelationType());
                stepExplorerCorrelationTypeList.setBackground(INHERITED_FROM_PARENT);
                
            }else{
                this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,new Vector<String>());
                stepExplorerCorrelationTypeList.setBackground(NOT_DEFINED);
            }
        }
        //selection
        this.stepExplorerCorrelationSelectionBox.setBackground(DEFAULT);
        if(stepData.getCorrelationSelection() != null){
            this.stepExplorerCorrelationSelectionBox.setSelectedItem(stepData.getCorrelationSelection());
            this.stepExplorerCorrelationSelectionBox.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getCorrelationSelection() != null){
                this.stepExplorerCorrelationSelectionBox.setSelectedItem(inheritedData.getCorrelationSelection());
                stepExplorerCorrelationSelectionBox.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.stepExplorerCorrelationSelectionBox.setSelectedIndex(0);
                stepExplorerCorrelationSelectionBox.setBackground(NOT_DEFINED);
            }
        }
        //Baseline Selection
        this.BaselineSelectionPanel.setBackground(DEFAULT);
        this.baselineStation1Text.setText("");
        this.baselineStation2Text.setText("");
        this.fillBaselineTableFromVectors(new Vector<String>(),new Vector<String>());
        
        if(stepData.getStation1Selection() != null && stepData.getStation2Selection() != null){
            Vector<String> station1 = stepData.getStation1Selection();
            Vector<String> station2 = stepData.getStation2Selection();
            if(station1.size()>0 && station2.size()>0){
                this.baselineUseAllCheckbox.setSelected(false);
                this.fillBaselineTableFromVectors(station1,station2);
            }else{
                this.baselineUseAllCheckbox.setSelected(true);
            }
            this.BaselineSelectionPanel.setBackground(NOT_INHERITED_FROM_PARENT);
        }else{
            if(inheritedData.getStation1Selection() != null && inheritedData.getStation2Selection() != null){
                Vector<String> station1 = inheritedData.getStation1Selection();
                Vector<String> station2 = inheritedData.getStation2Selection();
                if(station1.size()>0 && station2.size()>0){
                    this.baselineUseAllCheckbox.setSelected(false);
                    this.fillBaselineTableFromVectors(station1,station2);
                }else{
                    this.baselineUseAllCheckbox.setSelected(true);
                }
                BaselineSelectionPanel.setBackground(INHERITED_FROM_PARENT);
            }else{
                baselineStationsTable.clearSelection();
                this.baselineUseAllCheckbox.setSelected(false);
                this.BaselineSelectionPanel.setBackground(NOT_DEFINED);
            }
        }
        //Operation type
        
        this.stepExplorerOperationComboBox.setBackground(DEFAULT);
        if(stepData.getOperationName() == null){
            if(inheritedData.getOperationName() != null){
                this.stepExplorerOperationComboBox.setSelectedItem(inheritedData.getOperationName());
                stepExplorerOperationComboBox.setBackground(INHERITED_FROM_PARENT);
                this.loadStepOperationsPanel(this.stepOperationPanels.get(inheritedData.getOperationName()),stepData,inheritedData);
            }else{
                this.stepExplorerOperationComboBox.setSelectedItem("NOT DEFINED");
                this.seOperationAttributesScrollPane.getViewport().removeAll();
                stepExplorerOperationComboBox.setBackground(NOT_DEFINED);
            }
        }else{
            this.stepExplorerOperationComboBox.setSelectedItem(stepData.getOperationName());
            this.stepExplorerOperationComboBox.setBackground(NOT_INHERITED_FROM_PARENT);
            this.loadStepOperationsPanel(this.stepOperationPanels.get(stepData.getOperationName()),stepData,inheritedData);
            
        }
    }
    
    /**
     * Helper method that saves the input fields in the panel to a BBSStep object.
     *
     * @parm aStep the BBSStep object to save all the fields in the panel to.
     */
    private void saveInput(BBSStep aStep) {
        BBSStepData aStepData = BBSStepDataManager.getInstance().getStepData(aStep.getName());
        BBSStepData inheritedData = BBSStepDataManager.getInstance().getInheritedStepData(aStep);
        
        //normal sources
        Vector<String> sources = createList(stepExplorerNSourcesList);
        if(this.useAllSourcesCheckbox.isSelected()){
            sources = new Vector<String>();
        }
        if(sources.equals(inheritedData.getSources())){
            aStepData.setSources(null);
        }else{
            if(this.useAllSourcesCheckbox.isSelected()){
                aStepData.setSources(new Vector<String>());
            }else{
                if(sources.size()>0){
                    aStepData.setSources(new Vector<String>());
                }else{
                    aStepData.setSources(null);
                }
                
            }
        }
        if(aStepData.getSources()!=null){
            if(!sources.equals(aStepData.getSources())){
                aStepData.setSources(sources);
            }
        }
        
        //instrument model        
        
        Vector<String> imodels = createList(stepExplorerInstrumentModelList);
        if(this.noInstrumentModelCheckbox.isSelected()){
            imodels = new Vector<String>();
        }
        if(imodels.equals(inheritedData.getInstrumentModel())){
            aStepData.setInstrumentModel(null);
        }else{
            if(this.noInstrumentModelCheckbox.isSelected()){
                aStepData.setInstrumentModel(new Vector<String>());
            }else{
                if(imodels.size()>0){
                    aStepData.setInstrumentModel(new Vector<String>());
                }else{
                    aStepData.setInstrumentModel(null);
                }
                
            }
        }
        if(aStepData.getInstrumentModel()!=null){
            if(!imodels.equals(aStepData.getInstrumentModel())){
                aStepData.setInstrumentModel(imodels);
            }
        }
        
/*        //instrument model        
        
        Vector<String> imodels = createVectorFromSelectionList(stepExplorerInstrumentModelList);
        if(this.noInstrumentModelCheckbox.isSelected()){
            imodels = new Vector<String>();
        }
        if(imodels.equals(inheritedData.getInstrumentModel())){
            aStepData.setInstrumentModel(null);
        }else{
            if(this.noInstrumentModelCheckbox.isSelected()){
                aStepData.setInstrumentModel(new Vector<String>());
            }else{
                if(imodels.size()>0){
                    aStepData.setInstrumentModel(new Vector<String>());
                }else{
                    aStepData.setInstrumentModel(null);
                }
                
            }
        }
        if(aStepData.getInstrumentModel()!=null){
            if(!imodels.equals(aStepData.getInstrumentModel())){
                aStepData.setInstrumentModel(imodels);
            }
        }        
*/       

//extra sources
//        Vector<String> esources = createList(stepExplorerESourcesList);
//        if(!this.useExtraSourcesCheckbox.isSelected()){
//            esources = new Vector<String>();
//        }
//        if(esources.equals(inheritedData.getExtraSources())){
//            aStepData.setExtraSources(null);
//        }else{
//            if(!this.useExtraSourcesCheckbox.isSelected()){
//                aStepData.setExtraSources(new Vector<String>());
//            }else{
//                if(esources.size()>0){
//                    aStepData.setExtraSources(new Vector<String>());
//                }else{
//                    aStepData.setExtraSources(null);
//                }
                
//            }
//        }
//        if(aStepData.getExtraSources()!=null){
//            if(!esources.equals(aStepData.getExtraSources())){
//                aStepData.setExtraSources(esources);
//            }
//        }
        //output data column
        String outputdata = stepExplorerOutputDataText.getText();
        if(this.writeOutputCheckbox.isSelected()){
            outputdata = "";
        }
        if(outputdata.equals(inheritedData.getOutputDataColumn())){
            aStepData.setOutputDataColumn(null);
        }else{
            if(this.writeOutputCheckbox.isSelected()){
                aStepData.setOutputDataColumn("");
            }else{
                if(!outputdata.equals("")){
                    aStepData.setOutputDataColumn("");
                }else{
                    aStepData.setOutputDataColumn(null);
                }
                
            }
        }
        if(aStepData.getOutputDataColumn()!=null){
            if(!outputdata.equals(aStepData.getOutputDataColumn())){
                aStepData.setOutputDataColumn(outputdata);
            }
        }
        

        
        //Integration
        //Time
        if(this.integrationTimeText.getText().equals("")){
            aStepData.setIntegrationTime(-1.0);
        }else{
            if(Double.parseDouble(integrationTimeText.getText()) == inheritedData.getIntegrationTime()){
                aStepData.setIntegrationTime(-1.0);
            }else{
                aStepData.setIntegrationTime(0.0);
            }
        }
        if(aStepData.getIntegrationTime()!=-1.0){
            if(Double.parseDouble(integrationTimeText.getText()) != aStepData.getIntegrationTime()){
                aStepData.setIntegrationTime(Double.parseDouble(integrationTimeText.getText()));
            }
        }
        //Frequency
        if(this.integrationFrequencyText.getText().equals("")){
            aStepData.setIntegrationFrequency(-1.0);
        }else{
            if(Double.parseDouble(integrationFrequencyText.getText()) == inheritedData.getIntegrationFrequency()){
                aStepData.setIntegrationFrequency(-1.0);
            }else{
                aStepData.setIntegrationFrequency(0.0);
            }
        }
        if(aStepData.getIntegrationFrequency()!=-1.0){
            if(Double.parseDouble(integrationFrequencyText.getText()) != aStepData.getIntegrationFrequency()){
                aStepData.setIntegrationFrequency(Double.parseDouble(integrationFrequencyText.getText()));
            }
        }
        
        //Correlation
        //Type
        if(this.createVectorFromSelectionList(this.stepExplorerCorrelationTypeList).size()==0){
            aStepData.setCorrelationType(null);
        }else{
            if(createVectorFromSelectionList(stepExplorerCorrelationTypeList).equals(inheritedData.getCorrelationType())){
                aStepData.setCorrelationType(null);
            }else{
                aStepData.setCorrelationType(new Vector<String>());
            }
        }
        if(aStepData.getCorrelationType()!=null){
            if(!createVectorFromSelectionList(stepExplorerCorrelationTypeList).equals(aStepData.getCorrelationType())){
                aStepData.setCorrelationType(createVectorFromSelectionList(stepExplorerCorrelationTypeList));
            }
        }
        //Selection
        String selectedCSelection = null;
        if(this.stepExplorerCorrelationSelectionBox.getSelectedItem() != null){
            selectedCSelection = this.stepExplorerCorrelationSelectionBox.getSelectedItem().toString();
        }
        if(selectedCSelection == null){
            aStepData.setCorrelationSelection(null);
        }else{
            if(selectedCSelection.equals("N/A")){
                aStepData.setCorrelationSelection(null);
            }else{
                if(selectedCSelection.equals(inheritedData.getCorrelationSelection())){
                    aStepData.setCorrelationSelection(null);
                }else{
                    aStepData.setCorrelationSelection("Generated by BBS GUI");
                }
            }
        }
        if(aStepData.getCorrelationSelection()!=null){
            if(selectedCSelection!= null && !selectedCSelection.equals(aStepData.getCorrelationSelection())){
                aStepData.setCorrelationSelection(selectedCSelection);
            }
        }
        
        //baseline selection
        Vector<Vector<String>> baselines = createVectorsFromBaselineTable();
        Vector<String> station1 = baselines.get(0);
        Vector<String> station2 = baselines.get(1);
        if(this.baselineUseAllCheckbox.isSelected()){
            station1 = new Vector<String>();
            station2 = new Vector<String>();
        }
        if(station1.equals(inheritedData.getStation1Selection()) && station2.equals(inheritedData.getStation2Selection())){
            aStepData.setStation1Selection(null);
        }else{
            if(this.baselineUseAllCheckbox.isSelected()){
                aStepData.setStation1Selection(new Vector<String>());
            }else{
                if(station1.size()>0 && station2.size()>0){
                    aStepData.setStation1Selection(new Vector<String>());
                }else{
                    aStepData.setStation1Selection(null);
                }
                
            }
            
        }
        
        if(station1.equals(inheritedData.getStation1Selection()) && station2.equals(inheritedData.getStation2Selection())){
            aStepData.setStation2Selection(null);
        }else{
            if(this.baselineUseAllCheckbox.isSelected()){
                aStepData.setStation2Selection(new Vector<String>());
            }else{
                if(station1.size()>0 && station2.size()>0){
                    aStepData.setStation2Selection(new Vector<String>());
                }else{
                    aStepData.setStation2Selection(null);
                }
                
            }
        }
        
        if(aStepData.getStation1Selection()!=null){
            if(!station1.equals(aStepData.getStation1Selection())){
                aStepData.setStation1Selection(station1);
            }
        }
        if(aStepData.getStation2Selection()!=null){
            if(!station2.equals(aStepData.getStation2Selection())){
                aStepData.setStation2Selection(station2);
            }
        }
        
        //Operation
        
        //name
        String selectedOSelection = null;
        if(this.stepExplorerOperationComboBox.getSelectedItem() != null){
            selectedOSelection = this.stepExplorerOperationComboBox.getSelectedItem().toString();
        }
        if(selectedOSelection == null){
            aStepData.setOperationName(null);
            
        }else{
            if(selectedOSelection.equals("NOT DEFINED")){
                aStepData.setOperationName(null);
            }else{
                if(selectedOSelection.equals(inheritedData.getOperationName())){
                    aStepData.setOperationName(null);
                }else{
                    aStepData.setOperationName("Generated by BBS GUI");
                }
            }
        }
        if(aStepData.getOperationName()!=null){
            if(selectedOSelection!= null && !selectedOSelection.equals(aStepData.getOperationName())){
                aStepData.setOperationName(selectedOSelection);
            }
        }
        //fetch variables from operation attributes panel
        if(!selectedOSelection.equals("NOT DEFINED") && currentStepOperationsPanel != null){
            if(this.currentStepOperationsPanel.getBBSStepOperationAttributes()!=null){
                
                HashMap<String,String> valuesFromForm = currentStepOperationsPanel.getBBSStepOperationAttributes();
                
                HashMap<String,String> oldValuesFromStep = aStepData.getOperationAttributes();
                
                if(oldValuesFromStep == null) oldValuesFromStep = new HashMap<String,String>();
                
                for(String aKey : valuesFromForm.keySet()){
                    if(oldValuesFromStep.containsKey(aKey)){
                        if(valuesFromForm.get(aKey) == null){
                            oldValuesFromStep.put(aKey,null);
                        }else{
                            if(valuesFromForm.get(aKey).equals(inheritedData.getOperationAttribute(aKey))){
                                oldValuesFromStep.put(aKey,null);
                            }else{
                                oldValuesFromStep.put(aKey,"Generated by BBS GUI");
                            }
                        }
                        if(oldValuesFromStep.get(aKey) != null){
                            if(!valuesFromForm.get(aKey).equals(oldValuesFromStep.get(aKey))){
                                oldValuesFromStep.put(aKey,valuesFromForm.get(aKey));
                                aStepData.setOperationName(selectedOSelection);
                            }
                        }
                    }else{
                        String newValue = valuesFromForm.get(aKey);
                        String inheritedValue = inheritedData.getOperationAttribute(aKey);
                        if(newValue!= null && newValue.equals(inheritedValue)){
                            oldValuesFromStep.put(aKey,null);
                        }else{
                            oldValuesFromStep.put(aKey,valuesFromForm.get(aKey));
                            aStepData.setOperationName(selectedOSelection);
                        }
                    }
                }
                aStepData.setOperationAttributes(oldValuesFromStep);
            }
        }else if(currentStepOperationsPanel == null){
            aStepData.setOperationAttributes(null);
        }
    }
    
    /**
     * Helper method that builds a String Vector representation of the contents of a JList.
     *
     * @parm aListComponent the JList component to build a String Vector representation for.
     * @return String Vector representation of the contents of the given JList.
     */
    private Vector<String> createList(JList aListComponent) {
        Vector<String> aList = new Vector<String>();
        if (aListComponent.getModel().getSize() > 0) {
            for (int i=0; i < aListComponent.getModel().getSize();i++) {
                aList.add(aListComponent.getModel().getElementAt(i).toString());
            }
        }
        return aList;
    }
    
    /**
     * Helper method that fills a JList with a String Vector representation of a JList.
     *
     * @parm aListComponent the JList to fill
     * @parm theList the String Vector to fill the JList with.
     */
    private void fillList(JList aListComponent,Vector<String> theList) {
        DefaultListModel itsModel = new DefaultListModel();
        aListComponent.setModel(itsModel);
        for(String anItem : theList){
            itsModel.addElement(anItem);
        }
        aListComponent.setModel(itsModel);
    }
    
    /**
     * Helper method that builds a String Vector representation of the <i>selected</i> contents of a JList.
     *
     * @parm aListComponent the JList component to build a String Vector representation for.
     * @return String Vector representation of the <i>selected</i> contents of the given JList.
     */
    private Vector<String> createVectorFromSelectionList(JList aListComponent) {
        Vector<String> aList= new Vector<String>();
        int[] selectedIndices = aListComponent.getSelectedIndices();
        if (selectedIndices.length > 0) {
            for (int i=0; i < selectedIndices.length;i++) {
                aList.add(aListComponent.getModel().getElementAt(selectedIndices[i]).toString());
            }
        }
        return aList;
    }
    
    /**
     * Helper method that selects items in a JList with a String Vector representation of the selected items of a JList.
     *
     * @parm aListComponent the JList to select items in.
     * @parm theList the String Vector to select items in the JList with.
     */
    private void fillSelectionListFromVector(JList aListComponent,Vector<String> theList) {
        int[] toBeSelectedIndices = new int[theList.size()];
        int aValueIndex = 0;
        if(theList.size()>0){
            for(String aValue : theList){
                for(int in = 0; in < aListComponent.getModel().getSize();in++){
                    String aCorrType = (String)aListComponent.getModel().getElementAt(in);
                    if(aValue.equals(aCorrType)){
                        toBeSelectedIndices[aValueIndex] = in;
                    }
                }
                aValueIndex++;
            }
            aListComponent.setSelectedIndices(toBeSelectedIndices);
            
        }else{
            aListComponent.clearSelection();
        }
        
    }
    
    /**
     * Helper method that fills the Baseline table with the Station1 and Station2 Vectors
     *
     * @parm station1 the Station1 part of the Baseline pair of Vectors.
     * @parm station2 the Station2 part of the Baseline pair of Vectors.
     */
    private void fillBaselineTableFromVectors(Vector<String> station1,Vector<String> station2) {
        baselineStationsTable.setModel(new javax.swing.table.DefaultTableModel(
                new Object [][] {
            
        },
                new String [] {
            "Station 1", "Station 2"
        }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            
            @Override
            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }
        });
        DefaultTableModel bsltm = (DefaultTableModel)baselineStationsTable.getModel();
        
        if(station1.size() == station2.size()){
            for(int i = 0; i<station1.size();i++){
                Vector<String> newRow = new Vector<String>();
                newRow.add(station1.get(i));
                newRow.add(station2.get(i));
                bsltm.addRow(newRow);
            }
        }
    }
    
    /**
     * Helper method that creates the Station1 and Station2 vectors out of the items in the Baseline table
     *
     * @return Vector containing 2 String Vectors : Station1 (index 0) and Station2 (index 1).
     */
    private Vector<Vector<String>> createVectorsFromBaselineTable(){
        Vector<Vector<String>> returnVector = new Vector<Vector<String>>();
        Vector<String> station1Vector = new Vector<String>();
        Vector<String> station2Vector = new Vector<String>();
        DefaultTableModel bsltm = (DefaultTableModel)baselineStationsTable.getModel();
        if(bsltm.getRowCount()>0){
            for(int i = 0; i<bsltm.getRowCount();i++){
                String station1 = ((Vector)bsltm.getDataVector().elementAt(i)).elementAt(0).toString();
                String station2 = ((Vector)bsltm.getDataVector().elementAt(i)).elementAt(1).toString();
                station1Vector.add(station1);
                station2Vector.add(station2);
            }
        }
        
        returnVector.add(station1Vector);
        returnVector.add(station2Vector);
        
        return returnVector;
    }
    /**
     * Helper method that checks if the Baseline input text fields are filled 
     * correctly before entering a new row in the Baseline table
     */
    private void checkBaselineInput(){
        String typedText=baselineStation1Text.getText();
        String typedText2=baselineStation2Text.getText();
        if(!typedText.equals("") && !typedText2.equals("")){
            this.addBaseLineButton.setEnabled(true);
        }else{
            this.addBaseLineButton.setEnabled(false);
        }
    }
    /**
     * Helper method that loads a Step Operation panel if the BBSStep displayed
     * in this panel contains a valid Operation that needs extra attributes to be entered.
     *
     * @parm name the full String representation of a Step Operation panel to load 
     * (eg. nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations.***) 
     * @parm data the BBSStep to display the Operation Attributes of in the Step Operation panel when modifying a step.
     * @parm inheritedData the BBSStep to display the Operation Attributes of in the Step Operation panel when adding a new step.
     */
    private void loadStepOperationsPanel(String name, BBSStepData data, BBSStepData inheritedData){
        JPanel newPanel = null;
        if(name!=null){
            try {
                newPanel = (JPanel) Class.forName(name).newInstance();
            } catch (ClassNotFoundException ex) {
                logger.debug("Error during getPanel: "+ ex);
                return;
            } catch (InstantiationException ex) {
                logger.debug("Error during getPanel: "+ ex);
                return;
            } catch (IllegalAccessException ex) {
                logger.debug("Error during getPanel: "+ ex);
                return;
            }
            
            if (newPanel!=null) {
                currentStepOperationsPanel = (IBBSStepOperationPanel)newPanel;
                currentStepOperationsPanel.setBBSStepContent(data,inheritedData);
                this.seOperationAttributesScrollPane.getViewport().removeAll();
                this.seOperationAttributesScrollPane.setViewportView(newPanel);
            }
        }else{
            this.seOperationAttributesScrollPane.getViewport().removeAll();
            this.seOperationAttributesScrollPane.revalidate();
            this.seOperationAttributesScrollPane.repaint();
            currentStepOperationsPanel = null;
        }
        
    }
    
    /**
     * Helper method that checks the input of all non-String attributes of a BBS Step, like double and integer attributes.
     *
     * It changes the background color of an erroneous input field to Color.RED to instruct the user to correct it.
     *
     * @return <i>true</i> - if all input is OK, <i>false</i> - if some input is not OK. 
     */
    private boolean validateInput(){
        boolean operationOK = true;
        //perform input validation on the double values in the form
        String integrationTime = this.integrationTimeText.getText();
        String integrationFrequency = this.integrationFrequencyText.getText();
        
        if(!integrationTime.equals("")){
            try {
                Double itime = Double.parseDouble(integrationTime);
                integrationTimeText.setBackground(Color.WHITE);
                integrationTimeText.setToolTipText("Time in seconds (s)");
            } catch (NumberFormatException ex) {
                integrationTimeText.setBackground(Color.RED);
                operationOK=false;
                integrationTimeText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(!integrationFrequency.equals("")){
            try {
                Double itime = Double.parseDouble(integrationFrequency);
                integrationFrequencyText.setBackground(Color.WHITE);
                integrationFrequencyText.setToolTipText("Frequency in Hertz (Hz)");
                
            } catch (NumberFormatException ex) {
                operationOK=false;
                integrationFrequencyText.setBackground(Color.RED);
                integrationFrequencyText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(currentStepOperationsPanel!=null){
            operationOK = this.currentStepOperationsPanel.validateInput();
            
        }
        return operationOK;
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        BBSStepExplorerPanel = new javax.swing.JPanel();
        stepExplorerScrollPanel = new javax.swing.JScrollPane();
        stepExplorerPanel = new javax.swing.JPanel();
        stepExplorerStepNameLabel = new javax.swing.JLabel();
        stepExplorerStepNameText = new javax.swing.JTextField();
        stepExplorerRevertButton = new javax.swing.JButton();
        stepExplorerOperationPanel = new javax.swing.JPanel();
        stepExplorerOperationTypeHeaderPanel = new javax.swing.JPanel();
        stepExplorerOperationTypeLabel = new javax.swing.JLabel();
        stepExplorerOperationComboBox = new javax.swing.JComboBox();
        stepExplorerOperationAttributesPanel = new javax.swing.JPanel();
        seOperationAttributesScrollPane = new javax.swing.JScrollPane();
        stepExplorerOutputDataPanel = new javax.swing.JPanel();
        stepExplorerOutputDataText = new javax.swing.JTextField();
        writeOutputCheckbox = new javax.swing.JCheckBox();
        stepExplorerNSources = new javax.swing.JPanel();
        stepExplorerNSourcesPanel = new javax.swing.JPanel();
        useAllSourcesCheckbox = new javax.swing.JCheckBox();
        stepExplorerNSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerNSourcesList = new javax.swing.JList();
        stepExplorerNSourcesEditPanel = new javax.swing.JPanel();
        stepExplorerModifyNSourceCombobox = new javax.swing.JComboBox();
        stepExplorerNSourcesButtonPanel = new javax.swing.JPanel();
        addNSourceButton = new javax.swing.JButton();
        deleteNSourceButton = new javax.swing.JButton();
        stepExplorerInstrumentModel = new javax.swing.JPanel();
        stepExplorerInstrumentModelPanel = new javax.swing.JPanel();
        noInstrumentModelCheckbox = new javax.swing.JCheckBox();
        stepExplorerInstrumentModelScrollPane = new javax.swing.JScrollPane();
        stepExplorerInstrumentModelList = new javax.swing.JList();
        stepExplorerInstrumentModelEditPanel = new javax.swing.JPanel();
        stepExplorerModifyInstrumentModelCombobox = new javax.swing.JComboBox();
        StepExplorerInstrumentModelButtonPanel = new javax.swing.JPanel();
        addInstrumentButton = new javax.swing.JButton();
        deleteInstrumentButton = new javax.swing.JButton();
        stepExplorerCorrelationPanel = new javax.swing.JPanel();
        stepExplorerCorrelationSelectionLabel = new javax.swing.JLabel();
        stepExplorerCorrelationSelectionBox = new javax.swing.JComboBox();
        stepExplorerCorrelationTypeLabel = new javax.swing.JLabel();
        stepExplorerCorrelationTypeScrollPane = new javax.swing.JScrollPane();
        stepExplorerCorrelationTypeList = new javax.swing.JList();
        integrationIntervalPanel = new javax.swing.JPanel();
        integrationFrequencyLabel = new javax.swing.JLabel();
        integrationFrequencyText = new javax.swing.JTextField();
        integrationFrequencyUnitLabel = new javax.swing.JLabel();
        integrationTimeLabel = new javax.swing.JLabel();
        integrationTimeText = new javax.swing.JTextField();
        integrationTimeUnitLabel = new javax.swing.JLabel();
        BaselineSelectionPanel = new javax.swing.JPanel();
        baselinePanel = new javax.swing.JPanel();
        baselineStationsScrollPane = new javax.swing.JScrollPane();
        baselineStationsTable = new javax.swing.JTable();
        baselineInputPanel = new javax.swing.JPanel();
        baselineStation1Text = new javax.swing.JTextField();
        baselineStation2Text = new javax.swing.JTextField();
        baselineUseAllCheckbox = new javax.swing.JCheckBox();
        baselineModsPanel = new javax.swing.JPanel();
        addBaseLineButton = new javax.swing.JButton();
        deleteBaseLineButton = new javax.swing.JButton();
        helpButton = new javax.swing.JButton();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        BBSStepExplorerPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerStepNameLabel.setFont(new java.awt.Font("Dialog", 1, 18));
        stepExplorerStepNameLabel.setText("Step");
        stepExplorerPanel.add(stepExplorerStepNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 10, -1, 30));

        stepExplorerStepNameText.setEditable(false);
        stepExplorerStepNameText.setFont(new java.awt.Font("Dialog", 1, 18));
        stepExplorerStepNameText.setToolTipText("This is the name of the displayed step");
        stepExplorerStepNameText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerStepNameTextKeyReleased(evt);
            }
        });
        stepExplorerPanel.add(stepExplorerStepNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(80, 10, 260, 30));

        stepExplorerRevertButton.setText("Revert");
        stepExplorerRevertButton.setToolTipText("Revert the step variables to the values present when this dialog was opened.");
        stepExplorerRevertButton.setEnabled(false);
        stepExplorerRevertButton.setMaximumSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.setMinimumSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.setPreferredSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerRevertButtonActionPerformed(evt);
            }
        });
        stepExplorerPanel.add(stepExplorerRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 650, 100, -1));

        stepExplorerOperationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Operation"));
        stepExplorerOperationPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationTypeHeaderPanel.setBackground(new java.awt.Color(204, 204, 204));
        stepExplorerOperationTypeHeaderPanel.setLayout(new java.awt.GridBagLayout());

        stepExplorerOperationTypeLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        stepExplorerOperationTypeLabel.setText("Type :");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationTypeLabel, gridBagConstraints);

        stepExplorerOperationComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "NOT DEFINED", "Predict", "Solve", "Subtract", "Correct" }));
        stepExplorerOperationComboBox.setToolTipText("The type of operation to be performed in this step. Use NOT DEFINED when this step is/will be a multistep, or when this step should not perform an operation.");
        stepExplorerOperationComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                stepExplorerOperationComboBoxItemStateChanged(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationComboBox, gridBagConstraints);

        stepExplorerOperationPanel.add(stepExplorerOperationTypeHeaderPanel, java.awt.BorderLayout.NORTH);

        stepExplorerOperationAttributesPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(204, 204, 204), null));
        stepExplorerOperationAttributesPanel.setLayout(new java.awt.BorderLayout());
        stepExplorerOperationAttributesPanel.add(seOperationAttributesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerOperationPanel.add(stepExplorerOperationAttributesPanel, java.awt.BorderLayout.CENTER);

        stepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 400, 700, 240));

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data Column"));
        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataText.setText("CORRECTED_DATA");
        stepExplorerOutputDataText.setToolTipText("Column of the measurement set wherein the output values of this step should be written");
        stepExplorerOutputDataText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerOutputDataTextKeyReleased(evt);
            }
        });
        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 150, 20));

        writeOutputCheckbox.setText("No output");
        writeOutputCheckbox.setToolTipText("Check this box if no output data should be written in this step");
        writeOutputCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        writeOutputCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        writeOutputCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                writeOutputCheckboxItemStateChanged(evt);
            }
        });
        stepExplorerOutputDataPanel.add(writeOutputCheckbox, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        stepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 280, 170, 110));

        stepExplorerNSources.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
        stepExplorerNSources.setMinimumSize(new java.awt.Dimension(150, 150));
        stepExplorerNSources.setPreferredSize(new java.awt.Dimension(150, 150));
        stepExplorerNSources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerNSourcesPanel.setLayout(new java.awt.BorderLayout());

        useAllSourcesCheckbox.setSelected(true);
        useAllSourcesCheckbox.setText("Use all sources");
        useAllSourcesCheckbox.setToolTipText("Check this box to use all the sources");
        useAllSourcesCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        useAllSourcesCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        useAllSourcesCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                useAllSourcesCheckboxItemStateChanged(evt);
            }
        });
        stepExplorerNSourcesPanel.add(useAllSourcesCheckbox, java.awt.BorderLayout.NORTH);

        stepExplorerNSourcesList.setBackground(java.awt.Color.lightGray);
        stepExplorerNSourcesList.setToolTipText("The specified sources for this step");
        stepExplorerNSourcesList.setEnabled(false);
        stepExplorerNSourcesList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerNSourcesListValueChanged(evt);
            }
        });
        stepExplorerNSourcesScrollPane.setViewportView(stepExplorerNSourcesList);

        stepExplorerNSourcesPanel.add(stepExplorerNSourcesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerNSourcesEditPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerModifyNSourceCombobox.setToolTipText("Add sources");
        stepExplorerModifyNSourceCombobox.setEnabled(false);
        stepExplorerNSourcesEditPanel.add(stepExplorerModifyNSourceCombobox, java.awt.BorderLayout.CENTER);

        stepExplorerNSourcesButtonPanel.setLayout(new java.awt.GridBagLayout());

        addNSourceButton.setText("Add");
        addNSourceButton.setToolTipText("Add the source entered above to the list of sources");
        addNSourceButton.setEnabled(false);
        addNSourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addNSourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addNSourceButton.setPreferredSize(new java.awt.Dimension(53, 23));
        addNSourceButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addNSourceButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerNSourcesButtonPanel.add(addNSourceButton, gridBagConstraints);

        deleteNSourceButton.setText("Delete");
        deleteNSourceButton.setToolTipText("Remove the selected source from the list");
        deleteNSourceButton.setEnabled(false);
        deleteNSourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteNSourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteNSourceButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteNSourceButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteNSourceButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerNSourcesButtonPanel.add(deleteNSourceButton, gridBagConstraints);

        stepExplorerNSourcesEditPanel.add(stepExplorerNSourcesButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerNSourcesPanel.add(stepExplorerNSourcesEditPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerNSources.add(stepExplorerNSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 150, 240));

        stepExplorerPanel.add(stepExplorerNSources, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 0, 170, 270));

        stepExplorerInstrumentModel.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Model"));
        stepExplorerInstrumentModel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerInstrumentModelPanel.setLayout(new java.awt.BorderLayout());

        noInstrumentModelCheckbox.setSelected(true);
        noInstrumentModelCheckbox.setText("No model");
        noInstrumentModelCheckbox.setToolTipText("Check this box when no instrument model should be used");
        noInstrumentModelCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        noInstrumentModelCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        noInstrumentModelCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                noInstrumentModelCheckboxItemStateChanged(evt);
            }
        });
        stepExplorerInstrumentModelPanel.add(noInstrumentModelCheckbox, java.awt.BorderLayout.NORTH);

        stepExplorerInstrumentModelScrollPane.setPreferredSize(new java.awt.Dimension(260, 132));

        stepExplorerInstrumentModelList.setBackground(java.awt.Color.lightGray);
        stepExplorerInstrumentModelList.setToolTipText("the specified instrument models for this step");
        stepExplorerInstrumentModelList.setEnabled(false);
        stepExplorerInstrumentModelList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerInstrumentModelListValueChanged(evt);
            }
        });
        stepExplorerInstrumentModelScrollPane.setViewportView(stepExplorerInstrumentModelList);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModelEditPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerModifyInstrumentModelCombobox.setToolTipText("Input Instrument Models");
        stepExplorerModifyInstrumentModelCombobox.setEnabled(false);
        stepExplorerInstrumentModelEditPanel.add(stepExplorerModifyInstrumentModelCombobox, java.awt.BorderLayout.CENTER);

        StepExplorerInstrumentModelButtonPanel.setLayout(new java.awt.GridBagLayout());

        addInstrumentButton.setText("Add");
        addInstrumentButton.setToolTipText("Add the source entered above to the list of sources");
        addInstrumentButton.setEnabled(false);
        addInstrumentButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addInstrumentButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addInstrumentButton.setPreferredSize(new java.awt.Dimension(53, 23));
        addInstrumentButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addInstrumentButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        StepExplorerInstrumentModelButtonPanel.add(addInstrumentButton, gridBagConstraints);

        deleteInstrumentButton.setText("Delete");
        deleteInstrumentButton.setToolTipText("Remove the selected source from the list");
        deleteInstrumentButton.setEnabled(false);
        deleteInstrumentButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteInstrumentButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteInstrumentButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        StepExplorerInstrumentModelButtonPanel.add(deleteInstrumentButton, gridBagConstraints);

        stepExplorerInstrumentModelEditPanel.add(StepExplorerInstrumentModelButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelEditPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerInstrumentModel.add(stepExplorerInstrumentModelPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(6, 14, 158, 250));

        stepExplorerPanel.add(stepExplorerInstrumentModel, new org.netbeans.lib.awtextra.AbsoluteConstraints(550, 0, 170, 270));

        stepExplorerCorrelationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        stepExplorerCorrelationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerCorrelationSelectionLabel.setText("Selection :");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        stepExplorerCorrelationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "N/A", "AUTO", "CROSS", "ALL" }));
        stepExplorerCorrelationSelectionBox.setSelectedIndex(3);
        stepExplorerCorrelationSelectionBox.setToolTipText("Specifies which station correlations to use.");
        stepExplorerCorrelationSelectionBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                stepExplorerCorrelationSelectionBoxItemStateChanged(evt);
            }
        });
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 80, -1));

        stepExplorerCorrelationTypeLabel.setText("Type :");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 10, -1, -1));

        stepExplorerCorrelationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerCorrelationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY.");
        stepExplorerCorrelationTypeList.setSelectedIndices(new int[]{0,3});
        stepExplorerCorrelationTypeList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerCorrelationTypeListValueChanged(evt);
            }
        });
        stepExplorerCorrelationTypeScrollPane.setViewportView(stepExplorerCorrelationTypeList);

        stepExplorerCorrelationPanel.add(stepExplorerCorrelationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        stepExplorerPanel.add(stepExplorerCorrelationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(550, 270, 170, 120));

        integrationIntervalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Integration", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), java.awt.Color.lightGray));
        integrationIntervalPanel.setToolTipText("Cell size for integration. Not yet implemented.");
        integrationIntervalPanel.setEnabled(false);
        integrationIntervalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        integrationFrequencyLabel.setText("Freq. Interval :");
        integrationFrequencyLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 20, -1, -1));

        integrationFrequencyText.setEditable(false);
        integrationFrequencyText.setToolTipText("Frequency interval in Hertz (Hz)");
        integrationFrequencyText.setEnabled(false);
        integrationFrequencyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                integrationFrequencyTextKeyReleased(evt);
            }
        });
        integrationIntervalPanel.add(integrationFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, 70, -1));

        integrationFrequencyUnitLabel.setText("Hz");
        integrationFrequencyUnitLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 20, -1, -1));

        integrationTimeLabel.setText("Time Interval :");
        integrationTimeLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 50, -1, -1));

        integrationTimeText.setEditable(false);
        integrationTimeText.setEnabled(false);
        integrationTimeText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                integrationTimeTextKeyReleased(evt);
            }
        });
        integrationIntervalPanel.add(integrationTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 50, 70, -1));

        integrationTimeUnitLabel.setText("s");
        integrationTimeUnitLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 50, 10, -1));

        stepExplorerPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 310, 320, 80));

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline Selection"));
        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        baselinePanel.setLayout(new java.awt.BorderLayout());

        baselineStationsScrollPane.setToolTipText("The baseline pairs of stations");
        baselineStationsScrollPane.setEnabled(false);
        baselineStationsScrollPane.setPreferredSize(new java.awt.Dimension(453, 250));

        baselineStationsTable.setBackground(java.awt.Color.lightGray);
        baselineStationsTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {
                "Station 1", "Station 2"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }
        });
        baselineStationsTable.setToolTipText("The baselines used");
        baselineStationsTable.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseReleased(java.awt.event.MouseEvent evt) {
                baselineStationsTableMouseReleased(evt);
            }
        });
        baselineStationsScrollPane.setViewportView(baselineStationsTable);

        baselinePanel.add(baselineStationsScrollPane, java.awt.BorderLayout.CENTER);

        baselineInputPanel.setLayout(new java.awt.GridBagLayout());

        baselineStation1Text.setToolTipText("Input field for the name of the first station part that forms the baseline");
        baselineStation1Text.setEnabled(false);
        baselineStation1Text.setMinimumSize(new java.awt.Dimension(120, 19));
        baselineStation1Text.setPreferredSize(new java.awt.Dimension(200, 19));
        baselineStation1Text.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                baselineStation1TextKeyReleased(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineInputPanel.add(baselineStation1Text, gridBagConstraints);

        baselineStation2Text.setToolTipText("Input field for the name of the second station part that forms the baseline");
        baselineStation2Text.setEnabled(false);
        baselineStation2Text.setMinimumSize(new java.awt.Dimension(120, 19));
        baselineStation2Text.setPreferredSize(new java.awt.Dimension(200, 19));
        baselineStation2Text.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                baselineStation2TextKeyReleased(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineInputPanel.add(baselineStation2Text, gridBagConstraints);

        baselinePanel.add(baselineInputPanel, java.awt.BorderLayout.SOUTH);

        baselineUseAllCheckbox.setSelected(true);
        baselineUseAllCheckbox.setText("Use all baselines");
        baselineUseAllCheckbox.setToolTipText("Check this box to use all baselines available");
        baselineUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        baselineUseAllCheckbox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        baselineUseAllCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                baselineUseAllCheckboxItemStateChanged(evt);
            }
        });
        baselinePanel.add(baselineUseAllCheckbox, java.awt.BorderLayout.NORTH);

        BaselineSelectionPanel.add(baselinePanel, java.awt.BorderLayout.CENTER);

        baselineModsPanel.setLayout(new java.awt.GridBagLayout());

        addBaseLineButton.setText("Add");
        addBaseLineButton.setToolTipText("Adds a baseline using the Station1 and Station2 values in the input boxes above");
        addBaseLineButton.setEnabled(false);
        addBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setPreferredSize(new java.awt.Dimension(50, 23));
        addBaseLineButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addBaseLineButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 7;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(addBaseLineButton, gridBagConstraints);

        deleteBaseLineButton.setText("Delete");
        deleteBaseLineButton.setToolTipText("Deletes the selected baseline (the selected Station 1 and Station 2 pair)");
        deleteBaseLineButton.setEnabled(false);
        deleteBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteBaseLineButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteBaseLineButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 6;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(deleteBaseLineButton, gridBagConstraints);

        BaselineSelectionPanel.add(baselineModsPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerPanel.add(BaselineSelectionPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 320, 250));

        helpButton.setText("Help");
        helpButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                helpButtonActionPerformed(evt);
            }
        });
        stepExplorerPanel.add(helpButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 650, -1, -1));

        stepExplorerScrollPanel.setViewportView(stepExplorerPanel);

        BBSStepExplorerPanel.add(stepExplorerScrollPanel, java.awt.BorderLayout.CENTER);

        add(BBSStepExplorerPanel, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents

    private void deleteInstrumentButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteInstrumentButtonActionPerformed
        DefaultListModel theInstrumentModel = (DefaultListModel)stepExplorerInstrumentModelList.getModel();
        int[] selectedIndices = stepExplorerInstrumentModelList.getSelectedIndices();
        while(selectedIndices.length>0){
            theInstrumentModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerInstrumentModelList.getSelectedIndices();
            stepExplorerInstrumentModel.setBackground(NOT_INHERITED_FROM_PARENT);
        }
        if(theInstrumentModel.size()==0){
            this.deleteInstrumentButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteInstrumentButtonActionPerformed

    private void addInstrumentButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addInstrumentButtonActionPerformed
        String toBeAddedInstrumentModel = (String)this.stepExplorerModifyInstrumentModelCombobox.getSelectedItem();
        DefaultListModel theInstrumentModel = (DefaultListModel)stepExplorerInstrumentModelList.getModel();
        if(!theInstrumentModel.contains(toBeAddedInstrumentModel)){
            theInstrumentModel.addElement(toBeAddedInstrumentModel);
            stepExplorerInstrumentModel.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addInstrumentButtonActionPerformed
    
    private void noInstrumentModelCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_noInstrumentModelCheckboxItemStateChanged
     if(this.noInstrumentModelCheckbox.isSelected()){
            this.stepExplorerInstrumentModelList.setEnabled(false);
            this.stepExplorerInstrumentModelList.setBackground(Color.LIGHT_GRAY);
            this.stepExplorerInstrumentModel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerInstrumentModelList.clearSelection();
            this.deleteInstrumentButton.setEnabled(false);
            this.addInstrumentButton.setEnabled(false);
            this.stepExplorerModifyInstrumentModelCombobox.setEnabled(false);
        }else{
            this.stepExplorerInstrumentModelList.setEnabled(true);
            this.stepExplorerInstrumentModelList.setBackground(Color.WHITE);
            this.stepExplorerInstrumentModel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerInstrumentModelList.clearSelection();
            this.addInstrumentButton.setEnabled(true);
            this.stepExplorerModifyInstrumentModelCombobox.setEnabled(true);
        }
    }//GEN-LAST:event_noInstrumentModelCheckboxItemStateChanged
    
    private void writeOutputCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_writeOutputCheckboxItemStateChanged
        if(this.writeOutputCheckbox.isSelected()){
            this.stepExplorerOutputDataText.setEnabled(false);
            this.stepExplorerOutputDataPanel.setBackground(this.NOT_INHERITED_FROM_PARENT);
        }else{
            this.stepExplorerOutputDataText.setEnabled(true);
            this.stepExplorerOutputDataPanel.setBackground(this.NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_writeOutputCheckboxItemStateChanged
        
    private void useAllSourcesCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_useAllSourcesCheckboxItemStateChanged
        if(this.useAllSourcesCheckbox.isSelected()){
            this.stepExplorerNSourcesList.setEnabled(false);
            this.stepExplorerNSourcesList.setBackground(Color.LIGHT_GRAY);
            this.stepExplorerNSources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerNSourcesList.clearSelection();
            this.deleteNSourceButton.setEnabled(false);
            this.addNSourceButton.setEnabled(false);
            this.stepExplorerModifyNSourceCombobox.setEnabled(false);
        }else{
            this.stepExplorerNSourcesList.setEnabled(true);
            this.stepExplorerNSourcesList.setBackground(Color.WHITE);
            this.stepExplorerNSources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerNSourcesList.clearSelection();
            this.stepExplorerModifyNSourceCombobox.setEnabled(true);
            this.addNSourceButton.setEnabled(true);
        }
    }//GEN-LAST:event_useAllSourcesCheckboxItemStateChanged
        
    private void stepExplorerOperationComboBoxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_stepExplorerOperationComboBoxItemStateChanged
        stepExplorerOperationComboBox.setBackground(this.NOT_INHERITED_FROM_PARENT);
        String item = stepExplorerOperationComboBox.getSelectedItem().toString();
        if(item.equals("NOT DEFINED")){
            this.seOperationAttributesScrollPane.getViewport().removeAll();
            this.seOperationAttributesScrollPane.revalidate();
            this.seOperationAttributesScrollPane.repaint();
            currentStepOperationsPanel = null;
        }else{
            BBSStepData stepData = null;
            BBSStepData inheritedData = null;
            
            if(itsBBSStep!=null){
                stepData=BBSStepDataManager.getInstance().getStepData(itsBBSStep.getName());
                inheritedData=BBSStepDataManager.getInstance().getInheritedStepData(itsBBSStep);
            }
            this.loadStepOperationsPanel(this.stepOperationPanels.get(item),stepData,inheritedData);
            
        }
    }//GEN-LAST:event_stepExplorerOperationComboBoxItemStateChanged
    
    private void baselineUseAllCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_baselineUseAllCheckboxItemStateChanged
        if(baselineUseAllCheckbox.isSelected()){
            this.baselineStationsTable.setBackground(Color.LIGHT_GRAY);
            this.BaselineSelectionPanel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.baselineStationsTable.setEnabled(false);
            this.baselineStationsTable.clearSelection();
            this.deleteBaseLineButton.setEnabled(false);
            this.addBaseLineButton.setEnabled(false);
            this.baselineStation1Text.setEnabled(false);
            this.baselineStation2Text.setEnabled(false);
        }else{
            this.baselineStationsTable.setBackground(Color.WHITE);
            this.BaselineSelectionPanel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.baselineStationsTable.setEnabled(true);
            this.baselineStation1Text.setEnabled(true);
            this.baselineStation2Text.setEnabled(true);
            checkBaselineInput();
        }
    }//GEN-LAST:event_baselineUseAllCheckboxItemStateChanged
    
    private void deleteBaseLineButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteBaseLineButtonActionPerformed
        DefaultTableModel bsltm = (DefaultTableModel)this.baselineStationsTable.getModel();
        int[] selectedRows = this.baselineStationsTable.getSelectedRows();
        while(selectedRows.length>0){
            bsltm.removeRow(selectedRows[0]);
            selectedRows = this.baselineStationsTable.getSelectedRows();
            this.BaselineSelectionPanel.setBackground(NOT_INHERITED_FROM_PARENT);
        }
        this.deleteBaseLineButton.setEnabled(false);
        
    }//GEN-LAST:event_deleteBaseLineButtonActionPerformed
    
    private void baselineStation2TextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_baselineStation2TextKeyReleased
        checkBaselineInput();
    }//GEN-LAST:event_baselineStation2TextKeyReleased
    
    private void baselineStation1TextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_baselineStation1TextKeyReleased
        checkBaselineInput();
    }//GEN-LAST:event_baselineStation1TextKeyReleased
    
    private void baselineStationsTableMouseReleased(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_baselineStationsTableMouseReleased
        int[] selectedRows = this.baselineStationsTable.getSelectedRows();
        if(selectedRows.length>0){
            this.deleteBaseLineButton.setEnabled(true);
        }else{
            this.deleteBaseLineButton.setEnabled(false);
        }
    }//GEN-LAST:event_baselineStationsTableMouseReleased
    
    private void addBaseLineButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addBaseLineButtonActionPerformed
        DefaultTableModel bsltm = (DefaultTableModel)this.baselineStationsTable.getModel();
        String typedText=baselineStation1Text.getText();
        String typedText2=baselineStation2Text.getText();
        Vector<String> baselinePair = new Vector<String>();
        baselinePair.add(typedText);
        baselinePair.add(typedText2);
        bsltm.addRow(baselinePair);
        this.BaselineSelectionPanel.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_addBaseLineButtonActionPerformed
    
    private void helpButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_helpButtonActionPerformed
        String message = "";
        if(this.stepExplorerStepNameText.isEditable()){
            message = "Color coding in input fields while creating a new step:\n\n";
            message+="Red - Indicates that the value is not set in the parent step tree. Caution is advised.\n";
            message+="Yellow - Indicates that the value is set somewhere in its parent step tree.\n";
            message+="Green - Indicates that the value is set in the parent of the to-be created BBS Step.\n";
            message+="White - Indicates that the value has never been set and a parent step for the to-be created step is not applicable.\n";
            
        }else{
            message = "Color coding in input fields while editing an existing step:\n\n";
            message+="Red - Indicates that the value is neither set in this BBS Step or its parent step tree. Caution is advised.\n";
            message+="Yellow - Indicates that the value is not set in this BBS Step but is set somewhere in its parent step tree.\n";
            message+="Green - Indicates that the value is set in this BBS Step and thereby overrides any value set in its parent step tree.\n";
            
        }
        
        message+="\nOperations:\n\nUse NOT DEFINED when this step is/will be a multistep,\n or when this step should not perform an operation.";
        
        JOptionPane.showMessageDialog(null,message, "Step Explorer Help",JOptionPane.INFORMATION_MESSAGE);
    }//GEN-LAST:event_helpButtonActionPerformed
    
    private void stepExplorerOutputDataTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_stepExplorerOutputDataTextKeyReleased
        stepExplorerOutputDataPanel.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_stepExplorerOutputDataTextKeyReleased
    
    private void stepExplorerCorrelationTypeListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerCorrelationTypeListValueChanged
        stepExplorerCorrelationTypeList.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_stepExplorerCorrelationTypeListValueChanged
    
    private void stepExplorerCorrelationSelectionBoxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_stepExplorerCorrelationSelectionBoxItemStateChanged
        stepExplorerCorrelationSelectionBox.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_stepExplorerCorrelationSelectionBoxItemStateChanged
    
    private void integrationTimeTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_integrationTimeTextKeyReleased
        integrationTimeText.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_integrationTimeTextKeyReleased
    
    private void integrationFrequencyTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_integrationFrequencyTextKeyReleased
        integrationFrequencyText.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_integrationFrequencyTextKeyReleased
    
    private void stepExplorerInstrumentModelListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerInstrumentModelListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteInstrumentButton.setEnabled(true);
        }else{
            this.deleteInstrumentButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerInstrumentModelListValueChanged
                    
    private void stepExplorerNSourcesListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerNSourcesListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteNSourceButton.setEnabled(true);
        }else{
            this.deleteNSourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerNSourcesListValueChanged
    
    private void addNSourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addNSourceButtonActionPerformed
        String toBeAddedNSource = (String)this.stepExplorerModifyNSourceCombobox.getSelectedItem();
        DefaultListModel theSourcesModel = (DefaultListModel)stepExplorerNSourcesList.getModel();
        if(!theSourcesModel.contains(toBeAddedNSource)){
            theSourcesModel.addElement(toBeAddedNSource);
            stepExplorerNSources.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addNSourceButtonActionPerformed
    
    private void deleteNSourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteNSourceButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)stepExplorerNSourcesList.getModel();
        int[] selectedIndices = stepExplorerNSourcesList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerNSourcesList.getSelectedIndices();
            stepExplorerNSources.setBackground(NOT_INHERITED_FROM_PARENT);
        }
        if(theSourceModel.size()==0){
            this.deleteNSourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteNSourceButtonActionPerformed
        
    private void stepExplorerStepNameTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_stepExplorerStepNameTextKeyReleased
        if(stepExplorerStepNameText.isEnabled()){
            String currentText = stepExplorerStepNameText.getText();
            if(!currentText.equals("")){
                stepExplorerStepNameText.setBackground(Color.WHITE);
                stepExplorerStepNameLabel.setForeground(Color.BLACK);
            }else{
                stepExplorerStepNameText.setBackground(Color.RED);
                stepExplorerStepNameLabel.setForeground(Color.RED);
            }
        }
    }//GEN-LAST:event_stepExplorerStepNameTextKeyReleased
    
    private void stepExplorerRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepExplorerRevertButtonActionPerformed
        this.restoreBBSStepExplorerPanel();
    }//GEN-LAST:event_stepExplorerRevertButtonActionPerformed
    
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply step and close")) {
            
            boolean inputOK = validateInput();
            
            //perform input validation on step name if a new step is entered
            if(itsBBSStep == null && inputOK){
                integrationTimeText.setBackground(Color.WHITE);
                integrationFrequencyText.setBackground(Color.WHITE);
                String newStepName = this.stepExplorerStepNameText.getText();
                
                if(newStepName.equals("")){
                    inputOK=false;
                    stepExplorerStepNameText.setBackground(Color.RED);
                    stepExplorerStepNameLabel.setForeground(Color.RED);
                }else{
                    stepExplorerStepNameText.setBackground(Color.WHITE);
                    stepExplorerStepNameLabel.setForeground(Color.BLACK);
                    boolean stepExists = BBSStepDataManager.getInstance().stepExists(newStepName);
                    if(stepExists){
                        String message = "A step with name '"+newStepName+"' is already defined. Should the already defined step be overwritten with the values you entered in this dialog?";
                        String[] buttons = {"Yes","No, add the already defined step","Cancel, I will select a different name"};
                        int choice =  JOptionPane.showOptionDialog(this,message, "Step conflict detected", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
                        if(choice == 0){
                            //updating the existing step will be handled further on in the method
                        }else if(choice == 1){
                            //existing BBS step to be added
                            if(itsParentBBSStep != null){
                                itsBBSStep = BBSStepDataManager.getInstance().getBBSStep(itsParentBBSStep,newStepName);
                                itsBBSStep.setParentStep(itsParentBBSStep);
                                BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(itsParentBBSStep.getName(),itsBBSStep);
                                BBSStepDataManager.getInstance().assureBBSStepIsInCollection(itsParentBBSStep);
                            }else{
                                //adding new BBS Step to a strategy
                                
                                itsBBSStep = BBSStepDataManager.getInstance().getBBSStep(null,newStepName);
                                BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(null,itsBBSStep);
                                
                            }
                            inputOK=false;
                            //tell parents that panel is ready to close...
                            ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
                            fireActionListenerActionPerformed(closeEvt);
                        }else{
                            inputOK=false;
                        }
                    }
                }
            }
            
            
            if(inputOK){
                
                //modifying a step
                if(itsBBSStep != null){
                    //update all fields in the BBS Step Data object
                    saveInput(itsBBSStep);
                    
                }else{
                    
                    //new BBS step to be added
                    if(itsBBSStep == null && itsParentBBSStep != null){
                        itsBBSStep = BBSStepDataManager.getInstance().getBBSStep(itsParentBBSStep,this.stepExplorerStepNameText.getText());
                        BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(itsParentBBSStep.getName(),itsBBSStep);
                        BBSStepDataManager.getInstance().assureBBSStepIsInCollection(itsParentBBSStep);
                        
                    }else if(itsBBSStep == null && itsParentBBSStep == null){
                        //adding new BBS Step to a strategy
                        itsBBSStep = BBSStepDataManager.getInstance().getBBSStep(null,this.stepExplorerStepNameText.getText());
                        BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(null,itsBBSStep);
                        
                    }
                    if(itsBBSStep != null){
                        //update all fields in the BBS Step object
                        saveInput(itsBBSStep);
                        
                    }else{
                        //warning
                    }
                }
                //tell parents that panel is ready to close...
                ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
                fireActionListenerActionPerformed(closeEvt);
            }
        }
        if(evt.getActionCommand().equals("Close")){
            //tell parents that panel is ready to close...
            ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
            fireActionListenerActionPerformed(closeEvt);
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
   
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel BBSStepExplorerPanel;
    private javax.swing.JPanel BaselineSelectionPanel;
    private javax.swing.JPanel StepExplorerInstrumentModelButtonPanel;
    private javax.swing.JButton addBaseLineButton;
    private javax.swing.JButton addInstrumentButton;
    private javax.swing.JButton addNSourceButton;
    private javax.swing.JPanel baselineInputPanel;
    private javax.swing.JPanel baselineModsPanel;
    private javax.swing.JPanel baselinePanel;
    private javax.swing.JTextField baselineStation1Text;
    private javax.swing.JTextField baselineStation2Text;
    private javax.swing.JScrollPane baselineStationsScrollPane;
    private javax.swing.JTable baselineStationsTable;
    private javax.swing.JCheckBox baselineUseAllCheckbox;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deleteBaseLineButton;
    private javax.swing.JButton deleteInstrumentButton;
    private javax.swing.JButton deleteNSourceButton;
    private javax.swing.JButton helpButton;
    private javax.swing.JLabel integrationFrequencyLabel;
    private javax.swing.JTextField integrationFrequencyText;
    private javax.swing.JLabel integrationFrequencyUnitLabel;
    private javax.swing.JPanel integrationIntervalPanel;
    private javax.swing.JLabel integrationTimeLabel;
    private javax.swing.JTextField integrationTimeText;
    private javax.swing.JLabel integrationTimeUnitLabel;
    private javax.swing.JCheckBox noInstrumentModelCheckbox;
    private javax.swing.JScrollPane seOperationAttributesScrollPane;
    private javax.swing.JPanel stepExplorerCorrelationPanel;
    private javax.swing.JComboBox stepExplorerCorrelationSelectionBox;
    private javax.swing.JLabel stepExplorerCorrelationSelectionLabel;
    private javax.swing.JLabel stepExplorerCorrelationTypeLabel;
    private javax.swing.JList stepExplorerCorrelationTypeList;
    private javax.swing.JScrollPane stepExplorerCorrelationTypeScrollPane;
    private javax.swing.JPanel stepExplorerInstrumentModel;
    private javax.swing.JPanel stepExplorerInstrumentModelEditPanel;
    private javax.swing.JList stepExplorerInstrumentModelList;
    private javax.swing.JPanel stepExplorerInstrumentModelPanel;
    private javax.swing.JScrollPane stepExplorerInstrumentModelScrollPane;
    private javax.swing.JComboBox stepExplorerModifyInstrumentModelCombobox;
    private javax.swing.JComboBox stepExplorerModifyNSourceCombobox;
    private javax.swing.JPanel stepExplorerNSources;
    private javax.swing.JPanel stepExplorerNSourcesButtonPanel;
    private javax.swing.JPanel stepExplorerNSourcesEditPanel;
    private javax.swing.JList stepExplorerNSourcesList;
    private javax.swing.JPanel stepExplorerNSourcesPanel;
    private javax.swing.JScrollPane stepExplorerNSourcesScrollPane;
    private javax.swing.JPanel stepExplorerOperationAttributesPanel;
    private javax.swing.JComboBox stepExplorerOperationComboBox;
    private javax.swing.JPanel stepExplorerOperationPanel;
    private javax.swing.JPanel stepExplorerOperationTypeHeaderPanel;
    private javax.swing.JLabel stepExplorerOperationTypeLabel;
    private javax.swing.JPanel stepExplorerOutputDataPanel;
    private javax.swing.JTextField stepExplorerOutputDataText;
    private javax.swing.JPanel stepExplorerPanel;
    private javax.swing.JButton stepExplorerRevertButton;
    private javax.swing.JScrollPane stepExplorerScrollPanel;
    private javax.swing.JLabel stepExplorerStepNameLabel;
    private javax.swing.JTextField stepExplorerStepNameText;
    private javax.swing.JCheckBox useAllSourcesCheckbox;
    private javax.swing.JCheckBox writeOutputCheckbox;
    // End of variables declaration//GEN-END:variables
    
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {
        
        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        myListenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
}
