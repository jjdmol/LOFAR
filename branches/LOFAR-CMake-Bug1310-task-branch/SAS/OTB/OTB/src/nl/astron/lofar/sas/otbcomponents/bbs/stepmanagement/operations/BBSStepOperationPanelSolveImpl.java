/*
 *  BBSStepOperationPanelSolveImpl.java
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

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations;

import java.awt.Color;
import java.util.HashMap;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepExplorerPanel;

/**
 * Solve operation panel implementation of IBBSStepOperationPanel
 *
 * @author pompert
 * @version $Id$
 * @created 18-8-2006, 13:56
 * @updated
 */
public class BBSStepOperationPanelSolveImpl extends javax.swing.JPanel implements IBBSStepOperationPanel{
    
    private HashMap<String,String> itsOperationParameters = new HashMap<String,String>();
    private HashMap<String,String> itsInheritedOperationParameters = new HashMap<String,String>();
    
    
    /** 
     * Creates new form BBSStepOperationPanelSolveImpl 
     */
    public BBSStepOperationPanelSolveImpl() {
        initComponents();
        this.solvableParmsList.setModel(new DefaultListModel());
        this.excludedParmsList.setModel(new DefaultListModel());
    }
    
    public void setBBSStepContent(BBSStepData stepData,BBSStepData inheritedData){
        if(stepData != null && inheritedData != null){
            String operationName = stepData.getOperationName();
            if(operationName==null){
                operationName = inheritedData.getOperationName();
            }
            if(operationName != null && operationName.equals("Solve")){
                fillStepOperationPanel(stepData,inheritedData);
            }
        }
    }
    
    private void fillStepOperationPanel(BBSStepData stepData, BBSStepData inheritedData){
        
        if(stepData.getOperationAttributes()!=null){
            itsOperationParameters = stepData.getOperationAttributes();
        }
        if(inheritedData.getOperationAttributes()!=null){
            itsInheritedOperationParameters = inheritedData.getOperationAttributes();
        }
        
        
        //Max iterations
        this.maxIterationsText.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("MaxIter") != null){
            this.maxIterationsText.setText(itsOperationParameters.get("MaxIter"));
            maxIterationsText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("MaxIter") != null){
                this.maxIterationsText.setText(itsInheritedOperationParameters.get("MaxIter"));
                maxIterationsText.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.maxIterationsText.setText("");
                maxIterationsText.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //Epsilon
        this.epsilonText.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("Epsilon") != null){
            this.epsilonText.setText(itsOperationParameters.get("Epsilon"));
            epsilonText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("Epsilon") != null){
                this.epsilonText.setText(itsInheritedOperationParameters.get("Epsilon"));
                epsilonText.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.epsilonText.setText("");
                epsilonText.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //Min converged
        this.minConvergedText.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("MinConverged") != null){
            this.minConvergedText.setText(itsOperationParameters.get("MinConverged"));
            minConvergedText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("MinConverged") != null){
                this.minConvergedText.setText(itsInheritedOperationParameters.get("MinConverged"));
                minConvergedText.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.minConvergedText.setText("");
                minConvergedText.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //DomainSize
        ////Frequency
        this.dsFrequencyText.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("DomainSize.Freq") != null){
            this.dsFrequencyText.setText(itsOperationParameters.get("DomainSize.Freq"));
            dsFrequencyText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("DomainSize.Freq") != null){
                this.dsFrequencyText.setText(itsInheritedOperationParameters.get("DomainSize.Freq"));
                dsFrequencyText.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.dsFrequencyText.setText("");
                dsFrequencyText.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        ////Time
        this.dsTimeText.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("DomainSize.Time") != null){
            this.dsTimeText.setText(itsOperationParameters.get("DomainSize.Time"));
            dsTimeText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("DomainSize.Time") != null){
                this.dsTimeText.setText(itsInheritedOperationParameters.get("DomainSize.Time"));
                dsTimeText.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.dsTimeText.setText("");
                dsTimeText.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //Solvable Parms
        this.solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.DEFAULT);
        this.solvableParmsModifyText.setText("");
        this.fillList(this.solvableParmsList,"[]",true);
        if(itsOperationParameters.get("Parms") != null){
            String solvParms = itsOperationParameters.get("Parms");
            if(!solvParms.equals("[]")){
                this.fillList(this.solvableParmsList,solvParms,true);
                this.solveAllParmsCheckbox.setSelected(false);
            }else{
                this.solveAllParmsCheckbox.setSelected(true);
            }
            solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }else{
            if(itsInheritedOperationParameters.get("Parms") != null){
                String solvParms = itsInheritedOperationParameters.get("Parms");
                if(!solvParms.equals("[]")){
                    this.fillList(this.solvableParmsList,solvParms,true);
                    this.solveAllParmsCheckbox.setSelected(false);
                }else{
                    this.solveAllParmsCheckbox.setSelected(true);
                }
                solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            }else{
                this.solveAllParmsCheckbox.setSelected(false);
                solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //Excluded Parms
        
        this.excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.DEFAULT);
        this.excludedParmsModifyText.setText("");
        this.fillList(excludedParmsList,"[]",true);
        if(itsOperationParameters.get("ExclParms") != null){
            String excludedParms = itsOperationParameters.get("ExclParms");
            if(!excludedParms.equals("[]")){
                this.fillList(this.excludedParmsList,excludedParms,true);
                this.excludeParmsCheckbox.setSelected(true);
            }else{
                this.excludeParmsCheckbox.setSelected(false);
            }
            excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            
        }else{
            if(itsInheritedOperationParameters.get("ExclParms") != null){
                String excludedParms = itsInheritedOperationParameters.get("ExclParms");
                if(!excludedParms.equals("[]")){
                    this.fillList(this.excludedParmsList,excludedParms,true);
                    this.excludeParmsCheckbox.setSelected(true);
                }else{
                    this.excludeParmsCheckbox.setSelected(false);
                }
                excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            }else{
                this.excludeParmsCheckbox.setSelected(true);
                excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
    }
    
    public HashMap<String,String> getBBSStepOperationAttributes(){
        HashMap<String,String> returnMap = new HashMap<String,String>();
        
        //Max iterations
        if(this.maxIterationsText.getText().equals("")){
            returnMap.put("MaxIter",null);
        }else{
            returnMap.put("MaxIter",maxIterationsText.getText());
        }
        //Epsilon
        if(this.epsilonText.getText().equals("")){
            returnMap.put("Epsilon",null);
        }else{
            returnMap.put("Epsilon",epsilonText.getText());
            
        }
        //Min Converged
        if(this.minConvergedText.getText().equals("")){
            returnMap.put("MinConverged",null);
        }else{
            returnMap.put("MinConverged",minConvergedText.getText());
        }
        //DomainSize
        ////Frequency
        if(this.dsFrequencyText.getText().equals("")){
            returnMap.put("DomainSize.Freq",null);
        }else{
            returnMap.put("DomainSize.Freq",dsFrequencyText.getText());
        }
        ////Time
        if(this.dsTimeText.getText().equals("")){
            returnMap.put("DomainSize.Time",null);
        }else{
            returnMap.put("DomainSize.Time",dsTimeText.getText());
        }
        
        //Solvable Parms
        if(this.solveAllParmsCheckbox.isSelected()){
            returnMap.put("Parms","[]");
        }else{
            if(this.solvableParmsList.getModel().getSize() == 0){
                returnMap.put("Parms",null);
            }else{
                returnMap.put("Parms",this.createList(solvableParmsList,true));
            }
        }
        //Excluded Parms
        
        if(!this.excludeParmsCheckbox.isSelected()){
            returnMap.put("ExclParms","[]");
        }else{
            if(this.excludedParmsList.getModel().getSize() == 0){
                returnMap.put("ExclParms",null);
            }else{
                returnMap.put("ExclParms",this.createList(excludedParmsList,true));
            }
        }
        
        return returnMap;
    }
    
    public boolean validateInput(){
        boolean warning = true;
        String dsintegrationTime = this.dsTimeText.getText();
        String dsintegrationFrequency = this.dsFrequencyText.getText();
        String converged = this.minConvergedText.getText();
        String epsilon = this.epsilonText.getText();
        String iterations = this.maxIterationsText.getText();
        if(!dsintegrationTime.equals("")){
            try {
                Double itime = Double.parseDouble(dsintegrationTime);
                this.dsTimeText.setBackground(Color.WHITE);
                dsTimeText.setToolTipText("Time in seconds");
            } catch (NumberFormatException ex) {
                warning=false;
                dsTimeText.setBackground(Color.RED);
                dsTimeText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(!dsintegrationFrequency.equals("")){
            try {
                Double ifreq = Double.parseDouble(dsintegrationFrequency);
                this.dsFrequencyText.setBackground(Color.WHITE);
                dsFrequencyText.setToolTipText("Frequency in Hertz (Hz)");
            } catch (NumberFormatException ex) {
                warning=false;
                dsFrequencyText.setBackground(Color.RED);
                dsFrequencyText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(!converged.equals("")){
            try {
                Double iconverged = Double.parseDouble(converged);
                this.minConvergedText.setBackground(Color.WHITE);
                minConvergedText.setToolTipText("Fraction that must have converged");
            } catch (NumberFormatException ex) {
                warning=false;
                minConvergedText.setBackground(Color.RED);
                minConvergedText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(!epsilon.equals("")){
            try {
                Double iepsilon = Double.parseDouble(epsilon);
                this.epsilonText.setBackground(Color.WHITE);
                epsilonText.setToolTipText("Convergence Threshold");
            } catch (NumberFormatException ex) {
                warning=false;
                epsilonText.setBackground(Color.RED);
                epsilonText.setToolTipText("This field has to be a valid double (eg. 1.5)");
            }
        }
        if(!iterations.equals("")){
            try {
                Integer iiterations = Integer.parseInt(iterations);
                this.maxIterationsText.setBackground(Color.WHITE);
                maxIterationsText.setToolTipText("Maximum number of iterations");
            } catch (NumberFormatException ex) {
                warning=false;
                maxIterationsText.setBackground(Color.RED);
                maxIterationsText.setToolTipText("This field has to be a valid integer (eg. 2)");
            }
        }
        return warning;
    }
    
    private String createList(JList aListComponent,boolean createQuotes) {
        String aList="[";
        if (aListComponent.getModel().getSize() > 0) {
            if(createQuotes){
                aList += "\"";
            }
            aList += (String)aListComponent.getModel().getElementAt(0);
            if(createQuotes){
                aList += "\"";
            }
            for (int i=1; i < aListComponent.getModel().getSize();i++) {
                aList+= ",";
                if(createQuotes){
                    aList += "\"";
                }
                aList += aListComponent.getModel().getElementAt(i);
                if(createQuotes){
                    aList += "\"";
                }
            }
            
        }
        aList+="]";
        return aList;
    }
    private void fillList(JList aListComponent,String theList,boolean removeQuotes) {
        DefaultListModel itsModel = new DefaultListModel();
        aListComponent.setModel(itsModel);
        String aList = theList;
        if (aList.startsWith("[")) {
            aList = aList.substring(1,aList.length());
        }
        if (aList.endsWith("]")) {
            aList = aList.substring(0,aList.length()-1);
        }
        if(!aList.equals("")){
            String[] aS=aList.split(",");
            for (int i=0; i< aS.length;i++) {
                if(removeQuotes){
                    itsModel.addElement(aS[i].substring(1,aS[i].length()-1));
                }else{
                    itsModel.addElement(aS[i]);
                }
            }
            aListComponent.setModel(itsModel);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        seOperationAttributesInputPanel = new javax.swing.JPanel();
        maxIterationsLabel = new javax.swing.JLabel();
        maxIterationsText = new javax.swing.JTextField();
        epsilonText = new javax.swing.JTextField();
        epsilonLabel = new javax.swing.JLabel();
        minConvergedText = new javax.swing.JTextField();
        minConvergedLabel = new javax.swing.JLabel();
        domainSizeGroupPanel = new javax.swing.JPanel();
        dsFrequencyLabel = new javax.swing.JLabel();
        dsTimeLabel = new javax.swing.JLabel();
        dsFrequencyText = new javax.swing.JTextField();
        dsTimeText = new javax.swing.JTextField();
        dsFrequencyUnitLabel = new javax.swing.JLabel();
        dsTimeUnitLabel = new javax.swing.JLabel();
        solvableParmsGroupPanel = new javax.swing.JPanel();
        solvableParmsPanel = new javax.swing.JPanel();
        solvableParmsScrollPane = new javax.swing.JScrollPane();
        solvableParmsList = new javax.swing.JList();
        solvableParmsEditPanel = new javax.swing.JPanel();
        solvableParmsModifyText = new javax.swing.JTextField();
        solvableParmsButtonPanel = new javax.swing.JPanel();
        deleteSolvableParmButton = new javax.swing.JButton();
        addSolvableParmButton = new javax.swing.JButton();
        solveAllParmsCheckbox = new javax.swing.JCheckBox();
        excludedParmsGroupPanel = new javax.swing.JPanel();
        excludedParmsPanel = new javax.swing.JPanel();
        excludedParmsScrollPane = new javax.swing.JScrollPane();
        excludedParmsList = new javax.swing.JList();
        excludedParmsEditPanel = new javax.swing.JPanel();
        excludedParmsModifyText = new javax.swing.JTextField();
        excludedParmsButtonPanel = new javax.swing.JPanel();
        deleteExcludedParmButton = new javax.swing.JButton();
        addExcludedParmButton = new javax.swing.JButton();
        excludeParmsCheckbox = new javax.swing.JCheckBox();

        setMaximumSize(new java.awt.Dimension(540, 180));
        setMinimumSize(new java.awt.Dimension(540, 180));
        setPreferredSize(new java.awt.Dimension(540, 180));
        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributesInputPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        maxIterationsLabel.setText("Max. iterations :");
        seOperationAttributesInputPanel.add(maxIterationsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        maxIterationsText.setText("10");
        maxIterationsText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                maxIterationsTextKeyReleased(evt);
            }
        });
        seOperationAttributesInputPanel.add(maxIterationsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 60, -1));

        epsilonText.setText("1e-6");
        epsilonText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                epsilonTextKeyReleased(evt);
            }
        });
        seOperationAttributesInputPanel.add(epsilonText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, -1));

        epsilonLabel.setText("Epsilon :");
        seOperationAttributesInputPanel.add(epsilonLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));

        minConvergedText.setText("100.0");
        minConvergedText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                minConvergedTextKeyReleased(evt);
            }
        });
        seOperationAttributesInputPanel.add(minConvergedText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 60, -1));

        minConvergedLabel.setText("Min. converged :");
        seOperationAttributesInputPanel.add(minConvergedLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, -1));

        domainSizeGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Domain Size"));
        domainSizeGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        dsFrequencyLabel.setText("Frequency :");
        domainSizeGroupPanel.add(dsFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        dsTimeLabel.setText("Time :");
        domainSizeGroupPanel.add(dsTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 40, -1, -1));

        dsFrequencyText.setText("1e25");
        dsFrequencyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                dsFrequencyTextKeyReleased(evt);
            }
        });
        domainSizeGroupPanel.add(dsFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 20, 60, -1));

        dsTimeText.setText("1e25");
        dsTimeText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                dsTimeTextKeyReleased(evt);
            }
        });
        domainSizeGroupPanel.add(dsTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 40, 60, -1));

        dsFrequencyUnitLabel.setText("Hz");
        domainSizeGroupPanel.add(dsFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, -1, 20));

        dsTimeUnitLabel.setText("s");
        domainSizeGroupPanel.add(dsTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 40, 20, 20));

        seOperationAttributesInputPanel.add(domainSizeGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 180, 70));

        solvableParmsGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Solvable Parameters"));
        solvableParmsGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        solvableParmsPanel.setLayout(new java.awt.BorderLayout());

        solvableParmsList.setBackground(java.awt.Color.lightGray);
        solvableParmsList.setEnabled(false);
        solvableParmsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                solvableParmsListValueChanged(evt);
            }
        });
        solvableParmsScrollPane.setViewportView(solvableParmsList);

        solvableParmsPanel.add(solvableParmsScrollPane, java.awt.BorderLayout.CENTER);

        solvableParmsEditPanel.setLayout(new java.awt.BorderLayout());

        solvableParmsModifyText.setEnabled(false);
        solvableParmsModifyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                solvableParmsModifyTextKeyReleased(evt);
            }
        });
        solvableParmsEditPanel.add(solvableParmsModifyText, java.awt.BorderLayout.CENTER);

        solvableParmsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteSolvableParmButton.setText("Delete");
        deleteSolvableParmButton.setToolTipText("Remove the selected Station from the list");
        deleteSolvableParmButton.setEnabled(false);
        deleteSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteSolvableParmButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteSolvableParmButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        solvableParmsButtonPanel.add(deleteSolvableParmButton, gridBagConstraints);

        addSolvableParmButton.setText("Add");
        addSolvableParmButton.setToolTipText("Add the station entered to the list");
        addSolvableParmButton.setEnabled(false);
        addSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setPreferredSize(new java.awt.Dimension(55, 23));
        addSolvableParmButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addSolvableParmButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        solvableParmsButtonPanel.add(addSolvableParmButton, gridBagConstraints);

        solvableParmsEditPanel.add(solvableParmsButtonPanel, java.awt.BorderLayout.SOUTH);

        solvableParmsPanel.add(solvableParmsEditPanel, java.awt.BorderLayout.SOUTH);

        solveAllParmsCheckbox.setSelected(true);
        solveAllParmsCheckbox.setText("Solve all parms");
        solveAllParmsCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        solveAllParmsCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        solveAllParmsCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                solveAllParmsCheckboxItemStateChanged(evt);
            }
        });
        solvableParmsPanel.add(solveAllParmsCheckbox, java.awt.BorderLayout.NORTH);

        solvableParmsGroupPanel.add(solvableParmsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 140, 140));

        seOperationAttributesInputPanel.add(solvableParmsGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, 160, 170));

        add(seOperationAttributesInputPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, -1, -1));

        excludedParmsGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Excluded Parameters"));
        excludedParmsGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        excludedParmsPanel.setLayout(new java.awt.BorderLayout());

        excludedParmsList.setBackground(java.awt.Color.lightGray);
        excludedParmsList.setEnabled(false);
        excludedParmsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                excludedParmsListValueChanged(evt);
            }
        });
        excludedParmsScrollPane.setViewportView(excludedParmsList);

        excludedParmsPanel.add(excludedParmsScrollPane, java.awt.BorderLayout.CENTER);

        excludedParmsEditPanel.setLayout(new java.awt.BorderLayout());

        excludedParmsModifyText.setEnabled(false);
        excludedParmsModifyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                excludedParmsModifyTextKeyReleased(evt);
            }
        });
        excludedParmsEditPanel.add(excludedParmsModifyText, java.awt.BorderLayout.CENTER);

        excludedParmsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteExcludedParmButton.setText("Delete");
        deleteExcludedParmButton.setToolTipText("Remove the selected Station from the list");
        deleteExcludedParmButton.setEnabled(false);
        deleteExcludedParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteExcludedParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteExcludedParmButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteExcludedParmButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteExcludedParmButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        excludedParmsButtonPanel.add(deleteExcludedParmButton, gridBagConstraints);

        addExcludedParmButton.setText("Add");
        addExcludedParmButton.setToolTipText("Add the station entered to the list");
        addExcludedParmButton.setEnabled(false);
        addExcludedParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addExcludedParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addExcludedParmButton.setPreferredSize(new java.awt.Dimension(55, 23));
        addExcludedParmButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addExcludedParmButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        excludedParmsButtonPanel.add(addExcludedParmButton, gridBagConstraints);

        excludedParmsEditPanel.add(excludedParmsButtonPanel, java.awt.BorderLayout.SOUTH);

        excludedParmsPanel.add(excludedParmsEditPanel, java.awt.BorderLayout.SOUTH);

        excludeParmsCheckbox.setText("Exclude parms :");
        excludeParmsCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        excludeParmsCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        excludeParmsCheckbox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                excludeParmsCheckboxItemStateChanged(evt);
            }
        });
        excludedParmsPanel.add(excludeParmsCheckbox, java.awt.BorderLayout.NORTH);

        excludedParmsGroupPanel.add(excludedParmsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 140, 140));

        add(excludedParmsGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 10, 160, 170));
    }// </editor-fold>//GEN-END:initComponents
    
    private void excludeParmsCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_excludeParmsCheckboxItemStateChanged
        
        if(this.excludeParmsCheckbox.isSelected()){
            this.excludedParmsList.setEnabled(true);
            this.excludedParmsList.setBackground(Color.WHITE);
            this.excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            this.excludedParmsList.clearSelection();
            this.excludedParmsModifyText.setText("");
            this.excludedParmsModifyText.setEnabled(true);
        }else{
            this.excludedParmsList.setEnabled(false);
            this.excludedParmsList.setBackground(Color.LIGHT_GRAY);
            this.excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            this.excludedParmsList.clearSelection();
            this.excludedParmsModifyText.setEnabled(false);
            this.excludedParmsModifyText.setText("");
        }
    }//GEN-LAST:event_excludeParmsCheckboxItemStateChanged
    
    private void solveAllParmsCheckboxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_solveAllParmsCheckboxItemStateChanged
        if(this.solveAllParmsCheckbox.isSelected()){
            this.solvableParmsList.setEnabled(false);
            this.solvableParmsList.setBackground(Color.LIGHT_GRAY);
            this.solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            this.solvableParmsList.clearSelection();
            this.deleteSolvableParmButton.setEnabled(false);
            this.addSolvableParmButton.setEnabled(false);
            this.solvableParmsModifyText.setEnabled(false);
        }else{
            this.solvableParmsList.setEnabled(true);
            this.solvableParmsList.setBackground(Color.WHITE);
            this.solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
            this.solvableParmsList.clearSelection();
            this.solvableParmsModifyText.setEnabled(true);
            this.solvableParmsModifyText.setText("");
        }
    }//GEN-LAST:event_solveAllParmsCheckboxItemStateChanged
    
    private void addExcludedParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addExcludedParmButtonActionPerformed
        String toBeAddedNSource = this.excludedParmsModifyText.getText();
        if(!toBeAddedNSource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)this.excludedParmsList.getModel();
            theStationModel.addElement(toBeAddedNSource);
            excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addExcludedParmButtonActionPerformed
    
    private void deleteExcludedParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteExcludedParmButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)excludedParmsList.getModel();
        int[] selectedIndices = excludedParmsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = excludedParmsList.getSelectedIndices();
            excludedParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
        if(theSourceModel.size()==0){
            this.deleteExcludedParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteExcludedParmButtonActionPerformed
    
    private void excludedParmsModifyTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_excludedParmsModifyTextKeyReleased
        String toBeAddedNSource = excludedParmsModifyText.getText();
        if(!toBeAddedNSource.equals("")){
            this.addExcludedParmButton.setEnabled(true);
        }else{
            this.addExcludedParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_excludedParmsModifyTextKeyReleased
    
    private void excludedParmsListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_excludedParmsListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteExcludedParmButton.setEnabled(true);
        }else{
            this.deleteExcludedParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_excludedParmsListValueChanged
    
    private void addSolvableParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addSolvableParmButtonActionPerformed
        String toBeAddedNSource = this.solvableParmsModifyText.getText();
        if(!toBeAddedNSource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)this.solvableParmsList.getModel();
            theStationModel.addElement(toBeAddedNSource);
            this.solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addSolvableParmButtonActionPerformed
    
    private void deleteSolvableParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteSolvableParmButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)solvableParmsList.getModel();
        int[] selectedIndices = solvableParmsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = solvableParmsList.getSelectedIndices();
            this.solvableParmsGroupPanel.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
        if(theSourceModel.size()==0){
            this.deleteSolvableParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteSolvableParmButtonActionPerformed
    
    private void solvableParmsModifyTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_solvableParmsModifyTextKeyReleased
        String toBeAddedNSource = solvableParmsModifyText.getText();
        if(!toBeAddedNSource.equals("")){
            this.addSolvableParmButton.setEnabled(true);
        }else{
            this.addSolvableParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_solvableParmsModifyTextKeyReleased
    
    private void solvableParmsListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_solvableParmsListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteSolvableParmButton.setEnabled(true);
        }else{
            this.deleteSolvableParmButton.setEnabled(false);
        }
    }//GEN-LAST:event_solvableParmsListValueChanged
    
    private void dsTimeTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_dsTimeTextKeyReleased
        dsTimeText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_dsTimeTextKeyReleased
    
    private void dsFrequencyTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_dsFrequencyTextKeyReleased
        dsFrequencyText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_dsFrequencyTextKeyReleased
    
    private void minConvergedTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_minConvergedTextKeyReleased
        minConvergedText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_minConvergedTextKeyReleased
    
    private void epsilonTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_epsilonTextKeyReleased
        epsilonText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_epsilonTextKeyReleased
    
    private void maxIterationsTextKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_maxIterationsTextKeyReleased
        maxIterationsText.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
    }//GEN-LAST:event_maxIterationsTextKeyReleased
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton addExcludedParmButton;
    private javax.swing.JButton addSolvableParmButton;
    private javax.swing.JButton deleteExcludedParmButton;
    private javax.swing.JButton deleteSolvableParmButton;
    private javax.swing.JPanel domainSizeGroupPanel;
    private javax.swing.JLabel dsFrequencyLabel;
    private javax.swing.JTextField dsFrequencyText;
    private javax.swing.JLabel dsFrequencyUnitLabel;
    private javax.swing.JLabel dsTimeLabel;
    private javax.swing.JTextField dsTimeText;
    private javax.swing.JLabel dsTimeUnitLabel;
    private javax.swing.JLabel epsilonLabel;
    private javax.swing.JTextField epsilonText;
    private javax.swing.JCheckBox excludeParmsCheckbox;
    private javax.swing.JPanel excludedParmsButtonPanel;
    private javax.swing.JPanel excludedParmsEditPanel;
    private javax.swing.JPanel excludedParmsGroupPanel;
    private javax.swing.JList excludedParmsList;
    private javax.swing.JTextField excludedParmsModifyText;
    private javax.swing.JPanel excludedParmsPanel;
    private javax.swing.JScrollPane excludedParmsScrollPane;
    private javax.swing.JLabel maxIterationsLabel;
    private javax.swing.JTextField maxIterationsText;
    private javax.swing.JLabel minConvergedLabel;
    private javax.swing.JTextField minConvergedText;
    private javax.swing.JPanel seOperationAttributesInputPanel;
    private javax.swing.JPanel solvableParmsButtonPanel;
    private javax.swing.JPanel solvableParmsEditPanel;
    private javax.swing.JPanel solvableParmsGroupPanel;
    private javax.swing.JList solvableParmsList;
    private javax.swing.JTextField solvableParmsModifyText;
    private javax.swing.JPanel solvableParmsPanel;
    private javax.swing.JScrollPane solvableParmsScrollPane;
    private javax.swing.JCheckBox solveAllParmsCheckbox;
    // End of variables declaration//GEN-END:variables
    
}
