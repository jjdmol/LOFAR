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
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStep;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepDataManager;
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
    
    static Logger logger = Logger.getLogger(BBSPanel.class);
    static String name = "BBS Step Explorer";
    final static Color INHERITED_FROM_PARENT = new Color(255,255,204);
    final static Color NOT_INHERITED_FROM_PARENT = Color.WHITE;
    
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
        this.stepExplorerInstrumentModelList.setModel(new DefaultListModel());
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
        
        //sources
        stepExplorerNSourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getSources() != null && stepData.getSources().size()>0){
            this.fillList(this.stepExplorerNSourcesList,stepData.getSources());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getSources() != null && stepParentData.getSources().size()>0){
                    this.fillList(this.stepExplorerNSourcesList,stepParentData.getSources());
                    stepExplorerNSourcesList.setBackground(INHERITED_FROM_PARENT);
                } else{
                    this.fillList(this.stepExplorerNSourcesList,new Vector<String>());
                }
            }else{
                this.fillList(this.stepExplorerNSourcesList,new Vector<String>());
            }
        }
        //extra sources
        stepExplorerESourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getExtraSources() != null && stepData.getExtraSources().size()>0){
            this.fillList(this.stepExplorerESourcesList,stepData.getExtraSources());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getExtraSources() != null && stepParentData.getExtraSources().size()>0){
                    this.fillList(this.stepExplorerESourcesList,stepParentData.getExtraSources());
                    stepExplorerESourcesList.setBackground(INHERITED_FROM_PARENT);
                } else{
                    this.fillList(this.stepExplorerESourcesList,new Vector<String>());
                }
            }else{
                this.fillList(this.stepExplorerESourcesList,new Vector<String>());
            }
        }
        //output data column
        stepExplorerOutputDataText.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getOutputDataColumn() != null){
            this.stepExplorerOutputDataText.setText(stepData.getOutputDataColumn());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getOutputDataColumn() != null){
                    this.stepExplorerOutputDataText.setText(stepParentData.getOutputDataColumn());
                    stepExplorerOutputDataText.setBackground(INHERITED_FROM_PARENT);
                }
            }else{
                this.stepExplorerOutputDataText.setText("");
            }
        }
        
        //instrument model
        stepExplorerInstrumentModelList.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getInstrumentModel() != null && stepData.getInstrumentModel().size()>0){
            this.fillList(this.stepExplorerInstrumentModelList,stepData.getInstrumentModel());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getInstrumentModel() != null && stepParentData.getInstrumentModel().size()>0){
                    this.fillList(this.stepExplorerInstrumentModelList,stepParentData.getInstrumentModel());
                    stepExplorerInstrumentModelList.setBackground(INHERITED_FROM_PARENT);
                } else{
                    this.fillList(this.stepExplorerInstrumentModelList,new Vector<String>());
                }
            }else{
                this.fillList(this.stepExplorerInstrumentModelList,new Vector<String>());
            }
        }
        //integration
        //time
        this.integrationTimeText.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getIntegrationTime() != -1.0){
            this.integrationTimeText.setText(""+stepData.getIntegrationTime());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getIntegrationTime() != -1.0){
                    this.integrationTimeText.setText(""+stepParentData.getIntegrationTime());
                    integrationTimeText.setBackground(INHERITED_FROM_PARENT);
                }
            }else{
                this.integrationTimeText.setText("");
            }
        }
        //frequency
        this.integrationFrequencyText.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getIntegrationFrequency() != -1.0){
            this.integrationFrequencyText.setText(""+stepData.getIntegrationFrequency());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getIntegrationFrequency() != -1.0){
                    this.integrationFrequencyText.setText(""+stepParentData.getIntegrationFrequency());
                    integrationFrequencyText.setBackground(INHERITED_FROM_PARENT);
                }
            }else{
                this.integrationFrequencyText.setText("");
            }
        }
        //correlation
        //type
        this.stepExplorerCorrelationTypeList.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getCorrelationType() != null){
            this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,stepData.getCorrelationType());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getCorrelationType() != null){
                    this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,stepParentData.getCorrelationType());
                    stepExplorerCorrelationTypeList.setBackground(INHERITED_FROM_PARENT);
                }
            }else{
                this.fillSelectionListFromVector(stepExplorerCorrelationTypeList,new Vector<String>());
            }
        }
        //selection
        this.stepExplorerCorrelationSelectionBox.setBackground(NOT_INHERITED_FROM_PARENT);
        if(stepData.getCorrelationSelection() != null){
            this.stepExplorerCorrelationSelectionBox.setSelectedItem(stepData.getCorrelationSelection());
        }else{
            if(theBBSStep.hasParentStep()){
                BBSStepData stepParentData = BBSStepDataManager.getInstance().getStepData(theBBSStep.getParentStep().getName());
                if(stepParentData.getCorrelationSelection() != null){
                    this.stepExplorerCorrelationSelectionBox.setSelectedItem(stepParentData.getCorrelationSelection());
                    stepExplorerCorrelationSelectionBox.setBackground(INHERITED_FROM_PARENT);
                }
            }else{
                this.stepExplorerCorrelationSelectionBox.setSelectedIndex(0);
            }
        }
        //add other variables
        
        
        //TODO: Add variables from BBSStep
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
    
    private void saveInput(BBSStepData aStepData) {
        //normal sources
        Vector<String> currentNSources = aStepData.getSources();
        if(!createList(stepExplorerNSourcesList).equals(aStepData.getSources())){
            aStepData.setSources(createList(stepExplorerNSourcesList));
        }
        if(aStepData.getSources()!= null && aStepData.getSources().size()==0){
            aStepData.setSources(null);
        }
        //extra sources
        Vector<String> currentESources = aStepData.getExtraSources();
        if(!createList(stepExplorerESourcesList).equals(aStepData.getExtraSources())){
            aStepData.setExtraSources(createList(stepExplorerESourcesList));
        }
        if(aStepData.getExtraSources()!= null && aStepData.getExtraSources().size()==0){
            aStepData.setExtraSources(null);
        }
        //output data column
        if(stepExplorerOutputDataText.getText().equals("")){
            aStepData.setOutputDataColumn(null);
        }else{
            aStepData.setOutputDataColumn(stepExplorerOutputDataText.getText());
        }
        //instrument model
        Vector<String> currentInstrumentModel = aStepData.getInstrumentModel();
        if(!createList(stepExplorerInstrumentModelList).equals(aStepData.getInstrumentModel())){
            aStepData.setInstrumentModel(createList(stepExplorerInstrumentModelList));
        }
        if(aStepData.getInstrumentModel()!= null && aStepData.getInstrumentModel().size()==0){
            aStepData.setInstrumentModel(null);
        }
        //Integration
        //Time
        if(this.integrationTimeText.getText().equals("")){
            aStepData.setIntegrationTime(-1.0);
        }else{
            aStepData.setIntegrationTime(Double.parseDouble(integrationTimeText.getText()));
        }
        //Frequency
        if(this.integrationFrequencyText.getText().equals("")){
            aStepData.setIntegrationFrequency(-1.0);
        }else{
            aStepData.setIntegrationFrequency(Double.parseDouble(integrationFrequencyText.getText()));
        }
        //Correlation
        //Type
        Vector<String> currentCTypes = createVectorFromSelectionList(this.stepExplorerCorrelationTypeList);
        if(!currentCTypes.equals(aStepData.getCorrelationType())){
            aStepData.setCorrelationType(currentCTypes);
        }
        if(aStepData.getCorrelationType()!= null && aStepData.getCorrelationType().size()==0){
            aStepData.setCorrelationType(null);
        }
        //Selection
        String selectedCSelection = this.stepExplorerCorrelationSelectionBox.getSelectedItem().toString();
        if(!selectedCSelection.equals(aStepData.getCorrelationSelection())){
            aStepData.setCorrelationSelection(selectedCSelection);
        }
        
        //add other variables
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
        if (aListComponent.getModel().getSize() > 0) {
            int[] selectedIndices = aListComponent.getSelectedIndices();
            for (int i=0; i < selectedIndices.length;i++) {
                aList.add(aListComponent.getModel().getElementAt(selectedIndices[i]).toString());
            }
        }
        return aList;
    }
    private void fillSelectionListFromVector(JList aListComponent,Vector<String> theList) {
        int[] toBeSelectedIndices = new int[theList.size()];
        int aValueIndex = 0;
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
        seOperationAttributesInputPanel = new javax.swing.JPanel();
        seOperationAttributeLabel1 = new javax.swing.JLabel();
        seOperationAttributeText1 = new javax.swing.JTextField();
        seOperationAttributeText2 = new javax.swing.JTextField();
        seOperationAttributeLabel2 = new javax.swing.JLabel();
        seOperationAttributeText3 = new javax.swing.JTextField();
        seOperationAttributeLabel3 = new javax.swing.JLabel();
        seOperationAttributeGroup1 = new javax.swing.JPanel();
        seOperationAttributeLabel4 = new javax.swing.JLabel();
        seOperationAttributeLabel5 = new javax.swing.JLabel();
        seOperationAttributeText4 = new javax.swing.JTextField();
        seOperationAttributeText5 = new javax.swing.JTextField();
        seOperationAttributeUnitLabel1 = new javax.swing.JLabel();
        seOperationAttributeUnitLabel2 = new javax.swing.JLabel();
        seOperationAttributeGroup4 = new javax.swing.JPanel();
        seOperationAttributeGroup3 = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel2 = new javax.swing.JPanel();
        addSolvableParmButton1 = new javax.swing.JButton();
        modifySolvableParmButton1 = new javax.swing.JButton();
        deleteSolvableParmButton1 = new javax.swing.JButton();
        seSolvableParmScrollPane1 = new javax.swing.JScrollPane();
        seSolvableParmList1 = new javax.swing.JList();
        seOperationAttributeGroup2 = new javax.swing.JPanel();
        seOperationAttributeSolvableParmPanel = new javax.swing.JPanel();
        addSolvableParmButton = new javax.swing.JButton();
        modifySolvableParmButton = new javax.swing.JButton();
        deleteSolvableParmButton = new javax.swing.JButton();
        seSolvableParmScrollPane = new javax.swing.JScrollPane();
        seSolvableParmList = new javax.swing.JList();
        stepExplorerOutputDataPanel = new javax.swing.JPanel();
        stepExplorerOutputDataText = new javax.swing.JTextField();
        stepExplorerNSources = new javax.swing.JPanel();
        stepExplorerNSourcesPanel = new javax.swing.JPanel();
        stepExplorerNSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerNSourcesList = new javax.swing.JList();
        stepExplorerNSourcesEditPanel = new javax.swing.JPanel();
        stepExplorerModifyNSourceText = new javax.swing.JTextField();
        stepExplorerNSourcesButtonPanel = new javax.swing.JPanel();
        deleteNSourceButton = new javax.swing.JButton();
        addNSourceButton = new javax.swing.JButton();
        stepExplorerESources = new javax.swing.JPanel();
        stepExplorerESourcesPanel = new javax.swing.JPanel();
        stepExplorerESourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerESourcesList = new javax.swing.JList();
        stepExplorerESourcesEditPanel = new javax.swing.JPanel();
        stepExplorerModifyESourceText = new javax.swing.JTextField();
        stepExplorerESourcesButtonPanel = new javax.swing.JPanel();
        deleteESourceButton = new javax.swing.JButton();
        addESourceButton = new javax.swing.JButton();
        stepExplorerInstrumentModel = new javax.swing.JPanel();
        stepExplorerInstrumentModelPanel = new javax.swing.JPanel();
        stepExplorerInstrumentModelScrollPane = new javax.swing.JScrollPane();
        stepExplorerInstrumentModelList = new javax.swing.JList();
        stepExplorerInstrumentModelEditPanel = new javax.swing.JPanel();
        stepExplorerModifyInstrumentModelText = new javax.swing.JTextField();
        stepExplorerInstrumentModelButtonPanel = new javax.swing.JPanel();
        deleteInstrumentModelButton = new javax.swing.JButton();
        addInstrumentModelButton = new javax.swing.JButton();
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
        baselineStationsScrollPane = new javax.swing.JScrollPane();
        baselineStationsTable = new javax.swing.JTable();
        baselineModsPanel = new javax.swing.JPanel();
        addBaseLineButton = new javax.swing.JButton();
        deleteBaseLineButton = new javax.swing.JButton();
        baselineUseAllCheckbox = new javax.swing.JCheckBox();

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

        stepExplorerPanel.add(stepExplorerRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 640, 100, -1));

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

        stepExplorerOperationComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "SOLVE", "SUBTRACT", "CORRECT", "PREDICT", "SHIFT", "REFIT" }));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationComboBox, gridBagConstraints);

        stepExplorerOperationPanel.add(stepExplorerOperationTypeHeaderPanel, java.awt.BorderLayout.NORTH);

        stepExplorerOperationAttributesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationAttributesPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(204, 204, 204), null));
        seOperationAttributesInputPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeLabel1.setText("Max. iterations :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));
        seOperationAttributeLabel1.getAccessibleContext().setAccessibleName("MaxIter");

        seOperationAttributeText1.setText("5");
        seOperationAttributesInputPanel.add(seOperationAttributeText1, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 60, -1));
        seOperationAttributeText1.getAccessibleContext().setAccessibleName("MaxIterValue");

        seOperationAttributeText2.setText("1e-6");
        seOperationAttributesInputPanel.add(seOperationAttributeText2, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, -1));
        seOperationAttributeText2.getAccessibleContext().setAccessibleName("EpsilonValue");

        seOperationAttributeLabel2.setText("Epsilon :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));
        seOperationAttributeLabel2.getAccessibleContext().setAccessibleName("Epsilon");

        seOperationAttributeText3.setText("0.95");
        seOperationAttributesInputPanel.add(seOperationAttributeText3, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 60, -1));

        seOperationAttributeLabel3.setText("Min. converged :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, -1));
        seOperationAttributeLabel3.getAccessibleContext().setAccessibleName("Maxiterations :");

        seOperationAttributeGroup1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeGroup1.setBorder(javax.swing.BorderFactory.createTitledBorder("Domain Size"));
        seOperationAttributeLabel4.setText("Frequency :");
        seOperationAttributeGroup1.add(seOperationAttributeLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        seOperationAttributeLabel5.setText("Time :");
        seOperationAttributeGroup1.add(seOperationAttributeLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 40, -1, -1));

        seOperationAttributeText4.setText("500");
        seOperationAttributeGroup1.add(seOperationAttributeText4, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 20, 60, -1));

        seOperationAttributeText5.setText("2");
        seOperationAttributeGroup1.add(seOperationAttributeText5, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 40, 60, -1));

        seOperationAttributeUnitLabel1.setText("Hz");
        seOperationAttributeGroup1.add(seOperationAttributeUnitLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, -1, 20));

        seOperationAttributeUnitLabel2.setText("s");
        seOperationAttributeGroup1.add(seOperationAttributeUnitLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 40, 20, 20));

        seOperationAttributesInputPanel.add(seOperationAttributeGroup1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 180, 70));

        seOperationAttributeGroup4.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeGroup4.setBorder(javax.swing.BorderFactory.createTitledBorder("Parameters"));
        seOperationAttributeGroup3.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup3.setBorder(javax.swing.BorderFactory.createTitledBorder("Excluded"));
        stepExplorerSourcesModsPanel2.setLayout(new java.awt.GridBagLayout());

        addSolvableParmButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addSolvableParmButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(addSolvableParmButton1, gridBagConstraints);

        modifySolvableParmButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Edit16.gif")));
        modifySolvableParmButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        modifySolvableParmButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        modifySolvableParmButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(modifySolvableParmButton1, gridBagConstraints);

        deleteSolvableParmButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteSolvableParmButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(deleteSolvableParmButton1, gridBagConstraints);

        seOperationAttributeGroup3.add(stepExplorerSourcesModsPanel2, java.awt.BorderLayout.SOUTH);

        seSolvableParmScrollPane1.setViewportView(seSolvableParmList1);

        seOperationAttributeGroup3.add(seSolvableParmScrollPane1, java.awt.BorderLayout.CENTER);

        seOperationAttributeGroup4.add(seOperationAttributeGroup3, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 20, 160, 140));

        seOperationAttributeGroup2.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup2.setBorder(javax.swing.BorderFactory.createTitledBorder("Solvable"));
        seOperationAttributeSolvableParmPanel.setLayout(new java.awt.GridBagLayout());

        addSolvableParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(addSolvableParmButton, gridBagConstraints);

        modifySolvableParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Edit16.gif")));
        modifySolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        modifySolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        modifySolvableParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(modifySolvableParmButton, gridBagConstraints);

        deleteSolvableParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(deleteSolvableParmButton, gridBagConstraints);

        seOperationAttributeGroup2.add(seOperationAttributeSolvableParmPanel, java.awt.BorderLayout.SOUTH);

        seSolvableParmList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "GAIN:*", "PHASE:*" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        seSolvableParmScrollPane.setViewportView(seSolvableParmList);

        seOperationAttributeGroup2.add(seSolvableParmScrollPane, java.awt.BorderLayout.CENTER);

        seOperationAttributeGroup4.add(seOperationAttributeGroup2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 140));

        seOperationAttributesInputPanel.add(seOperationAttributeGroup4, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, 350, 170));

        seOperationAttributesScrollPane.setViewportView(seOperationAttributesInputPanel);

        stepExplorerOperationAttributesPanel.add(seOperationAttributesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerOperationPanel.add(stepExplorerOperationAttributesPanel, java.awt.BorderLayout.CENTER);

        stepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 400, 700, 240));

        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data Column"));
        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, 140, 20));

        stepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 330, 180, 60));

        stepExplorerNSources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerNSources.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
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
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
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
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerNSourcesButtonPanel.add(addNSourceButton, gridBagConstraints);

        stepExplorerNSourcesEditPanel.add(stepExplorerNSourcesButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerNSourcesPanel.add(stepExplorerNSourcesEditPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerNSources.add(stepExplorerNSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 150));

        stepExplorerPanel.add(stepExplorerNSources, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 0, 180, 190));

        stepExplorerESources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerESources.setBorder(javax.swing.BorderFactory.createTitledBorder("Extra Sources"));
        stepExplorerESourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerESourcesList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerESourcesListValueChanged(evt);
            }
        });

        stepExplorerESourcesScrollPane.setViewportView(stepExplorerESourcesList);

        stepExplorerESourcesPanel.add(stepExplorerESourcesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerESourcesEditPanel.setLayout(new java.awt.BorderLayout());

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
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
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
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerESourcesButtonPanel.add(addESourceButton, gridBagConstraints);

        stepExplorerESourcesEditPanel.add(stepExplorerESourcesButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerESourcesPanel.add(stepExplorerESourcesEditPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerESources.add(stepExplorerESourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 150));

        stepExplorerPanel.add(stepExplorerESources, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 0, 180, 190));

        stepExplorerInstrumentModel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerInstrumentModel.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Model"));
        stepExplorerInstrumentModelPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerInstrumentModelList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stepExplorerInstrumentModelListValueChanged(evt);
            }
        });

        stepExplorerInstrumentModelScrollPane.setViewportView(stepExplorerInstrumentModelList);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModelEditPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerModifyInstrumentModelText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                stepExplorerModifyInstrumentModelTextKeyReleased(evt);
            }
        });

        stepExplorerInstrumentModelEditPanel.add(stepExplorerModifyInstrumentModelText, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModelButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteInstrumentModelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteInstrumentModelButton.setToolTipText("Remove the selected Station from the list");
        deleteInstrumentModelButton.setEnabled(false);
        deleteInstrumentModelButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentModelButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentModelButton.setPreferredSize(new java.awt.Dimension(30, 25));
        deleteInstrumentModelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteInstrumentModelButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelButtonPanel.add(deleteInstrumentModelButton, gridBagConstraints);

        addInstrumentModelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addInstrumentModelButton.setToolTipText("Add the station entered to the list");
        addInstrumentModelButton.setEnabled(false);
        addInstrumentModelButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addInstrumentModelButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addInstrumentModelButton.setPreferredSize(new java.awt.Dimension(30, 25));
        addInstrumentModelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addInstrumentModelButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelButtonPanel.add(addInstrumentModelButton, gridBagConstraints);

        stepExplorerInstrumentModelEditPanel.add(stepExplorerInstrumentModelButtonPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelEditPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerInstrumentModel.add(stepExplorerInstrumentModelPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 150));

        stepExplorerPanel.add(stepExplorerInstrumentModel, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 200, 180, 190));

        stepExplorerCorrelationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerCorrelationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        stepExplorerCorrelationSelectionLabel.setText("Selection :");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        stepExplorerCorrelationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "AUTO", "CROSS", "ALL" }));
        stepExplorerCorrelationSelectionBox.setToolTipText("Station correlations to use.\n\nAUTO: Use only correlations of each station with itself (i.e. no base lines).Not yet implemented.\nCROSS: Use only correlations between stations (i.e. base lines).\nALL: Use both AUTO and CROSS correlations.");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 80, -1));

        stepExplorerCorrelationTypeLabel.setText("Type :");
        stepExplorerCorrelationPanel.add(stepExplorerCorrelationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 10, -1, -1));

        stepExplorerCorrelationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerCorrelationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY. \n\nAs an example, suppose you select 'XX' here and set Selection to AUTO, then the X polarization signal of each station is correlated with itself. However if we set Selection to CROSS, then the X polarization of station A is correlated with the X polarization of station B for each base line.");
        stepExplorerCorrelationTypeScrollPane.setViewportView(stepExplorerCorrelationTypeList);

        stepExplorerCorrelationPanel.add(stepExplorerCorrelationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        stepExplorerPanel.add(stepExplorerCorrelationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 200, 180, 120));

        integrationIntervalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        integrationIntervalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Integration"));
        integrationIntervalPanel.setToolTipText("Cell size for integration. Allows the user to perform operations on a lower resolution, which should be faster in most cases");
        integrationFrequencyLabel.setText("Freq. Interval :");
        integrationIntervalPanel.add(integrationFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 20, -1, -1));

        integrationIntervalPanel.add(integrationFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, 70, -1));

        integrationFrequencyUnitLabel.setText("Hz");
        integrationIntervalPanel.add(integrationFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 20, -1, -1));

        integrationTimeLabel.setText("Time Interval :");
        integrationIntervalPanel.add(integrationTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 50, -1, -1));

        integrationIntervalPanel.add(integrationTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 50, 70, -1));

        integrationTimeUnitLabel.setText("s");
        integrationIntervalPanel.add(integrationTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 50, 10, -1));

        stepExplorerPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 310, 320, 80));

        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline Selection"));
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
        baselineStationsScrollPane.setViewportView(baselineStationsTable);

        BaselineSelectionPanel.add(baselineStationsScrollPane, java.awt.BorderLayout.CENTER);

        baselineModsPanel.setLayout(new java.awt.GridBagLayout());

        addBaseLineButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addBaseLineButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 7;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(addBaseLineButton, gridBagConstraints);

        deleteBaseLineButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteBaseLineButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteBaseLineButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 6;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(deleteBaseLineButton, gridBagConstraints);

        baselineUseAllCheckbox.setText("Use all baselines");
        baselineUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        baselineUseAllCheckbox.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        baselineUseAllCheckbox.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                baselineUseAllCheckboxStateChanged(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(baselineUseAllCheckbox, gridBagConstraints);

        BaselineSelectionPanel.add(baselineModsPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerPanel.add(BaselineSelectionPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 320, 250));

        stepExplorerScrollPanel.setViewportView(stepExplorerPanel);

        BBSStepExplorerPanel.add(stepExplorerScrollPanel, java.awt.BorderLayout.CENTER);

        add(BBSStepExplorerPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    
    private void addInstrumentModelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addInstrumentModelButtonActionPerformed
        String toBeAddedInstrumentModel = this.stepExplorerModifyInstrumentModelText.getText();
        if(!toBeAddedInstrumentModel.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)stepExplorerInstrumentModelList.getModel();
            theStationModel.addElement(toBeAddedInstrumentModel);
            stepExplorerInstrumentModelList.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addInstrumentModelButtonActionPerformed
    
    private void deleteInstrumentModelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteInstrumentModelButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)stepExplorerInstrumentModelList.getModel();
        int[] selectedIndices = stepExplorerInstrumentModelList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerInstrumentModelList.getSelectedIndices();
            stepExplorerInstrumentModelList.setBackground(NOT_INHERITED_FROM_PARENT);
        }
        if(theSourceModel.size()==0){
            this.deleteInstrumentModelButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteInstrumentModelButtonActionPerformed
    
    private void stepExplorerModifyInstrumentModelTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_stepExplorerModifyInstrumentModelTextKeyReleased
        String toBeAddedInstrumentModel = stepExplorerModifyInstrumentModelText.getText();
        if(!toBeAddedInstrumentModel.equals("")){
            this.addInstrumentModelButton.setEnabled(true);
        }else{
            this.addInstrumentModelButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerModifyInstrumentModelTextKeyReleased
    
    private void stepExplorerInstrumentModelListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stepExplorerInstrumentModelListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteInstrumentModelButton.setEnabled(true);
        }else{
            this.deleteInstrumentModelButton.setEnabled(false);
        }
    }//GEN-LAST:event_stepExplorerInstrumentModelListValueChanged
    
    private void addESourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addESourceButtonActionPerformed
        String toBeAddedESource = this.stepExplorerModifyESourceText.getText();
        if(!toBeAddedESource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)stepExplorerESourcesList.getModel();
            theStationModel.addElement(toBeAddedESource);
            stepExplorerESourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addESourceButtonActionPerformed
    
    private void deleteESourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteESourceButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)stepExplorerESourcesList.getModel();
        int[] selectedIndices = stepExplorerESourcesList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerESourcesList.getSelectedIndices();
            stepExplorerESourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
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
            stepExplorerNSourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addNSourceButtonActionPerformed
    
    private void deleteNSourceButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteNSourceButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)stepExplorerNSourcesList.getModel();
        int[] selectedIndices = stepExplorerNSourcesList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = stepExplorerNSourcesList.getSelectedIndices();
            stepExplorerNSourcesList.setBackground(NOT_INHERITED_FROM_PARENT);
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
    
    private void baselineUseAllCheckboxStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_baselineUseAllCheckboxStateChanged
        if(baselineUseAllCheckbox.isSelected()){
            this.baselineStationsTable.setBackground(Color.LIGHT_GRAY);
            this.baselineStationsTable.setEnabled(false);
        }else{
            this.baselineStationsTable.setBackground(Color.WHITE);
            this.baselineStationsTable.setEnabled(true);
        }
        
    }//GEN-LAST:event_baselineUseAllCheckboxStateChanged
    
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save step and close") {
            boolean warning = false;
            //perform input validation on the double values in the form
            String integrationTime = this.integrationTimeText.getText();
            String integrationFrequency = this.integrationFrequencyText.getText();
            
            if(!integrationTime.equals("")){
                try {
                    Double itime = Double.parseDouble(integrationTime);
                    integrationTimeText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    integrationTimeText.setBackground(Color.RED);
                    warning=true;
                }
            }
            if(!integrationFrequency.equals("")){
                 try {
                    Double itime = Double.parseDouble(integrationFrequency);
                    integrationFrequencyText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    warning=true;
                    integrationFrequencyText.setBackground(Color.RED);
                }
            }
            
            //perform input validation on step name if a new step is entered
            if(itsBBSStep == null && !warning){
                integrationTimeText.setBackground(Color.WHITE);
                integrationFrequencyText.setBackground(Color.WHITE);
                String newStepName = this.stepExplorerStepNameText.getText();
                
                if(newStepName.equals("")){
                    warning=true;
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
                            //update all fields in the BBS Step Data object
                            saveInput(BBSStepDataManager.getInstance().getStepData(newStepName));
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
                            warning=true;
                            //tell parents that panel is ready to close...
                            ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
                            fireActionListenerActionPerformed(closeEvt);
                        }else{
                            warning=true;
                        }
                    }
                }
            }
            
            if(!warning){
                
                //modifying a step
                if(itsBBSStep != null){
                    //update all fields in the BBS Step Data object
                    saveInput(BBSStepDataManager.getInstance().getStepData(itsBBSStep.getName()));
                    
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
                        saveInput(BBSStepDataManager.getInstance().getStepData(itsBBSStep.getName()));
                        
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
    private javax.swing.JButton addInstrumentModelButton;
    private javax.swing.JButton addNSourceButton;
    private javax.swing.JButton addSolvableParmButton;
    private javax.swing.JButton addSolvableParmButton1;
    private javax.swing.JPanel baselineModsPanel;
    private javax.swing.JScrollPane baselineStationsScrollPane;
    private javax.swing.JTable baselineStationsTable;
    private javax.swing.JCheckBox baselineUseAllCheckbox;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deleteBaseLineButton;
    private javax.swing.JButton deleteESourceButton;
    private javax.swing.JButton deleteInstrumentModelButton;
    private javax.swing.JButton deleteNSourceButton;
    private javax.swing.JButton deleteSolvableParmButton;
    private javax.swing.JButton deleteSolvableParmButton1;
    private javax.swing.JLabel integrationFrequencyLabel;
    private javax.swing.JTextField integrationFrequencyText;
    private javax.swing.JLabel integrationFrequencyUnitLabel;
    private javax.swing.JPanel integrationIntervalPanel;
    private javax.swing.JLabel integrationTimeLabel;
    private javax.swing.JTextField integrationTimeText;
    private javax.swing.JLabel integrationTimeUnitLabel;
    private javax.swing.JButton modifySolvableParmButton;
    private javax.swing.JButton modifySolvableParmButton1;
    private javax.swing.JPanel seOperationAttributeGroup1;
    private javax.swing.JPanel seOperationAttributeGroup2;
    private javax.swing.JPanel seOperationAttributeGroup3;
    private javax.swing.JPanel seOperationAttributeGroup4;
    private javax.swing.JLabel seOperationAttributeLabel1;
    private javax.swing.JLabel seOperationAttributeLabel2;
    private javax.swing.JLabel seOperationAttributeLabel3;
    private javax.swing.JLabel seOperationAttributeLabel4;
    private javax.swing.JLabel seOperationAttributeLabel5;
    private javax.swing.JPanel seOperationAttributeSolvableParmPanel;
    private javax.swing.JTextField seOperationAttributeText1;
    private javax.swing.JTextField seOperationAttributeText2;
    private javax.swing.JTextField seOperationAttributeText3;
    private javax.swing.JTextField seOperationAttributeText4;
    private javax.swing.JTextField seOperationAttributeText5;
    private javax.swing.JLabel seOperationAttributeUnitLabel1;
    private javax.swing.JLabel seOperationAttributeUnitLabel2;
    private javax.swing.JPanel seOperationAttributesInputPanel;
    private javax.swing.JScrollPane seOperationAttributesScrollPane;
    private javax.swing.JList seSolvableParmList;
    private javax.swing.JList seSolvableParmList1;
    private javax.swing.JScrollPane seSolvableParmScrollPane;
    private javax.swing.JScrollPane seSolvableParmScrollPane1;
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
    private javax.swing.JPanel stepExplorerInstrumentModelButtonPanel;
    private javax.swing.JPanel stepExplorerInstrumentModelEditPanel;
    private javax.swing.JList stepExplorerInstrumentModelList;
    private javax.swing.JPanel stepExplorerInstrumentModelPanel;
    private javax.swing.JScrollPane stepExplorerInstrumentModelScrollPane;
    private javax.swing.JTextField stepExplorerModifyESourceText;
    private javax.swing.JTextField stepExplorerModifyInstrumentModelText;
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
    private javax.swing.JPanel stepExplorerSourcesModsPanel2;
    private javax.swing.JLabel stepExplorerStepNameLabel;
    private javax.swing.JTextField stepExplorerStepNameText;
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
