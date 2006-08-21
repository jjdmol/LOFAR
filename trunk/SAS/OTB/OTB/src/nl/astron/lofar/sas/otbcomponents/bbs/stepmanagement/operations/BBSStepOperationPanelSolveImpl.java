/*
 * BBSStepOperationPanelSolveImpl.java
 *
 * Created on August 18, 2006, 12:25 PM
 */

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.operations;

import java.awt.Color;
import java.util.HashMap;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import nl.astron.lofar.sas.otbcomponents.bbs.BBSStepExplorerPanel;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepData;

/**
 *
 * @author  pompert
 */
public class BBSStepOperationPanelSolveImpl extends javax.swing.JPanel implements IBBSStepOperationPanel{
    
    private HashMap<String,String> itsOperationParameters = new HashMap<String,String>();
    private HashMap<String,String> itsInheritedOperationParameters = new HashMap<String,String>();
    
    
    /** Creates new form BBSStepOperationPanelSolveImpl */
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
        this.solvableParmsList.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("Parms") != null){
            this.fillList(solvableParmsList,itsOperationParameters.get("Parms"),true);
            solvableParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }else{
            if(itsInheritedOperationParameters.get("Parms") != null){
                this.fillList(solvableParmsList,itsInheritedOperationParameters.get("Parms"),true);
                solvableParmsList.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.fillList(solvableParmsList,"[]",true);
                solvableParmsList.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
            }
        }
        //Excluded Parms
        
        this.excludedParmsList.setBackground(BBSStepExplorerPanel.DEFAULT);
        if(itsOperationParameters.get("ExclParms") != null){
            this.fillList(excludedParmsList,itsOperationParameters.get("ExclParms"),true);
            excludedParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }else{
            if(itsInheritedOperationParameters.get("ExclParms") != null){
                this.fillList(excludedParmsList,itsInheritedOperationParameters.get("ExclParms"),true);
                excludedParmsList.setBackground(BBSStepExplorerPanel.INHERITED_FROM_PARENT);
            } else{
                this.fillList(excludedParmsList,"[]",true);
                excludedParmsList.setBackground(BBSStepExplorerPanel.NOT_DEFINED);
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
        
        if(this.solvableParmsList.getModel().getSize() == 0){
            returnMap.put("Parms",null);
        }else{
            returnMap.put("Parms",this.createList(solvableParmsList,true));
        }
        
        //Excluded Parms
        if(this.excludedParmsList.getModel().getSize() == 0){
            returnMap.put("ExclParms",null);
        }else{
            returnMap.put("ExclParms",this.createList(excludedParmsList,true));
        }
        
        
        return returnMap;
    }
    
    public boolean validateInput(){
        boolean warning = true;
        String dsintegrationTime = this.dsTimeText.getText();
        String dsintegrationFrequency = this.dsFrequencyText.getText();
        if(!dsintegrationTime.equals("")){
            try {
                Double itime = Double.parseDouble(dsintegrationTime);
                this.dsTimeText.setBackground(Color.WHITE);
            } catch (NumberFormatException ex) {
                warning=false;
                dsTimeText.setBackground(Color.RED);
            }
        }
        if(!dsintegrationFrequency.equals("")){
            try {
                Double ifreq = Double.parseDouble(dsintegrationFrequency);
                this.dsFrequencyText.setBackground(Color.WHITE);
            } catch (NumberFormatException ex) {
                warning=false;
                dsFrequencyText.setBackground(Color.RED);
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
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
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
        parametersGroupPanel = new javax.swing.JPanel();
        solvableParmsGroupPanel = new javax.swing.JPanel();
        solvableParmsPanel = new javax.swing.JPanel();
        solvableParmsScrollPane = new javax.swing.JScrollPane();
        solvableParmsList = new javax.swing.JList();
        solvableParmsEditPanel = new javax.swing.JPanel();
        solvableParmsModifyText = new javax.swing.JTextField();
        solvableParmsButtonPanel = new javax.swing.JPanel();
        deleteSolvableParmButton = new javax.swing.JButton();
        addSolvableParmButton = new javax.swing.JButton();
        excludedParmsGroupPanel = new javax.swing.JPanel();
        excludedParmsPanel = new javax.swing.JPanel();
        excludedParmsScrollPane = new javax.swing.JScrollPane();
        excludedParmsList = new javax.swing.JList();
        excludedParmsEditPanel = new javax.swing.JPanel();
        excludedParmsModifyText = new javax.swing.JTextField();
        excludedParmsButtonPanel = new javax.swing.JPanel();
        deleteExcludedParmButton = new javax.swing.JButton();
        addExcludedParmButton = new javax.swing.JButton();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setMaximumSize(new java.awt.Dimension(540, 180));
        setMinimumSize(new java.awt.Dimension(540, 180));
        setPreferredSize(new java.awt.Dimension(540, 180));
        seOperationAttributesInputPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        maxIterationsLabel.setText("Max. iterations :");
        seOperationAttributesInputPanel.add(maxIterationsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        maxIterationsText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                maxIterationsTextKeyReleased(evt);
            }
        });

        seOperationAttributesInputPanel.add(maxIterationsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 60, -1));

        epsilonText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                epsilonTextKeyReleased(evt);
            }
        });

        seOperationAttributesInputPanel.add(epsilonText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, -1));

        epsilonLabel.setText("Epsilon :");
        seOperationAttributesInputPanel.add(epsilonLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));

        minConvergedText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                minConvergedTextKeyReleased(evt);
            }
        });

        seOperationAttributesInputPanel.add(minConvergedText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 60, -1));

        minConvergedLabel.setText("Min. converged :");
        seOperationAttributesInputPanel.add(minConvergedLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, -1));

        domainSizeGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        domainSizeGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Domain Size"));
        dsFrequencyLabel.setText("Frequency :");
        domainSizeGroupPanel.add(dsFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        dsTimeLabel.setText("Time :");
        domainSizeGroupPanel.add(dsTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 40, -1, -1));

        dsFrequencyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                dsFrequencyTextKeyReleased(evt);
            }
        });

        domainSizeGroupPanel.add(dsFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 20, 60, -1));

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

        parametersGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        parametersGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Parameters"));
        solvableParmsGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        solvableParmsGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Solvable"));
        solvableParmsPanel.setLayout(new java.awt.BorderLayout());

        solvableParmsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                solvableParmsListValueChanged(evt);
            }
        });

        solvableParmsScrollPane.setViewportView(solvableParmsList);

        solvableParmsPanel.add(solvableParmsScrollPane, java.awt.BorderLayout.CENTER);

        solvableParmsEditPanel.setLayout(new java.awt.BorderLayout());

        solvableParmsModifyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                solvableParmsModifyTextKeyReleased(evt);
            }
        });

        solvableParmsEditPanel.add(solvableParmsModifyText, java.awt.BorderLayout.CENTER);

        solvableParmsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteSolvableParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteSolvableParmButton.setToolTipText("Remove the selected Station from the list");
        deleteSolvableParmButton.setEnabled(false);
        deleteSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteSolvableParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        addSolvableParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addSolvableParmButton.setToolTipText("Add the station entered to the list");
        addSolvableParmButton.setEnabled(false);
        addSolvableParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addSolvableParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        solvableParmsGroupPanel.add(solvableParmsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 140, 110));

        parametersGroupPanel.add(solvableParmsGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 140));

        excludedParmsGroupPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        excludedParmsGroupPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Excluded"));
        excludedParmsPanel.setLayout(new java.awt.BorderLayout());

        excludedParmsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                excludedParmsListValueChanged(evt);
            }
        });

        excludedParmsScrollPane.setViewportView(excludedParmsList);

        excludedParmsPanel.add(excludedParmsScrollPane, java.awt.BorderLayout.CENTER);

        excludedParmsEditPanel.setLayout(new java.awt.BorderLayout());

        excludedParmsModifyText.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                excludedParmsModifyTextKeyReleased(evt);
            }
        });

        excludedParmsEditPanel.add(excludedParmsModifyText, java.awt.BorderLayout.CENTER);

        excludedParmsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteExcludedParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteExcludedParmButton.setToolTipText("Remove the selected Station from the list");
        deleteExcludedParmButton.setEnabled(false);
        deleteExcludedParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteExcludedParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteExcludedParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        addExcludedParmButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addExcludedParmButton.setToolTipText("Add the station entered to the list");
        addExcludedParmButton.setEnabled(false);
        addExcludedParmButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addExcludedParmButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addExcludedParmButton.setPreferredSize(new java.awt.Dimension(30, 25));
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

        excludedParmsGroupPanel.add(excludedParmsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 140, 110));

        parametersGroupPanel.add(excludedParmsGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 20, 160, 140));

        seOperationAttributesInputPanel.add(parametersGroupPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, 350, 170));

        add(seOperationAttributesInputPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, -1, -1));

    }// </editor-fold>//GEN-END:initComponents

    private void addExcludedParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addExcludedParmButtonActionPerformed
        String toBeAddedNSource = this.excludedParmsModifyText.getText();
        if(!toBeAddedNSource.equals("")){
            DefaultListModel theStationModel = (DefaultListModel)this.excludedParmsList.getModel();
            theStationModel.addElement(toBeAddedNSource);
            excludedParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addExcludedParmButtonActionPerformed

    private void deleteExcludedParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteExcludedParmButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)excludedParmsList.getModel();
        int[] selectedIndices = excludedParmsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = excludedParmsList.getSelectedIndices();
            excludedParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
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
            solvableParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
        }
    }//GEN-LAST:event_addSolvableParmButtonActionPerformed

    private void deleteSolvableParmButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteSolvableParmButtonActionPerformed
        DefaultListModel theSourceModel = (DefaultListModel)solvableParmsList.getModel();
        int[] selectedIndices = solvableParmsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theSourceModel.remove(selectedIndices[0]);
            selectedIndices = solvableParmsList.getSelectedIndices();
            solvableParmsList.setBackground(BBSStepExplorerPanel.NOT_INHERITED_FROM_PARENT);
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
    private javax.swing.JPanel parametersGroupPanel;
    private javax.swing.JPanel seOperationAttributesInputPanel;
    private javax.swing.JPanel solvableParmsButtonPanel;
    private javax.swing.JPanel solvableParmsEditPanel;
    private javax.swing.JPanel solvableParmsGroupPanel;
    private javax.swing.JList solvableParmsList;
    private javax.swing.JTextField solvableParmsModifyText;
    private javax.swing.JPanel solvableParmsPanel;
    private javax.swing.JScrollPane solvableParmsScrollPane;
    // End of variables declaration//GEN-END:variables
    
}
