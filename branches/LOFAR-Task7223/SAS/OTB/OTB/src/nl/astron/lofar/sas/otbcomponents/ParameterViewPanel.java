/*
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

package nl.astron.lofar.sas.otbcomponents;

import java.awt.CardLayout;
import java.awt.Component;
import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.lofarutils.inputfieldbuilder.inputFieldBuilder;
import nl.astron.lofar.lofarutils.validation.WantsValidationStatus;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.util.AccessRights;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * @created 26-01-2006, 15:47
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class ParameterViewPanel extends javax.swing.JPanel implements IViewPanel,WantsValidationStatus {

    static Logger logger = Logger.getLogger(ParameterViewPanel.class);
    static String name = "Parameter";

    /** Creates new form based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */
    public ParameterViewPanel(MainFrame aMainFrame,jOTDBparam aParam) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParam = aParam;
        itsOtdbRmi=SharedVars.getOTDBrmi();
        initComboLists();
        initPanel();
    }

    /** Creates new form based upon aNode
     *
     * @params  aNode   Node to obtain the info from
     *
     */
    public ParameterViewPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        getParam(itsNode);
        itsOtdbRmi=SharedVars.getOTDBrmi();
        initComboLists();
        initPanel();
    }

    /** Creates new form BeanForm */
    public ParameterViewPanel() {
        initComponents();
    }

    /** Sets the Mainframe for this class
     *
     * @params  aMainFrame  Mainframe we need to link to
     */
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=SharedVars.getOTDBrmi();
            initComboLists();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }

    /** Returns the shortname of this class */
    public String getShortName() {
        return name;
    }
    public boolean isSingleton() {
        return false;
    }

    public JPanel getInstance() {
        return new ParameterViewPanel();
    }
    /** Sets the content for this class
     *
     * @params anObject  The class that contains the content.
     */
    public void setContent(Object anObject) {
        if (anObject != null) {
            itsNode = (jOTDBnode)anObject;
            try {
                // refresh the node (it could have been altered meanwhile)
                // in the case of nodes that have multiple instances it is possible that a save has
                // thrown away the old instances and created new ones.
                // in that case a user might have clicked on an "old" instance that now is away in the database.
                // then getNode will return and empty Node with ID's seet to 0
                itsNode = OtdbRmi.getRemoteMaintenance().getNode(itsNode.treeID(), itsNode.nodeID());
                if (itsNode.nodeID()== 0) {
             // create an Info popup.
                    JOptionPane.showMessageDialog(this,"This node has been deleted, please refresh tree!!","Warning",JOptionPane.WARNING_MESSAGE);
                    itsNode=null;
                }
            } catch (RemoteException ex) {
                String aS="Error during getNode: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsParam=null;
                return;
            }
            getParam(itsNode);
            initPanel();
        } else {
            logger.error("No node supplied");
        }
    }

    /** has this panel a popupmenu?
     *
     *@returns  false/true depending on the availability of a popupmenu
     *
     */
    public boolean hasPopupMenu() {
        return true;
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

        ///// TEST ONLY /////
        aPopupMenu= new JPopupMenu();
        aMenuItem=new JMenuItem("Param Choice 1");
        aMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                popupMenuHandler(evt);
            }
        });
        aMenuItem.setActionCommand("Choice 1");
        aPopupMenu.add(aMenuItem);

        aMenuItem=new JMenuItem("Param Choice 2");
        aMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                popupMenuHandler(evt);
            }
        });
        aMenuItem.setActionCommand("Choice 2");
        aPopupMenu.add(aMenuItem);

        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y );
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
        /// TEST ONLY ///
        switch (evt.getActionCommand()) {
            case "Choice 1":
                logger.debug("Param Choice 1 chosen");
                break;
            case "Choice 2":
                logger.debug("Param Choice 2 chosen");
                break;
        }
    }


    private void getParam(jOTDBnode aNode) {
        itsParam=null;
        if (aNode == null) {
            logger.error("Empty Node supplied for getParam");
            return;
        }
        itsNode=aNode;
        try {
            if (itsNode.leaf) {
                itsParam = OtdbRmi.getRemoteMaintenance().getParam(itsNode);
            } else {
                itsParam = OtdbRmi.getRemoteMaintenance().getParam(itsNode.treeID(),itsNode.paramDefID());
            }
        } catch (RemoteException ex) {
            String aS="Error during getParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            itsParam=null;
            return;
        }
    }

    private void initComboLists() {
        DefaultComboBoxModel aTypeModel = new DefaultComboBoxModel();
        TreeMap aTypeMap = OtdbRmi.getParamType();
        Iterator typeIt = aTypeMap.keySet().iterator();
        while (typeIt.hasNext()) {
            aTypeModel.addElement((String)aTypeMap.get(typeIt.next()));
        }
        ParamTypeText.setModel(aTypeModel);

        DefaultComboBoxModel aUnitModel = new DefaultComboBoxModel();
        TreeMap aUnitMap=OtdbRmi.getUnit();
        Iterator unitIt = aUnitMap.keySet().iterator();
        while (unitIt.hasNext()) {
            aUnitModel.addElement((String)aUnitMap.get(unitIt.next()));
        }
        ParamUnitText.setModel(aUnitModel);
    }

    private void initPanel() {

        if (itsAccessRights==null) {
            itsAccessRights = new AccessRights();
        }
        itsAccessRights.setMainFrame(itsMainFrame);

        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();


        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }

        try {
            itsTreeType = OtdbRmi.getRemoteTypes().getTreeType(OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(), false).type);
        } catch (RemoteException ex) {
            String aS="Error getting treetype "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }

        if (itsParam != null) {
            setParamName(itsParam.name);
            setIndex(String.valueOf(itsParam.index));
            setType(itsParam.type);
            CardLayout cl=(CardLayout)CardPanel.getLayout();
            if (this.getType().substring(0,1).equals("p")) {
                try {
                    // Have to get new param because we need the unresolved limits field.
                    itsParam = OtdbRmi.getRemoteMaintenance().getParam(itsNode.treeID(),itsNode.paramDefID());
                } catch (RemoteException ex) {
                    String aS="Error during getParam: "+ ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }
                cl.show(CardPanel,"ComboCard");
            } else {
                cl.show(CardPanel,"TextCard");
            }
            setUnit(itsParam.unit);
            setPruning(String.valueOf(itsParam.pruning));
            setValMoment(String.valueOf(itsParam.valMoment));
            setRuntimeMod(itsParam.runtimeMod);
            if (itsParam != null && LofarUtils.isReference(itsNode.limits)) {
                derefText.setVisible(true);
                derefLabel.setVisible(true);
                setDeref(itsParam.limits);
            } else {
                derefText.setVisible(false);
                derefLabel.setVisible(false);
            }
            setDescription(itsParam.description);
            CardPanel.setContent(itsMainFrame,this,itsNode,itsParam);
        } else {
            logger.error("no Param given");
        }
        if (itsTreeType.equals("hardware")) {
            CardPanel.setVisible(false);
            this.setButtonsVisible(false);
        } else {
            CardPanel.setVisible(true);
            this.setButtonsVisible(true);
        }

    }

    private String getParamName() {
        return this.ParamNameText.getText();
    }

    private void setParamName(String aS) {
        this.ParamNameText.setText(aS);
    }

    private void enableParamName(boolean enabled) {
        this.ParamNameText.setEnabled(enabled);
        this.ParamNameText.setEditable(enabled);
    }

    private String getIndex() {
        return this.ParamIndexText.getText();
    }

    private void setIndex(String aS) {
        this.ParamIndexText.setText(aS);
    }

    private void enableIndex(boolean enabled) {
        this.ParamIndexText.setEnabled(enabled);
        this.ParamIndexText.setEditable(enabled);
    }

    private String getType() {
        return (String)this.ParamTypeText.getSelectedItem();
    }

    private void setType(short aSh) {
        try {
            this.ParamTypeText.setSelectedItem(OtdbRmi.getRemoteTypes().getParamType(aSh));
        } catch (RemoteException e) {
            String aS="Error: GetParamType("+aSh+") failed " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
       }
    }

    private void enableType(boolean enabled) {
        this.ParamTypeText.setEnabled(enabled);
        this.ParamTypeText.setEditable(enabled);
    }

    private String getUnit() {
        return (String)this.ParamUnitText.getSelectedItem();
    }

    private void setUnit(short aSh) {
        try {
            this.ParamUnitText.setSelectedItem(OtdbRmi.getRemoteTypes().getUnit(aSh));
        } catch (RemoteException e) {
            String aS="ERROR: getUnit("+aSh+") failed " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }

    private void enableUnit(boolean enabled) {
        this.ParamUnitText.setEnabled(enabled);
        this.ParamUnitText.setEditable(enabled);
    }

    private String getPruning() {
        return this.ParamPruningText.getText();
    }

    private void setPruning(String aS) {
        this.ParamPruningText.setText(aS);
    }

    private void enablePruning(boolean enabled) {
        this.ParamPruningText.setEnabled(enabled);
        this.ParamPruningText.setEditable(enabled);
    }

    private String getValMoment() {
        return this.ParamValMomentText.getText();
    }

    private void setValMoment(String aS) {
        this.ParamValMomentText.setText(aS);
    }

    private void enableValMoment(boolean enabled) {
        this.ParamValMomentText.setEnabled(enabled);
        this.ParamValMomentText.setEditable(enabled);
    }

    private boolean getRuntimeMod() {
        if (this.ParamRuntimeModText.getSelectedItem().equals("true")) {
            return true;
        } else {
            return false;
        }
    }

    private void setRuntimeMod(boolean aB) {
        if (aB) {
            this.ParamRuntimeModText.setSelectedItem("true");
        } else {
            this.ParamRuntimeModText.setSelectedItem("false");
        }
    }

    private void enableRuntimeMod(boolean enabled) {
        this.ParamRuntimeModText.setEnabled(enabled);
        this.ParamRuntimeModText.setEditable(enabled);
    }


    private String getLimits() {
       return CardPanel.getValue();
    }

    private String getDeref() {
        return this.derefText.getText();
    }


    private void setDeref(String aS) {
        this.derefText.setText(aS);
    }

    private void enableLimits(boolean enabled) {
        CardPanel.setButtonsEnabled(enabled);
        CardPanel.setButtonsEditable(enabled);
    }

    private void enableDeref(boolean enabled) {
        this.derefText.setEnabled(enabled);
    }

    private String getDescription() {
        return this.ParamDescriptionText.getText();
    }

    private void setDescription(String aS) {
        this.ParamDescriptionText.setText(aS);
    }

    private void enableDescription(boolean enabled) {
        this.ParamDescriptionText.setEnabled(enabled);
        this.ParamDescriptionText.setEditable(enabled);
    }

    /** Enables the buttons
     *
     * @param   enabled     true/false enable/disable
     */
    public void enableButtons(boolean enabled) {
        this.ParamApplyButton.setEnabled(enabled);
        this.ParamCancelButton.setEnabled(enabled);
    }

    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.ParamApplyButton.setVisible(visible);
        this.ParamCancelButton.setVisible(visible);
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableParamName(enabled);
        enableIndex(enabled);
        enableType(enabled);
        enableUnit(enabled);
        enablePruning(enabled);
        enableValMoment(enabled);
        enableRuntimeMod(enabled);
//        enableLimits(enabled);
        enableDeref(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }


    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean nodeHasChanged = false;
        boolean paramHasChanged = false;
        if (itsParam != null) {
            try {
                if (itsParam.type != OtdbRmi.getRemoteTypes().getParamType(getType())) {
                    itsParam.type=OtdbRmi.getRemoteTypes().getParamType(getType());
                    paramHasChanged=true;
                }

                if (itsParam.unit != OtdbRmi.getRemoteTypes().getUnit(getUnit())) {
                    itsParam.unit=OtdbRmi.getRemoteTypes().getUnit(getUnit());
                    paramHasChanged=true;
                }

                if (!String.valueOf(itsParam.pruning).equals(getPruning())) {
                    itsParam.pruning=Integer.valueOf(getPruning()).shortValue();
                    paramHasChanged=true;
                }

                if (!String.valueOf(itsParam.valMoment).equals(getValMoment())) {
                    itsParam.valMoment=Integer.valueOf(getValMoment()).shortValue();
                    paramHasChanged=true;
                }

                if (itsParam.runtimeMod != getRuntimeMod()) {
                    itsParam.runtimeMod=getRuntimeMod();
                    paramHasChanged=true;
                }

                if (!itsParam.description.equals(getDescription())) {
                    itsParam.description=getDescription();
                    paramHasChanged=true;
                }

                if (!itsNode.limits.equals(getLimits())) {
                    itsNode.limits=getLimits();
                    nodeHasChanged=true;
                }

                if (paramHasChanged) {
                    if (!OtdbRmi.getRemoteMaintenance().saveParam(itsParam)) {
                        String aS="Saving param "+itsParam.nodeID()+","+itsParam.paramID()+"failed: "+ OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                }
                if (nodeHasChanged) {
                    if (!OtdbRmi.getRemoteMaintenance().saveNode(itsNode)) {
                        String aS="Saving node "+itsNode.nodeID()+"failed: "+ OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                }

            } catch (RemoteException ex) {
                String aS="error in Remote connection: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        } else {
            logger.error("no Param given");
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        ParamNameLabel = new javax.swing.JLabel();
        ParamIndexLabel = new javax.swing.JLabel();
        ParamTypeLabel = new javax.swing.JLabel();
        ParamLimitsLabel = new javax.swing.JLabel();
        ParamIndexText = new javax.swing.JTextField();
        ParamPruningText = new javax.swing.JTextField();
        ParamNameText = new javax.swing.JTextField();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        ParamDescriptionText = new javax.swing.JTextArea();
        ParamUnitLabel = new javax.swing.JLabel();
        ParamPruningLabel = new javax.swing.JLabel();
        ParamValMomentLabel = new javax.swing.JLabel();
        ParamRuntimeModLabel = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        ParamTypeText = new javax.swing.JComboBox();
        ParamUnitText = new javax.swing.JComboBox();
        ParamValMomentText = new javax.swing.JTextField();
        ParamRuntimeModText = new javax.swing.JComboBox();
        CardPanel = new nl.astron.lofar.sas.otbcomponents.OTBInputFieldBuilder();
        derefLabel = new javax.swing.JLabel();
        derefText = new javax.swing.JTextField();

        ParamNameLabel.setText("Name :");

        ParamIndexLabel.setText("Index :");

        ParamTypeLabel.setText("Type :");

        ParamLimitsLabel.setText("Value :");

        ParamIndexText.setEditable(false);
        ParamIndexText.setText("None");
        ParamIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setPreferredSize(new java.awt.Dimension(200, 19));

        ParamPruningText.setEditable(false);
        ParamPruningText.setText("-1");
        ParamPruningText.setToolTipText("Pruning level");
        ParamPruningText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setPreferredSize(new java.awt.Dimension(200, 19));

        ParamNameText.setEditable(false);
        ParamNameText.setText("None");
        ParamNameText.setToolTipText("Name for this Node");
        ParamNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setPreferredSize(new java.awt.Dimension(440, 19));

        ParamCancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        ParamCancelButton.setText("Cancel");
        ParamCancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamCancelButtonActionPerformed(evt);
            }
        });

        ParamApplyButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        ParamApplyButton.setText("Apply");
        ParamApplyButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamApplyButtonActionPerformed(evt);
            }
        });

        ParamDescriptionText.setEditable(false);
        ParamDescriptionText.setRows(3);
        ParamDescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));

        ParamUnitLabel.setText("Unit :");

        ParamPruningLabel.setText("Pruning :");

        ParamValMomentLabel.setText("ValMoment :");

        ParamRuntimeModLabel.setText("RuntimeMod :");

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Parameter View Panel");

        ParamTypeText.setEnabled(false);

        ParamUnitText.setEnabled(false);

        ParamValMomentText.setEditable(false);
        ParamValMomentText.setText("None");

        ParamRuntimeModText.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "false", "true" }));
        ParamRuntimeModText.setEnabled(false);

        CardPanel.setPreferredSize(new java.awt.Dimension(625, 20));

        derefLabel.setText("Deref Value:");
        derefLabel.setVisible(false);

        derefText.setEditable(false);
        derefText.setVisible(false);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(114, 114, 114)
                        .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 320, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(40, 40, 40)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createSequentialGroup()
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(ParamIndexLabel)
                                    .add(ParamNameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamTypeLabel)
                                    .add(ParamUnitLabel)
                                    .add(ParamPruningLabel)
                                    .add(ParamValMomentLabel)
                                    .add(ParamRuntimeModLabel)
                                    .add(ParamLimitsLabel))
                                .add(20, 20, 20)
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(ParamPruningText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(layout.createSequentialGroup()
                                        .add(CardPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(derefLabel)
                                        .add(15, 15, 15)
                                        .add(derefText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 365, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                        .add(org.jdesktop.layout.GroupLayout.LEADING, ParamNameText, 0, 0, Short.MAX_VALUE)
                                        .add(org.jdesktop.layout.GroupLayout.LEADING, ParamIndexText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 240, Short.MAX_VALUE))
                                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                        .add(org.jdesktop.layout.GroupLayout.LEADING, ParamRuntimeModText, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .add(org.jdesktop.layout.GroupLayout.LEADING, ParamValMomentText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 240, Short.MAX_VALUE))))
                            .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 530, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(layout.createSequentialGroup()
                                .add(ParamCancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(ParamApplyButton)))))
                .add(421, 421, 421))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(10, 10, 10)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(20, 20, 20)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamNameLabel)
                    .add(ParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(11, 11, 11)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamIndexLabel)
                    .add(ParamIndexText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamTypeLabel)
                    .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamUnitLabel)
                    .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamPruningLabel)
                    .add(ParamPruningText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamValMomentLabel)
                    .add(ParamValMomentText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamRuntimeModLabel)
                    .add(ParamRuntimeModText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamLimitsLabel)
                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                        .add(derefLabel)
                        .add(derefText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(CardPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(30, 30, 30)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamApplyButton)
                    .add(ParamCancelButton))
                .addContainerGap(33, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        if (inputFieldBuilder.currentInputField != null) {
            if (inputFieldBuilder.currentInputField.checkInput()) {
//                System.out.println("currentInputField has no mistakes,so we can safely save" );
                saveInput();
            }
        } else {
//            System.out.println("No currentInputField, so we can safely save" );
            saveInput();
        }
    }//GEN-LAST:event_ParamApplyButtonActionPerformed

    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        if (inputFieldBuilder.currentInputField != null) {
//            System.out.println("ParamCancel: currentInputField needs to be unset (if visible)" );
            inputFieldBuilder.currentInputField.checkPopup();
        }
        initPanel();
    }//GEN-LAST:event_ParamCancelButtonActionPerformed

    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    private jOTDBnode  itsNode;
    private jOTDBparam itsParam;
    private AccessRights itsAccessRights;
    private String     itsTreeType="";


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.OTBInputFieldBuilder CardPanel;
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JTextArea ParamDescriptionText;
    private javax.swing.JLabel ParamIndexLabel;
    private javax.swing.JTextField ParamIndexText;
    private javax.swing.JLabel ParamLimitsLabel;
    private javax.swing.JLabel ParamNameLabel;
    private javax.swing.JTextField ParamNameText;
    private javax.swing.JLabel ParamPruningLabel;
    private javax.swing.JTextField ParamPruningText;
    private javax.swing.JLabel ParamRuntimeModLabel;
    private javax.swing.JComboBox ParamRuntimeModText;
    private javax.swing.JLabel ParamTypeLabel;
    private javax.swing.JComboBox ParamTypeText;
    private javax.swing.JLabel ParamUnitLabel;
    private javax.swing.JComboBox ParamUnitText;
    private javax.swing.JLabel ParamValMomentLabel;
    private javax.swing.JTextField ParamValMomentText;
    private javax.swing.JLabel derefLabel;
    private javax.swing.JTextField derefText;
    private javax.swing.JLabel jLabel1;
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
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }


    public void validateFailed() {
        ParamApplyButton.setEnabled(false);
    }

    public void validatePassed() {
        ParamApplyButton.setEnabled(true);
    }


}
