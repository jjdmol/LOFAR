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

package nl.astron.lofar.sas.otbcomponents.bbs;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.util.HashMap;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.table.DefaultTableModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStep;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepDataManager;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations.IBBSStepOperationPanel;
import org.apache.log4j.Logger;

/**
 * Panel for BBS Step Explorer
 *
 * @created 11-07-2006, 13:37
 *
 * @author  pompert
 *
 * @version $Id$
 */
public class BBSStepExplorerPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(BBSStepExplorerPanel.class);
    static String name = "BBS Step Explorer";
    public final static Color INHERITED_FROM_PARENT = new Color(255,255,204);
    public final static Color NOT_DEFINED = new Color(255,204,204);
    public final static Color NOT_INHERITED_FROM_PARENT = new Color(204,255,204);
    public final static Color DEFAULT = Color.WHITE;
    
    private static HashMap<String,String> stepOperationPanels = new HashMap<String,String>();
    private IBBSStepOperationPanel currentStepOperationsPanel = null;
    
    
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public BBSStepExplorerPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public BBSStepExplorerPanel() {
        initComponents();
        initialize();
        
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        if(anObject instanceof jOTDBnode){
            itsNode=(jOTDBnode)anObject;
        }
        initPanel();
    }
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
        initPanel();
    }
    
    
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new BBSPanel();
    }
    public boolean hasPopupMenu() {
        return false;
    }
    /** create popup menu for this panel
     *
     *  // build up the menu
     *  aPopupMenu= new JPopupMenu();
     *  aMenuItem=new JMenuItem("Choice 1");
     *  aMenuItem.addActionListener(new java.awt.event.ActionListener() {
     *      public void actionPerformed(java.awt.event.ActionEvent evt) {
     *          popupMenuHandler(evt);
     *      }
     *  });
     *  aMenuItem.setActionCommand("Choice 1");
     *  aPopupMenu.add(aMenuItem);
     *  aPopupMenu.setOpaque(true);
     *
     *
     *  aPopupMenu.show(aComponent, x, y );
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        //  Fill in menu as in the example above
    }
    /** handles the choice from the popupmenu
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
    }
    
    /** Restore original Values in Global Settings panel
     */
    private void restoreBBSStepExplorerPanel() {
        
        // Global Settings parameters
        if(itsBBSStep != null){
            this.stepExplorerStepNameText.setText(itsBBSStep.getName());
            this.fillBBSGui(itsBBSStep);
        }
    }
    
    private void initialize() {
        buttonPanel1.addButton("Save step and close");
        buttonPanel1.addButton("Close");
        if(itsBBSStep == null){
            stepExplorerStepNameText.setEditable(true);
        }
        this.stepExplorerNSourcesList.setModel(new DefaultListModel());
        this.stepExplorerESourcesList.setModel(new DefaultListModel());
        
        //fill the supported step operation panels
        stepOperationPanels.put("Predict",null);
        stepOperationPanels.put("Solve","nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations.BBSStepOperationPanelSolveImpl");
        stepOperationPanels.put("Subtract",null);
        stepOperationPanels.put("Correct",null);
        
    }
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        
        // for now:
        setAllEnabled(true);
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        if (itsBBSStep != null) {
            // [TODO]
            // Fill from existing cfg needed ????
        } else {
            logger.debug("ERROR:  no BBS Step given");
        }
        
    }
    /* Set's the different fields in the GUI */
    private void fillBBSGui(BBSStep theBBSStep) {
        
        this.stepExplorerStepNameText.setText(theBBSStep.getName());
        BBSStepData stepData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getName());
        BBSStepData inheritedData = BBSStepDataManager.getInstance().getInheritedStepData(theBBSStep);
        //sources
        stepExplorerNSources.setBackground(DEFAULT);
        this.stepExplorerModifyNSourceText.setText("");
        this.fillList(this.stepExplorerNSourcesList,new Vector<String>());
        if(stepData.getSources() != null){
            if(stepData.getSources().size()>0){
                this.fillList(this.stepExplorerNSourcesList,stepData.getSources());
                this.useAllSourcesCheckbox.setSelected(false);
            }else{
                this.useAllSourcesCheckbox.setSelected(true);
            }
            stepExplorerNSources.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getSources() != null){
                
                if(inheritedData.getSources().size()>0){
                    this.fillList(this.stepExplorerNSourcesList,inheritedData.getSources());
                    this.useAllSourcesCheckbox.setSelected(false);
                }else{
                    this.useAllSourcesCheckbox.setSelected(true);
                }
                stepExplorerNSources.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.useAllSourcesCheckbox.setSelected(false);
                stepExplorerNSources.setBackground(NOT_DEFINED);
            }
        }
        //extra sources
        stepExplorerESources.setBackground(DEFAULT);
        this.stepExplorerModifyESourceText.setText("");
        this.fillList(this.stepExplorerESourcesList,new Vector<String>());
        if(stepData.getExtraSources() != null){
            if(stepData.getExtraSources().size()>0){
                this.fillList(this.stepExplorerESourcesList,stepData.getExtraSources());
                this.useExtraSourcesCheckbox.setSelected(true);
            }else{
                this.useExtraSourcesCheckbox.setSelected(false);
            }
            stepExplorerESources.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getExtraSources() != null){
                if(inheritedData.getExtraSources().size()>0){
                    this.fillList(this.stepExplorerESourcesList,inheritedData.getExtraSources());
                    this.useExtraSourcesCheckbox.setSelected(true);
                }else{
                    this.useExtraSourcesCheckbox.setSelected(false);
                }
                stepExplorerESources.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.useExtraSourcesCheckbox.setSelected(true);
                stepExplorerESources.setBackground(NOT_DEFINED);
            }
        }
        //output data column
        
        this.stepExplorerOutputDataPanel.setBackground(DEFAULT);
        this.stepExplorerOutputDataText.setText("");
        if(stepData.getOutputDataColumn() != null){
            if(!stepData.getOutputDataColumn().equals("")){
                this.stepExplorerOutputDataText.setText(stepData.getOutputDataColumn());
                this.writeOutputCheckbox.setSelected(false);
            }else{
                this.writeOutputCheckbox.setSelected(true);
            }
            stepExplorerOutputDataPanel.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getOutputDataColumn() != null){
                if(!inheritedData.getOutputDataColumn().equals("")){
                    this.stepExplorerOutputDataText.setText(inheritedData.getOutputDataColumn());
                    this.writeOutputCheckbox.setSelected(false);
                }else{
                    this.writeOutputCheckbox.setSelected(true);
                }
                stepExplorerOutputDataPanel.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.writeOutputCheckbox.setSelected(false);
                stepExplorerOutputDataPanel.setBackground(NOT_DEFINED);
            }
        }
        //instrument model
        
        stepExplorerInstrumentModel.setBackground(DEFAULT);
        this.stepExplorerInstrumentModelList.clearSelection();
        if(stepData.getInstrumentModel() != null){
            if(stepData.getInstrumentModel().size()>0){
                this.fillSelectionListFromVector(this.stepExplorerInstrumentModelList,stepData.getInstrumentModel());
                this.noInstrumentModelCheckbox.setSelected(false);
            }else{
                this.noInstrumentModelCheckbox.setSelected(true);
            }
            stepExplorerInstrumentModel.setBackground(NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(inheritedData.getInstrumentModel() != null){
                
                if(inheritedData.getInstrumentModel().size()>0){
                    this.fillSelectionListFromVector(this.stepExplorerInstrumentModelList,inheritedData.getInstrumentModel());
                    this.noInstrumentModelCheckbox.setSelected(false);
                }else{
                    this.noInstrumentModelCheckbox.setSelected(true);
                }
                stepExplorerInstrumentModel.setBackground(INHERITED_FROM_PARENT);
            }else{
                this.noInstrumentModelCheckbox.setSelected(false);
                stepExplorerInstrumentModel.setBackground(NOT_DEFINED);
            }
        }
        
        //integration
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
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.enableStepExplorerButtons(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.setStepExplorerButtonsVisible(visible);
    }
    private void enableStepExplorerButtons(boolean enabled) {
        
    }
    
    private void setStepExplorerButtonsVisible(boolean visible) {
        
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableStepExplorerButtons(enabled);
    }
    
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
        
        //extra sources
        Vector<String> esources = createList(stepExplorerESourcesList);
        if(!this.useExtraSourcesCheckbox.isSelected()){
            esources = new Vector<String>();
        }
        if(esources.equals(inheritedData.getExtraSources())){
            aStepData.setExtraSources(null);
        }else{
            if(!this.useExtraSourcesCheckbox.isSelected()){
                aStepData.setExtraSources(new Vector<String>());
            }else{
                if(esources.size()>0){
                    aStepData.setExtraSources(new Vector<String>());
                }else{
                    aStepData.setExtraSources(null);
                }
                
            }
        }
        if(aStepData.getExtraSources()!=null){
            if(!esources.equals(aStepData.getExtraSources())){
                aStepData.setExtraSources(esources);
            }
        }
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
        
        //instrument model        
        
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
    
    private Vector<String> createList(JList aListComponent) {
        Vector<String> aList = new Vector<String>();
        if (aListComponent.getModel().getSize() > 0) {
            for (int i=0; i < aListComponent.getModel().getSize();i++) {
                aList.add(aListComponent.getModel().getElementAt(i).toString());
            }
        }
        return aList;
    }
    private void fillList(JList aListComponent,Vector<String> theList) {
        DefaultListModel itsModel = new DefaultListModel();
        aListComponent.setModel(itsModel);
        for(String anItem : theList){
            itsModel.addElement(anItem);
        }
        aListComponent.setModel(itsModel);
    }
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
    private void checkBaselineInput(){
        String typedText=baselineStation1Text.getText();
        String typedText2=baselineStation2Text.getText();
        if(!typedText.equals("") && !typedText2.equals("")){
            this.addBaseLineButton.setEnabled(true);
        }else{
            this.addBaseLineButton.setEnabled(false);
        }
    }
    
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
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
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
        stepExplorerNSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerNSourcesList = new javax.swing.JList();
        stepExplorerNSourcesEditPanel = new javax.swing.JPanel();
        stepExplorerModifyNSourceText = new javax.swing.JTextField();
        stepExplorerNSourcesButtonPanel = new javax.swing.JPanel();
        deleteNSourceButton = new javax.swing.JButton();
        addNSourceButton = new javax.swing.JButton();
        useAllSourcesCheckbox = new javax.swing.JCheckBox();
        stepExplorerESources = new javax.swing.JPanel();
        stepExplorerESourcesPanel = new javax.swing.JPanel();
        stepExplorerESourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerESourcesList = new javax.swing.JList();
        stepExplorerESourcesEditPanel = new javax.swing.JPanel();
        stepExplorerModifyESourceText = new javax.swing.JTextField();
        stepExplorerESourcesButtonPanel = new javax.swing.JPanel();
        deleteESourceButton = new javax.swing.JButton();
        addESourceButton = new javax.swing.JButton();
        useExtraSourcesCheckbox = new javax.swing.JCheckBox();
        stepExplorerInstrumentModel = new javax.swing.JPanel();
        stepExplorerInstrumentModelPanel = new javax.swing.JPanel();
        stepExplorerInstrumentModelScrollPane = new javax.swing.JScrollPane();
        stepExplorerInstrumentModelList = new javax.swing.JList();
        noInstrumentModelCheckbox = new javax.swing.JCheckBox();
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

        stepExplorerRevertButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Undo16.gif")));
        stepExplorerRevertButton.setText("Revert");
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

        stepExplorerOperationPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Operation"));
        stepExplorerOperationTypeHeaderPanel.setLayout(new java.awt.GridBagLayout());

        stepExplorerOperationTypeHeaderPanel.setBackground(new java.awt.Color(204, 204, 204));
        stepExplorerOperationTypeLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        stepExplorerOperationTypeLabel.setText("Type :");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationTypeLabel, gridBagConstraints);

        stepExplorerOperationComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "NOT DEFINED", "Predict", "Solve", "Subtract", "Correct" }));
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

        stepExplorerOperationAttributesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationAttributesPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(204, 204, 204), null));
        stepExplorerOperationAttributesPanel.add(seOperationAttributesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerOperationPanel.add(stepExplorerOperationAttributesPanel, java.awt.BorderLayout.CENTER);

        stepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 400, 700, 240));

        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data Column"));
        stepExplorerOutputDataText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerOutputDataTextKeyReleased(evt);
            }
        });

        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 150, 20));

        writeOutputCheckbox.setText("No output");
        writeOutputCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        writeOutputCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        writeOutputCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                writeOutputCheckboxItemStateChanged(evt);
            }
        });

        stepExplorerOutputDataPanel.add(writeOutputCheckbox, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        stepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 300, 170, 90));

        stepExplorerNSources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerNSources.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
        stepExplorerNSources.setMinimumSize(new java.awt.Dimension(150, 150));
        stepExplorerNSources.setPreferredSize(new java.awt.Dimension(150, 150));
        stepExplorerNSourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerNSourcesList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerNSourcesListValueChanged(evt);
            }
        });

        stepExplorerNSourcesScrollPane.setViewportView(stepExplorerNSourcesList);

        stepExplorerNSourcesPanel.add(stepExplorerNSourcesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerNSourcesEditPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerModifyNSourceText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerModifyNSourceTextKeyReleased(evt);
            }
        });

        stepExplorerNSourcesEditPanel.add(stepExplorerModifyNSourceText, java.awt.BorderLayout.CENTER);

        stepExplorerNSourcesButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteNSourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteNSourceButton.setToolTipText("Remove the selected Station from the list");
        deleteNSourceButton.setEnabled(false);
        deleteNSourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteNSourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteNSourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        addNSourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addNSourceButton.setToolTipText("Add the station entered to the list");
        addNSourceButton.setEnabled(false);
        addNSourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addNSourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addNSourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        stepExplorerNSourcesEditPanel.add(stepExplorerNSourcesButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerNSourcesPanel.add(stepExplorerNSourcesEditPanel, java.awt.BorderLayout.SOUTH);

        useAllSourcesCheckbox.setText("Use all sources");
        useAllSourcesCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        useAllSourcesCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        useAllSourcesCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                useAllSourcesCheckboxItemStateChanged(evt);
            }
        });

        stepExplorerNSourcesPanel.add(useAllSourcesCheckbox, java.awt.BorderLayout.NORTH);

        stepExplorerNSources.add(stepExplorerNSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 150, 130));

        stepExplorerPanel.add(stepExplorerNSources, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 0, 170, 160));

        stepExplorerESources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerESources.setBorder(javax.swing.BorderFactory.createTitledBorder("Extra Sources"));
        stepExplorerESourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerESourcesList.setBackground(java.awt.Color.lightGray);
        stepExplorerESourcesList.setEnabled(false);
        stepExplorerESourcesList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerESourcesListValueChanged(evt);
            }
        });

        stepExplorerESourcesScrollPane.setViewportView(stepExplorerESourcesList);

        stepExplorerESourcesPanel.add(stepExplorerESourcesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerESourcesEditPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerModifyESourceText.setEnabled(false);
        stepExplorerModifyESourceText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerModifyESourceTextKeyReleased(evt);
            }
        });

        stepExplorerESourcesEditPanel.add(stepExplorerModifyESourceText, java.awt.BorderLayout.CENTER);

        stepExplorerESourcesButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteESourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteESourceButton.setToolTipText("Remove the selected Station from the list");
        deleteESourceButton.setEnabled(false);
        deleteESourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteESourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteESourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
        deleteESourceButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteESourceButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerESourcesButtonPanel.add(deleteESourceButton, gridBagConstraints);

        addESourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addESourceButton.setToolTipText("Add the station entered to the list");
        addESourceButton.setEnabled(false);
        addESourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addESourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addESourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
        addESourceButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addESourceButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerESourcesButtonPanel.add(addESourceButton, gridBagConstraints);

        stepExplorerESourcesEditPanel.add(stepExplorerESourcesButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerESourcesPanel.add(stepExplorerESourcesEditPanel, java.awt.BorderLayout.SOUTH);

        useExtraSourcesCheckbox.setText("Use extra sources :");
        useExtraSourcesCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        useExtraSourcesCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        useExtraSourcesCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                useExtraSourcesCheckboxItemStateChanged(evt);
            }
        });

        stepExplorerESourcesPanel.add(useExtraSourcesCheckbox, java.awt.BorderLayout.NORTH);

        stepExplorerESources.add(stepExplorerESourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 150, 130));

        stepExplorerPanel.add(stepExplorerESources, new org.netbeans.lib.awtextra.AbsoluteConstraints(530, 0, 170, 160));

        stepExplorerInstrumentModel.setLayout(new java.awt.BorderLayout());

        stepExplorerInstrumentModel.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Model"));
        stepExplorerInstrumentModelPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerInstrumentModelList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "TOTALGAIN", "PATCHGAIN", "BANDPASS" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerInstrumentModelList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerInstrumentModelListValueChanged(evt);
            }
        });

        stepExplorerInstrumentModelScrollPane.setViewportView(stepExplorerInstrumentModelList);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModel.add(stepExplorerInstrumentModelPanel, java.awt.BorderLayout.CENTER);

        noInstrumentModelCheckbox.setText("No model");
        noInstrumentModelCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        noInstrumentModelCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        noInstrumentModelCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                noInstrumentModelCheckboxItemStateChanged(evt);
            }
        });

        stepExplorerInstrumentModel.add(noInstrumentModelCheckbox, java.awt.BorderLayout.NORTH);

        stepExplorerPanel.add(stepExplorerInstrumentModel, new org.netbeans.lib.awtextra.AbsoluteConstraints(530, 170, 170, 120));

        stepExplorerCorrelationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerCorrelationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        stepExplorerCorrelationSelectionLabel.setText("Selection :");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        stepExplorerCorrelationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "N/A", "AUTO", "CROSS", "ALL" }));
        stepExplorerCorrelationSelectionBox.setToolTipText("Station correlations to use.\n\nAUTO: Use only correlations of each station with itself (i.e. no base lines).Not yet implemented.\nCROSS: Use only correlations between stations (i.e. base lines).\nALL: Use both AUTO and CROSS correlations.");
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
        stepExplorerCorrelationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY. \n\nAs an example, suppose you select 'XX' here and set Selection to AUTO, then the X polarization signal of each station is correlated with itself. However if we set Selection to CROSS, then the X polarization of station A is correlated with the X polarization of station B for each base line.");
        stepExplorerCorrelationTypeList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerCorrelationTypeListValueChanged(evt);
            }
        });

        stepExplorerCorrelationTypeScrollPane.setViewportView(stepExplorerCorrelationTypeList);

        stepExplorerCorrelationPanel.add(stepExplorerCorrelationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        stepExplorerPanel.add(stepExplorerCorrelationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 170, 170, 120));

        integrationIntervalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        integrationIntervalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Integration"));
        integrationIntervalPanel.setToolTipText("Cell size for integration. Allows the user to perform operations on a lower resolution, which should be faster in most cases");
        integrationFrequencyLabel.setText("Freq. Interval :");
        integrationIntervalPanel.add(integrationFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 20, -1, -1));

        integrationFrequencyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                integrationFrequencyTextKeyReleased(evt);
            }
        });

        integrationIntervalPanel.add(integrationFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, 70, -1));

        integrationFrequencyUnitLabel.setText("Hz");
        integrationIntervalPanel.add(integrationFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 20, -1, -1));

        integrationTimeLabel.setText("Time Interval :");
        integrationIntervalPanel.add(integrationTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 50, -1, -1));

        integrationTimeText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                integrationTimeTextKeyReleased(evt);
            }
        });

        integrationIntervalPanel.add(integrationTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 50, 70, -1));

        integrationTimeUnitLabel.setText("s");
        integrationIntervalPanel.add(integrationTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 50, 10, -1));

        stepExplorerPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 310, 320, 80));

        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline Selection"));
        baselinePanel.setLayout(new java.awt.BorderLayout());

        baselineStationsScrollPane.setPreferredSize(new java.awt.Dimension(453, 250));
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

        baselineUseAllCheckbox.setText("Use all baselines");
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

        addBaseLineButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addBaseLineButton.setEnabled(false);
        addBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        deleteBaseLineButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteBaseLineButton.setEnabled(false);
        deleteBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setPreferredSize(new java.awt.Dimension(30, 25));
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
    
    private void noInstrumentModelCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_noInstrumentModelCheckboxItemStateChanged
        if(this.noInstrumentModelCheckbox.isSelected()){
            this.stepExplorerInstrumentModelList.setEnabled(false);
            this.stepExplorerInstrumentModelList.setBackground(Color.LIGHT_GRAY);
            this.stepExplorerInstrumentModel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerInstrumentModelList.clearSelection();
        }else{
            this.stepExplorerInstrumentModelList.setEnabled(true);
            this.stepExplorerInstrumentModelList.setBackground(Color.WHITE);
            this.stepExplorerInstrumentModel.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerInstrumentModelList.clearSelection();
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
    
    private void useExtraSourcesCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_useExtraSourcesCheckboxItemStateChanged
        if(this.useExtraSourcesCheckbox.isSelected()){
            this.stepExplorerESourcesList.setEnabled(true);
            this.stepExplorerESourcesList.setBackground(Color.WHITE);
            this.stepExplorerESources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerESourcesList.clearSelection();
            this.stepExplorerModifyESourceText.setText("");
            this.stepExplorerModifyESourceText.setEnabled(true);
        }else{
            this.stepExplorerESourcesList.setEnabled(false);
            this.stepExplorerESourcesList.setBackground(Color.LIGHT_GRAY);
            this.stepExplorerESources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerESourcesList.clearSelection();
            this.stepExplorerModifyESourceText.setEnabled(false);
            this.stepExplorerModifyESourceText.setText("");
        }
    }//GEN-LAST:event_useExtraSourcesCheckboxItemStateChanged
    
    private void useAllSourcesCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_useAllSourcesCheckboxItemStateChanged
        if(this.useAllSourcesCheckbox.isSelected()){
            this.stepExplorerNSourcesList.setEnabled(false);
            this.stepExplorerNSourcesList.setBackground(Color.LIGHT_GRAY);
            this.stepExplorerNSources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerNSourcesList.clearSelection();
            this.deleteNSourceButton.setEnabled(false);
            this.addNSourceButton.setEnabled(false);
            this.stepExplorerModifyNSourceText.setEnabled(false);
        }else{
            this.stepExplorerNSourcesList.setEnabled(true);
            this.stepExplorerNSourcesList.setBackground(Color.WHITE);
            this.stepExplorerNSources.setBackground(this.NOT_INHERITED_FROM_PARENT);
            this.stepExplorerNSourcesList.clearSelection();
            this.stepExplorerModifyNSourceText.setEnabled(true);
            this.stepExplorerModifyNSourceText.setText("");
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
        this.stepExplorerInstrumentModel.setBackground(NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_stepExplorerInstrumentModelListValueChanged
    
    private void addESourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addESourceButtonActionPerformed
        String toBeAddedESource = this.stepExplorerModifyESourceText.getText();
        if(!toBeAddedESource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)stepExplorerESourcesList.getModel();
            theStationModel.addElement(toBeAddedESource);
            stepExplorerESources.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addESourceButtonActionPerformed
    
    private void deleteESourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteESourceButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)stepExplorerESourcesList.getModel();
        int[] selectedIndices = stepExplorerESourcesList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerESourcesList.getSelectedIndices();
            stepExplorerESources.setBackground(NOT_INHERITED_FROM_PARENT);
        }
        if(theSourceModel.size()==0){
            this.deleteESourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteESourceButtonActionPerformed
    
    private void stepExplorerModifyESourceTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_stepExplorerModifyESourceTextKeyReleased
        String toBeAddedESource = stepExplorerModifyESourceText.getText();
        if(!toBeAddedESource.equals("")){
            this.addESourceButton.setEnabled(true);
        }else{
            this.addESourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerModifyESourceTextKeyReleased
    
    private void stepExplorerESourcesListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerESourcesListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteESourceButton.setEnabled(true);
        }else{
            this.deleteESourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerESourcesListValueChanged
    
    private void stepExplorerNSourcesListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerNSourcesListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteNSourceButton.setEnabled(true);
        }else{
            this.deleteNSourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerNSourcesListValueChanged
    
    private void addNSourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addNSourceButtonActionPerformed
        String toBeAddedNSource = this.stepExplorerModifyNSourceText.getText();
        if(!toBeAddedNSource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)stepExplorerNSourcesList.getModel();
            theStationModel.addElement(toBeAddedNSource);
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
    
    private void stepExplorerModifyNSourceTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_stepExplorerModifyNSourceTextKeyReleased
        String toBeAddedNSource = stepExplorerModifyNSourceText.getText();
        if(!toBeAddedNSource.equals("")){
            this.addNSourceButton.setEnabled(true);
        }else{
            this.addNSourceButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerModifyNSourceTextKeyReleased
    
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
        if(evt.getActionCommand() == "Save step and close") {
            
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
        if(evt.getActionCommand() == "Close"){
            //tell parents that panel is ready to close...
            ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
            fireActionListenerActionPerformed(closeEvt);
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private BBSStep itsBBSStep = null;
    private BBSStep itsParentBBSStep = null;
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel BBSStepExplorerPanel;
    private javax.swing.JPanel BaselineSelectionPanel;
    private javax.swing.JButton addBaseLineButton;
    private javax.swing.JButton addESourceButton;
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
    private javax.swing.JButton deleteESourceButton;
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
    private javax.swing.JPanel stepExplorerESources;
    private javax.swing.JPanel stepExplorerESourcesButtonPanel;
    private javax.swing.JPanel stepExplorerESourcesEditPanel;
    private javax.swing.JList stepExplorerESourcesList;
    private javax.swing.JPanel stepExplorerESourcesPanel;
    private javax.swing.JScrollPane stepExplorerESourcesScrollPane;
    private javax.swing.JPanel stepExplorerInstrumentModel;
    private javax.swing.JList stepExplorerInstrumentModelList;
    private javax.swing.JPanel stepExplorerInstrumentModelPanel;
    private javax.swing.JScrollPane stepExplorerInstrumentModelScrollPane;
    private javax.swing.JTextField stepExplorerModifyESourceText;
    private javax.swing.JTextField stepExplorerModifyNSourceText;
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
    private javax.swing.JCheckBox useExtraSourcesCheckbox;
    private javax.swing.JCheckBox writeOutputCheckbox;
    // End of variables declaration//GEN-END:variables
    
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {
        
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        listenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
}
