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
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStep;
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
            
        }else if(itsStep == null && itsParentBBSStep != null){
            //creating a new step under a given parent step
            this.itsParentBBSStep = itsParentBBSStep;
            
        }else if (itsStep ==null && itsParentBBSStep == null){
            //assume a new strategy step will be generated
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
        }
    }
    
    private void initialize() {
        buttonPanel1.addButton("Save Settings");
        if(itsBBSStep == null){
            stepExplorerStepNameText.setEditable(true);
        }
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
        if(theBBSStep.getOutputDataColumn() != null){
            this.stepExplorerOutputDataText.setText(theBBSStep.getOutputDataColumn());
        }else{
            
            this.stepExplorerOutputDataText.setText("");
        }
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
        this.stepExplorerRevertButton.setEnabled(enabled);
    }
    
    private void setStepExplorerButtonsVisible(boolean visible) {
        this.stepExplorerRevertButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableStepExplorerButtons(enabled);
    }
    
    private void saveInput(BBSStep aStep) {
        aStep.setOutputDataColumn(stepExplorerOutputDataText.getText());
        //add other variables
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
        explorePreviousStepButton = new javax.swing.JButton();
        exploreParentStepButton = new javax.swing.JButton();
        exploreFirstChildStepButton = new javax.swing.JButton();
        exploreNextStepButton = new javax.swing.JButton();
        stepExplorerRevertButton = new javax.swing.JButton();
        stepExplorerInstrumentModelPanel = new javax.swing.JPanel();
        stepExplorerInstrumentModelScrollPane = new javax.swing.JScrollPane();
        stepExplorerInstrumentModelList = new javax.swing.JList();
        stepExplorerInstrumentModelModsPanel = new javax.swing.JPanel();
        addInstrumentModelButton = new javax.swing.JButton();
        modifyInstrumentModelButton = new javax.swing.JButton();
        deleteInstrumentModelButton = new javax.swing.JButton();
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
        baselineGlobalPanel = new javax.swing.JPanel();
        baseLineCorrelationPanel = new javax.swing.JPanel();
        blCorrelationSelectionLabel = new javax.swing.JLabel();
        blCorrelationSelectionBox = new javax.swing.JComboBox();
        blCorrelationTypeLabel = new javax.swing.JLabel();
        blCorrelationTypeScrollPane = new javax.swing.JScrollPane();
        blCorrelationTypeList = new javax.swing.JList();
        BaselineSelectionPanel = new javax.swing.JPanel();
        baselineStationsScrollPane = new javax.swing.JScrollPane();
        baselineStationsTable = new javax.swing.JTable();
        baselineModsPanel = new javax.swing.JPanel();
        addBaseLineButton = new javax.swing.JButton();
        deleteBaseLineButton = new javax.swing.JButton();
        baselineUseAllCheckbox = new javax.swing.JCheckBox();
        stepExplorerGlobalSources = new javax.swing.JPanel();
        stepExplorerSourcesPanel1 = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel1 = new javax.swing.JPanel();
        addSourceButton1 = new javax.swing.JButton();
        modifySourceButton1 = new javax.swing.JButton();
        deleteSourceButton2 = new javax.swing.JButton();
        stepExplorerSourcesScrollPane1 = new javax.swing.JScrollPane();
        stepExplorerSourcesList1 = new javax.swing.JList();
        stepExplorerSourcesPanel = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel = new javax.swing.JPanel();
        addSourceButton = new javax.swing.JButton();
        modifySourceButton = new javax.swing.JButton();
        deleteSourceButton1 = new javax.swing.JButton();
        stepExplorerSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerSourcesList = new javax.swing.JList();
        stepExplorerStepNameLabel = new javax.swing.JLabel();
        stepExplorerStepNameText = new javax.swing.JTextField();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        BBSStepExplorerPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        explorePreviousStepButton.setText("View previous step");
        explorePreviousStepButton.setEnabled(false);
        stepExplorerPanel.add(explorePreviousStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 10, -1, -1));

        exploreParentStepButton.setText("View parent multistep");
        stepExplorerPanel.add(exploreParentStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(730, 70, -1, -1));

        exploreFirstChildStepButton.setText("View first child step");
        exploreFirstChildStepButton.setEnabled(false);
        stepExplorerPanel.add(exploreFirstChildStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(730, 40, -1, -1));

        exploreNextStepButton.setText("View next step");
        stepExplorerPanel.add(exploreNextStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 10, -1, -1));

        stepExplorerRevertButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Undo16.gif")));
        stepExplorerRevertButton.setText("Revert");
        stepExplorerRevertButton.setMaximumSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.setMinimumSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.setPreferredSize(new java.awt.Dimension(100, 25));
        stepExplorerRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerRevertButtonActionPerformed(evt);
            }
        });

        stepExplorerPanel.add(stepExplorerRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 620, 100, -1));

        stepExplorerInstrumentModelPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerInstrumentModelPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Model"));
        stepExplorerInstrumentModelScrollPane.setViewportView(stepExplorerInstrumentModelList);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModelModsPanel.setLayout(new java.awt.GridBagLayout());

        addInstrumentModelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addInstrumentModelButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addInstrumentModelButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addInstrumentModelButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(addInstrumentModelButton, gridBagConstraints);

        modifyInstrumentModelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Edit16.gif")));
        modifyInstrumentModelButton.setMaximumSize(new java.awt.Dimension(30, 25));
        modifyInstrumentModelButton.setMinimumSize(new java.awt.Dimension(30, 25));
        modifyInstrumentModelButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(modifyInstrumentModelButton, gridBagConstraints);

        deleteInstrumentModelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteInstrumentModelButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentModelButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteInstrumentModelButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(deleteInstrumentModelButton, gridBagConstraints);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelModsPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerPanel.add(stepExplorerInstrumentModelPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 230, 160, 120));

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

        stepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 360, 680, 250));

        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data Column"));
        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 30, 140, 20));

        stepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(530, 230, 160, 120));

        baselineGlobalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        baselineGlobalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline"));
        baseLineCorrelationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        baseLineCorrelationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        blCorrelationSelectionLabel.setText("Selection :");
        baseLineCorrelationPanel.add(blCorrelationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        blCorrelationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "AUTO", "CROSS", "ALL" }));
        blCorrelationSelectionBox.setToolTipText("Station correlations to use.\n\nAUTO: Use only correlations of each station with itself (i.e. no base lines).Not yet implemented.\nCROSS: Use only correlations between stations (i.e. base lines).\nALL: Use both AUTO and CROSS correlations.");
        baseLineCorrelationPanel.add(blCorrelationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 80, -1));

        blCorrelationTypeLabel.setText("Type :");
        baseLineCorrelationPanel.add(blCorrelationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 10, -1, -1));

        blCorrelationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        blCorrelationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY. \n\nAs an example, suppose you select 'XX' here and set Selection to AUTO, then the X polarization signal of each station is correlated with itself. However if we set Selection to CROSS, then the X polarization of station A is correlated with the X polarization of station B for each base line.");
        blCorrelationTypeScrollPane.setViewportView(blCorrelationTypeList);

        baseLineCorrelationPanel.add(blCorrelationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        baselineGlobalPanel.add(baseLineCorrelationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 180, 170, 120));

        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Selection"));
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

        baselineGlobalPanel.add(BaselineSelectionPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 300, 160));

        stepExplorerPanel.add(baselineGlobalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 40, 320, 310));

        stepExplorerGlobalSources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerGlobalSources.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
        stepExplorerSourcesPanel1.setLayout(new java.awt.BorderLayout());

        stepExplorerSourcesModsPanel1.setLayout(new java.awt.GridBagLayout());

        addSourceButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addSourceButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        addSourceButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        addSourceButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(addSourceButton1, gridBagConstraints);

        modifySourceButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Edit16.gif")));
        modifySourceButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        modifySourceButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        modifySourceButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(modifySourceButton1, gridBagConstraints);

        deleteSourceButton2.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteSourceButton2.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSourceButton2.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSourceButton2.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(deleteSourceButton2, gridBagConstraints);

        stepExplorerSourcesPanel1.add(stepExplorerSourcesModsPanel1, java.awt.BorderLayout.SOUTH);

        stepExplorerSourcesScrollPane1.setViewportView(stepExplorerSourcesList1);

        stepExplorerSourcesPanel1.add(stepExplorerSourcesScrollPane1, java.awt.BorderLayout.CENTER);

        stepExplorerGlobalSources.add(stepExplorerSourcesPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 150));

        stepExplorerSourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerSourcesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Extra"));
        stepExplorerSourcesModsPanel.setLayout(new java.awt.GridBagLayout());

        addSourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addSourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addSourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addSourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(addSourceButton, gridBagConstraints);

        modifySourceButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Edit16.gif")));
        modifySourceButton.setMaximumSize(new java.awt.Dimension(30, 25));
        modifySourceButton.setMinimumSize(new java.awt.Dimension(30, 25));
        modifySourceButton.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(modifySourceButton, gridBagConstraints);

        deleteSourceButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteSourceButton1.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSourceButton1.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSourceButton1.setPreferredSize(new java.awt.Dimension(30, 25));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(deleteSourceButton1, gridBagConstraints);

        stepExplorerSourcesPanel.add(stepExplorerSourcesModsPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerSourcesScrollPane.setViewportView(stepExplorerSourcesList);

        stepExplorerSourcesPanel.add(stepExplorerSourcesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerGlobalSources.add(stepExplorerSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 10, 160, 160));

        stepExplorerPanel.add(stepExplorerGlobalSources, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 40, 350, 180));

        stepExplorerStepNameLabel.setFont(new java.awt.Font("Dialog", 1, 18));
        stepExplorerStepNameLabel.setText("Step");
        stepExplorerPanel.add(stepExplorerStepNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 10, -1, 30));

        stepExplorerStepNameText.setEditable(false);
        stepExplorerStepNameText.setFont(new java.awt.Font("Dialog", 1, 18));
        stepExplorerStepNameText.setToolTipText("This is the name of the displayed step");
        stepExplorerPanel.add(stepExplorerStepNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(80, 10, 250, 30));

        stepExplorerScrollPanel.setViewportView(stepExplorerPanel);

        BBSStepExplorerPanel.add(stepExplorerScrollPanel, java.awt.BorderLayout.CENTER);

        add(BBSStepExplorerPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    
    private void stepExplorerRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepExplorerRevertButtonActionPerformed
// TODO add your handling code here:
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
        if(evt.getActionCommand() == "Save Settings") {
            
            //perform input validation on step name
            
            boolean warning = false;
            //modifying a step
            if(itsBBSStep != null){
                //update all fields in the BBS Step object
                saveInput(itsBBSStep);
                
                //persist the BBS Step
                
                //BBSStepDataManager.getInstance().updateStep(itsBBSStep);
                
            }else{
                
                //new BBS step to be added
                if(itsBBSStep == null && itsParentBBSStep != null){
                    if(!this.stepExplorerStepNameText.getText().equals("")){
                        itsBBSStep = new BBSStep(this.stepExplorerStepNameText.getText());
                        itsBBSStep.setParentStep(itsParentBBSStep);
                        itsParentBBSStep.addChildStep(itsBBSStep);
                    }
                }else if(itsBBSStep == null && itsParentBBSStep == null){
                    //adding new BBS Step to a strategy
                    if(!this.stepExplorerStepNameText.getText().equals("")){
                        itsBBSStep = new BBSStep(this.stepExplorerStepNameText.getText());
                        BBSStepDataManager.getInstance().getStrategy().addChildStep(itsBBSStep);
                    }
                }
                if(itsBBSStep != null){
                    //update all fields in the BBS Step object
                    saveInput(itsBBSStep);
                    //persist the BBS Step
                    
                    //BBSStepDataManager.getInstance().addStep(itsBBSStep);
                    
                }else{
                    //warning
                }
            }
            
            if(!warning){
                //tell parents that panel is ready to close...
                ActionEvent closeEvt = new ActionEvent(this,1,"ReadyToClose");
                fireActionListenerActionPerformed(closeEvt);
            }
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private BBSStep itsBBSStep = null;
    private BBSStep itsParentBBSStep = null;
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    
    /*modify to step explorer panels
    // Global Settings parameters
    private jOTDBnode dataSet;
     
    private jOTDBnode BBDBHost;
    private jOTDBnode BBDBPort;
    private jOTDBnode BBDBDBName;
    private jOTDBnode BBDBUsername;
    private jOTDBnode BBDBPassword;
     
    private jOTDBnode ParmDBInstrument;
    private jOTDBnode ParmDBLocalSky;
     */
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel BBSStepExplorerPanel;
    private javax.swing.JPanel BaselineSelectionPanel;
    private javax.swing.JButton addBaseLineButton;
    private javax.swing.JButton addInstrumentModelButton;
    private javax.swing.JButton addSolvableParmButton;
    private javax.swing.JButton addSolvableParmButton1;
    private javax.swing.JButton addSourceButton;
    private javax.swing.JButton addSourceButton1;
    private javax.swing.JPanel baseLineCorrelationPanel;
    private javax.swing.JPanel baselineGlobalPanel;
    private javax.swing.JPanel baselineModsPanel;
    private javax.swing.JScrollPane baselineStationsScrollPane;
    private javax.swing.JTable baselineStationsTable;
    private javax.swing.JCheckBox baselineUseAllCheckbox;
    private javax.swing.JComboBox blCorrelationSelectionBox;
    private javax.swing.JLabel blCorrelationSelectionLabel;
    private javax.swing.JLabel blCorrelationTypeLabel;
    private javax.swing.JList blCorrelationTypeList;
    private javax.swing.JScrollPane blCorrelationTypeScrollPane;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deleteBaseLineButton;
    private javax.swing.JButton deleteInstrumentModelButton;
    private javax.swing.JButton deleteSolvableParmButton;
    private javax.swing.JButton deleteSolvableParmButton1;
    private javax.swing.JButton deleteSourceButton1;
    private javax.swing.JButton deleteSourceButton2;
    private javax.swing.JButton exploreFirstChildStepButton;
    private javax.swing.JButton exploreNextStepButton;
    private javax.swing.JButton exploreParentStepButton;
    private javax.swing.JButton explorePreviousStepButton;
    private javax.swing.JButton modifyInstrumentModelButton;
    private javax.swing.JButton modifySolvableParmButton;
    private javax.swing.JButton modifySolvableParmButton1;
    private javax.swing.JButton modifySourceButton;
    private javax.swing.JButton modifySourceButton1;
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
    private javax.swing.JPanel stepExplorerGlobalSources;
    private javax.swing.JList stepExplorerInstrumentModelList;
    private javax.swing.JPanel stepExplorerInstrumentModelModsPanel;
    private javax.swing.JPanel stepExplorerInstrumentModelPanel;
    private javax.swing.JScrollPane stepExplorerInstrumentModelScrollPane;
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
    private javax.swing.JList stepExplorerSourcesList;
    private javax.swing.JList stepExplorerSourcesList1;
    private javax.swing.JPanel stepExplorerSourcesModsPanel;
    private javax.swing.JPanel stepExplorerSourcesModsPanel1;
    private javax.swing.JPanel stepExplorerSourcesModsPanel2;
    private javax.swing.JPanel stepExplorerSourcesPanel;
    private javax.swing.JPanel stepExplorerSourcesPanel1;
    private javax.swing.JScrollPane stepExplorerSourcesScrollPane;
    private javax.swing.JScrollPane stepExplorerSourcesScrollPane1;
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
