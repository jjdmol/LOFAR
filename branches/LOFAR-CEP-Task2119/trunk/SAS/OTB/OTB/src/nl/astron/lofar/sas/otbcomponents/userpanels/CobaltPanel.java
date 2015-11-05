/*
 * CobaltPanel.java
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
package nl.astron.lofar.sas.otbcomponents.userpanels;

import java.awt.Component;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * Panel for Cobalt specific configuration
 *
 * @author  Coolen
 *
 * Created on 18 november 20013, 20:54
 *
 * @version $Id: CobaltPanel.java 25528 2013-07-02 09:23:01Z loose $
 */
public class CobaltPanel extends javax.swing.JPanel implements IViewPanel {

    static Logger logger = Logger.getLogger(CobaltPanel.class);
    static String name = "CobaltPanel";

    /** Creates new form CobaltPanel */
    public CobaltPanel(MainFrame aMainFrame, jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        initialize();
        initPanel();
    }

    /** Creates new form BeanForm */
    public CobaltPanel() {
        initComponents();
        initialize();
    }

    @Override
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame = aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }

    @Override
    public String getShortName() {
        return name;
    }

    @Override
    public void setContent(Object anObject) {
        itsNode = (jOTDBnode) anObject;
        jOTDBparam aParam = null;



        try {

            //we need to get all the childs from this node.    
            ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1));

            // get all the params per child
            for (jOTDBnode aNode : childs) {
                aParam = null;

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode, aParam, aNode);


                    //we need to get all the childs from the following nodes as well.
                } else if (LofarUtils.keyName(aNode.name).equals("Correlator")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("BeamFormer")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("CoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("IncoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }

            //we also need the Output_ nodes from the Dataproducts here
            ArrayList<jOTDBnode> OUchilds = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), "%Output_%"));

            // get all the params per child
            for (jOTDBnode aNode : OUchilds) {
                aParam = null;

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode, aParam, aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Output_Beamformed")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Output_CoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Output_Correlated")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Output_Filtered")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Output_IncoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }

        } catch (RemoteException ex) {
            String aS = "Error during getComponentParam: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

        initPanel();
    }

    @Override
    public boolean isSingleton() {
        return false;
    }

    @Override
    public JPanel getInstance() {
        return new CobaltPanel();
    }

    @Override
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
    @Override
    public void createPopupMenu(Component aComponent, int x, int y) {
        JPopupMenu aPopupMenu = null;
        JMenuItem aMenuItem = null;

        aPopupMenu = new JPopupMenu();
        // For VIC trees
        switch (itsTreeType) {
            case "VHtree":
                //  Fill in menu as in the example above
                aMenuItem = new JMenuItem("Create ParSet File");
                aMenuItem.addActionListener(new java.awt.event.ActionListener() {

                    @Override
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        popupMenuHandler(evt);
                    }
                });
                aMenuItem.setActionCommand("Create ParSet File");
                aPopupMenu.add(aMenuItem);

                aMenuItem = new JMenuItem("Create ParSetMeta File");
                aMenuItem.addActionListener(new java.awt.event.ActionListener() {

                    @Override
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        popupMenuHandler(evt);
                    }
                });
                aMenuItem.setActionCommand("Create ParSetMeta File");
                aPopupMenu.add(aMenuItem);

                // For template trees
                break;
            case "VItemplate":
                break;
        }

        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y);
    }

    /** handles the choice from the popupmenu 
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }  
     */
    @Override
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
        switch (evt.getActionCommand()) {
            case "Create ParSet File":
                {
            logger.trace("Create ParSet File");
            int aTreeID = itsMainFrame.getSharedVars().getTreeID();
            if (fc == null) {
                fc = new JFileChooser();
                fc.setApproveButtonText("Apply");
            }
            // try to get a new filename to write the parsetfile to
            if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                try {
                    File aFile = fc.getSelectedFile();

                    // create filename that can be used at the remote site    
                    String aRemoteFileName = "/tmp/" + aTreeID + "-" + itsNode.name + "_" + itsMainFrame.getUserAccount().getUserName() + ".ParSet";

                    // write the parset
                            OtdbRmi.getRemoteMaintenance().exportTree(aTreeID, itsNode.nodeID(), aRemoteFileName);

                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                    try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                        output.write(dldata, 0, dldata.length);
                        output.flush();
                    }
                    logger.trace("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    String aS = "exportTree failed : " + ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                } catch (FileNotFoundException ex) {
                    String aS = "Error during newPICTree creation: " + ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                } catch (IOException ex) {
                    String aS = "Error during newPICTree creation: " + ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }
            }
                    break;
        }
            case "Create ParSetMeta File":
                {
                    logger.trace("Create ParSetMeta File");
                    int aTreeID = itsMainFrame.getSharedVars().getTreeID();
                    if (fc == null) {
                        fc = new JFileChooser();
                        fc.setApproveButtonText("Apply");
    }
                    // try to get a new filename to write the parsetfile to
                    if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                        try {
                            File aFile = fc.getSelectedFile();

                            // create filename that can be used at the remote site    
                            String aRemoteFileName = "/tmp/" + aTreeID + "-" + itsNode.name + "_" + itsMainFrame.getUserAccount().getUserName() + ".ParSetMeta";

                            // write the parset
                            OtdbRmi.getRemoteMaintenance().exportResultTree(aTreeID, itsNode.nodeID(), aRemoteFileName);

                            //obtain the remote file
                            byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                            try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                                output.write(dldata, 0, dldata.length);
                                output.flush();
                            }
                            logger.trace("File written to: " + aFile.getPath());
                        } catch (RemoteException ex) {
                            String aS = "exportResultTree failed : " + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        } catch (FileNotFoundException ex) {
                            String aS = "Error during newPICTree creation: " + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        } catch (IOException ex) {
                            String aS = "Error during newPICTree creation: " + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
                }
        }
    }

    /** 
     * Helper method that retrieves the child nodes for a given jOTDBnode, 
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode) {
        jOTDBparam aParam = null;
        try {
            ArrayList<jOTDBnode> HWchilds = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
            // get all the params per child
            for (jOTDBnode aHWNode : HWchilds) {
                aParam = null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode, aParam, aHWNode);
            }
        } catch (RemoteException ex) {
            String aS = "Error during retrieveAndDisplayChildDataForNode: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }
    }

    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent, jOTDBparam aParam, jOTDBnode aNode) {
        if (aParam == null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        /* Set's the different fields in the GUI */

        // Generic Cobalt
        if (aParam == null) {
            return;
        }
        logger.debug("setField for: " + aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0, 1).equals("p")) {
                // Have to get new param because we need the unresolved limits field.
                aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(), aNode.paramDefID());
            }
        } catch (RemoteException ex) {
            String aS = "Error during getParam: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
        switch (parentName) {
            case "Correlator":
                // Cobalt Correlator params
                switch (aKeyName) {
                    case "integrationTime":
                        inputIntegrationTime.setToolTipText(aParam.description);
                        itsIntegrationTime = aNode;
                        if (isRef && aParam != null) {
                            inputIntegrationTime.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputIntegrationTime.setText(aNode.limits);
                        }
                        break;
                    case "nrChannelsPerSubband":
                        inputChannelsPerSubbandCorrelator.setToolTipText(aParam.description);
                        itsChannelsPerSubbandCorrelator = aNode;
                        if (isRef && aParam != null) {
                            inputChannelsPerSubbandCorrelator.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputChannelsPerSubbandCorrelator.setText(aNode.limits);
                        }
                        break;
                    case "nrBlocksPerIntegration":
                        inputBlocksPerIntegration.setToolTipText(aParam.description);
                        itsBlocksPerIntegration = aNode;
                        if (isRef && aParam != null) {
                            inputBlocksPerIntegration.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputBlocksPerIntegration.setText(aNode.limits);
                        }
                        break;
                }
                break;
            case "CoherentStokes":
                // Cobalt coherent stokes params
                switch (aKeyName) {
                    case "which":
                        inputWhichCoherent.setToolTipText(aParam.description);
                        LofarUtils.setPopupComboChoices(inputWhichCoherent, aParam.limits);
                        if (!aNode.limits.equals("")) {
                            inputWhichCoherent.setSelectedItem(aNode.limits);
                        }
                        itsWhichCoherent = aNode;
                        break;
                    case "channelsPerSubband":
                        inputChannelsPerSubbandCoherent.setToolTipText(aParam.description);
                        itsChannelsPerSubbandCoherent = aNode;
                        if (isRef && aParam != null) {
                            inputChannelsPerSubbandCoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputChannelsPerSubbandCoherent.setText(aNode.limits);
                        }
                        break;
                    case "timeIntegrationFactor":
                        inputTimeIntegrationFactorCoherent.setToolTipText(aParam.description);
                        itsTimeIntegrationFactorCoherent = aNode;
                        if (isRef && aParam != null) {
                            inputTimeIntegrationFactorCoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputTimeIntegrationFactorCoherent.setText(aNode.limits);
                        }
                        break;
                    case "subbandsPerFile":
                        inputSubbandsPerFileCoherent.setToolTipText(aParam.description);
                        itsSubbandsPerFileCoherent = aNode;
                        if (isRef && aParam != null) {
                            inputSubbandsPerFileCoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputSubbandsPerFileCoherent.setText(aNode.limits);
                        }
                        break;
                }
                break;
            case "IncoherentStokes":
                // Cobalt IncoherentStokes params
                
                switch (aKeyName) {
                    case "which":
                        inputWhichIncoherent.setToolTipText(aParam.description);
                        LofarUtils.setPopupComboChoices(inputWhichIncoherent, aParam.limits);
                        if (!aNode.limits.equals("")) {
                            inputWhichIncoherent.setSelectedItem(aNode.limits);
                        }
                        itsWhichIncoherent = aNode;
                        break;
                    case "channelsPerSubband":
                        inputChannelsPerSubbandIncoherent.setToolTipText(aParam.description);
                        itsChannelsPerSubbandIncoherent = aNode;
                        if (isRef && aParam != null) {
                            inputChannelsPerSubbandIncoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputChannelsPerSubbandIncoherent.setText(aNode.limits);
                        }
                        break;
                    case "timeIntegrationFactor":
                        inputTimeIntegrationFactorIncoherent.setToolTipText(aParam.description);
                        itsTimeIntegrationFactorIncoherent = aNode;
                        if (isRef && aParam != null) {
                            inputTimeIntegrationFactorIncoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputTimeIntegrationFactorIncoherent.setText(aNode.limits);
                        }
                        break;
                    case "subbandsPerFile":
                        inputSubbandsPerFileIncoherent.setToolTipText(aParam.description);
                        itsSubbandsPerFileIncoherent = aNode;
                        if (isRef && aParam != null) {
                            inputSubbandsPerFileIncoherent.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputSubbandsPerFileIncoherent.setText(aNode.limits);
                        }
                        break;
                }
                break;
            case "BeamFormer":
                // BeamFormer Specific parameters
                switch (aKeyName) {
                    case "coherentDedisperseChannels": {
                        inputCoherentDedisperseChannels.setToolTipText(aParam.description);
                        itsCoherentDedisperseChannels = aNode;
                        boolean aSelection = false;
                        if (isRef && aParam != null) {
                            if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        } else {
                            if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        }
                        inputCoherentDedisperseChannels.setSelected(aSelection);
                        break;
                    }
                }
                break;
            case "Cobalt":
                // Cobalt Specific parameters
                switch (aKeyName) {
                    case "delayCompensation": {
                        inputDelayCompensation.setToolTipText(aParam.description);
                        itsDelayCompensation = aNode;
                        boolean aSelection = false;
                        if (isRef && aParam != null) {
                            if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        } else {
                            if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        }
                        inputDelayCompensation.setSelected(aSelection);
                        break;
                    }
                    case "correctBandPass": {
                        inputCorrectBandPass.setToolTipText(aParam.description);
                        itsCorrectBandPass = aNode;
                        boolean aSelection = false;
                        if (isRef && aParam != null) {
                            if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        } else {
                            if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        }
                        inputCorrectBandPass.setSelected(aSelection);
                        break;
                    }
                    case "correctClocks": {
                        inputCorrectClocks.setToolTipText(aParam.description);
                        itsCorrectClocks = aNode;
                        boolean aSelection = false;
                        if (isRef && aParam != null) {
                            if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        } else {
                            if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        }
                        inputCorrectClocks.setSelected(aSelection);
                        break;
                    }
                    case "realTime": {
                        inputRealTime.setToolTipText(aParam.description);
                        itsRealTime = aNode;
                        boolean aSelection = false;
                        if (isRef && aParam != null) {
                            if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        } else {
                            if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                                aSelection = true;
                            }
                        }
                        inputRealTime.setSelected(aSelection);
                        break;
                    }
                    case "blockSize":
                        inputBlockSize.setToolTipText(aParam.description);
                        itsBlockSize = aNode;
                        if (isRef && aParam != null) {
                            inputBlockSize.setText(aNode.limits + " : " + aParam.limits);
                        } else {
                            inputBlockSize.setText(aNode.limits);
                        }
                        break;
                }
                break;
            case "Output_Beamformed":
                if (aKeyName.equals("enabled")) {
                    inputOutputBeamFormedData.setToolTipText(aParam.description);
                    itsOutputBeamFormedData = aNode;
                    boolean aSelection = false;
                    if (isRef && aParam != null) {
                        if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    } else {
                        if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    }
                    inputOutputBeamFormedData.setSelected(aSelection);
                    checkSettings();
                }
                break;
            case "Output_Correlated":
                if (aKeyName.equals("enabled")) {
                    inputOutputCorrelatedData.setToolTipText(aParam.description);
                    itsOutputCorrelatedData = aNode;
                    boolean aSelection = false;
                    if (isRef && aParam != null) {
                        if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    } else {
                        if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    }
                    inputOutputCorrelatedData.setSelected(aSelection);
                    inputIntegrationTime.setEnabled(aSelection);
                    checkSettings();
                }
                break;
            case "Output_Filtered":
                if (aKeyName.equals("enabled")) {
                    inputOutputFilteredData.setToolTipText(aParam.description);
                    itsOutputFilteredData = aNode;
                    boolean aSelection = false;
                    if (isRef && aParam != null) {
                        if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    } else {
                        if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    }
                    inputOutputFilteredData.setSelected(aSelection);
                    checkSettings();
                }
                break;
            case "Output_CoherentStokes":
                if (aKeyName.equals("enabled")) {
                    inputOutputCoherentStokes.setToolTipText(aParam.description);
                    itsOutputCoherentStokes = aNode;
                    boolean aSelection = false;
                    if (isRef && aParam != null) {
                        if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    } else {
                        if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    }
                    inputOutputCoherentStokes.setSelected(aSelection);
                    checkSettings();
                }
                break;
            case "Output_IncoherentStokes":
                if (aKeyName.equals("enabled")) {
                    inputOutputIncoherentStokes.setToolTipText(aParam.description);
                    itsOutputIncoherentStokes = aNode;
                    boolean aSelection = false;
                    if (isRef && aParam != null) {
                        if (aParam.limits.equals("true") || aParam.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    } else {
                        if (aNode.limits.equals("true") || aNode.limits.equals("TRUE")) {
                            aSelection = true;
                        }
                    }
                    inputOutputIncoherentStokes.setSelected(aSelection);
                    checkSettings();
                }
                break;
        }
    }

    // check all settings to make a choice about enabled/disables fields
    private void checkSettings() {
        if (inputOutputCorrelatedData.isSelected()) {
            inputIntegrationTime.setEnabled(true);
            inputChannelsPerSubbandCorrelator.setEnabled(true);
            inputBlocksPerIntegration.setEnabled(true);
        } else {
            inputIntegrationTime.setEnabled(false);
            inputChannelsPerSubbandCorrelator.setEnabled(false);
            inputBlocksPerIntegration.setEnabled(false);
        }


        if (inputOutputBeamFormedData.isSelected() || inputOutputCoherentStokes.isSelected()) {
            if (inputOutputBeamFormedData.isSelected()) {
                inputOutputCoherentStokes.setSelected(false);
                inputOutputCoherentStokes.setEnabled(false);
            } else if (inputOutputCoherentStokes.isSelected()) {
                inputOutputBeamFormedData.setSelected(false);
                inputOutputBeamFormedData.setEnabled(false);
            }
        }

        if (inputOutputCoherentStokes.isSelected()) {
            inputWhichCoherent.setEnabled(true);
            inputChannelsPerSubbandCoherent.setEnabled(true);
            inputTimeIntegrationFactorCoherent.setEnabled(true);
            inputSubbandsPerFileCoherent.setEnabled(true);
        } else {
            inputWhichCoherent.setEnabled(false);
            inputChannelsPerSubbandCoherent.setEnabled(false);
            inputTimeIntegrationFactorCoherent.setEnabled(false);
            inputSubbandsPerFileCoherent.setEnabled(false);
        }
        if (inputOutputIncoherentStokes.isSelected()) {
            inputWhichIncoherent.setEnabled(true);
            inputChannelsPerSubbandIncoherent.setEnabled(true);
            inputTimeIntegrationFactorIncoherent.setEnabled(true);
            inputSubbandsPerFileIncoherent.setEnabled(true);
        } else {
            inputWhichIncoherent.setEnabled(false);
            inputChannelsPerSubbandIncoherent.setEnabled(false);
            inputTimeIntegrationFactorIncoherent.setEnabled(false);
            inputSubbandsPerFileIncoherent.setEnabled(false);
        }

    }

    private void restore() {
        boolean aB = false;

        // Cobalt Specific parameters
        if (itsRealTime != null) {
            aB = false;
            if (itsRealTime.limits.equals("true") || itsRealTime.limits.equals("TRUE")) {
                aB = true;
            }
            inputRealTime.setSelected(aB);
        }
        if (itsDelayCompensation != null) {
            aB = false;
            if (itsDelayCompensation.limits.equals("true") || itsDelayCompensation.limits.equals("TRUE")) {
                aB = true;
            }
            inputDelayCompensation.setSelected(aB);
        }
        if (itsCorrectBandPass != null) {
            aB = false;
            if (itsCorrectBandPass.limits.equals("true") || itsCorrectBandPass.limits.equals("TRUE")) {
                aB = true;
            }
            inputCorrectBandPass.setSelected(aB);
        }
        if (itsCorrectClocks != null) {
            aB = false;
            if (itsCorrectClocks.limits.equals("true") || itsCorrectClocks.limits.equals("TRUE")) {
                aB = true;
            }
            inputCorrectClocks.setSelected(aB);
        }
        if (itsCoherentDedisperseChannels != null) {
            aB = false;
            if (itsCoherentDedisperseChannels.limits.equals("true") || itsCoherentDedisperseChannels.limits.equals("TRUE")) {
                aB = true;
            }
            inputCoherentDedisperseChannels.setSelected(aB);
        }
        
        if (itsBlockSize != null) {
            inputBlockSize.setText(itsBlockSize.limits);
        }
        
        if (itsOutputCorrelatedData != null) {
            aB = false;
            if (itsOutputCorrelatedData.limits.equals("true") || itsOutputCorrelatedData.limits.equals("TRUE")) {
                aB = true;
            }
            inputOutputCorrelatedData.setSelected(aB);
        }
        if (itsOutputFilteredData != null) {
            aB = false;
            if (itsOutputFilteredData.limits.equals("true") || itsOutputFilteredData.limits.equals("TRUE")) {
                aB = true;
            }
            inputOutputFilteredData.setSelected(aB);
        }
        if (itsOutputBeamFormedData != null) {
            aB = false;
            if (itsOutputBeamFormedData.limits.equals("true") || itsOutputBeamFormedData.limits.equals("TRUE")) {
                aB = true;
            }
            inputOutputBeamFormedData.setSelected(aB);
        }
        if (itsOutputCoherentStokes != null) {
            aB = false;
            if (itsOutputCoherentStokes.limits.equals("true") || itsOutputCoherentStokes.limits.equals("TRUE")) {
                aB = true;
            }
            inputOutputCoherentStokes.setSelected(aB);
        }
        if (itsOutputIncoherentStokes != null) {
            aB = false;
            if (itsOutputIncoherentStokes.limits.equals("true") || itsOutputIncoherentStokes.limits.equals("TRUE")) {
                aB = true;
            }
            inputOutputIncoherentStokes.setSelected(aB);
        }


        // Correlator
        if (itsIntegrationTime != null) {
            inputIntegrationTime.setText(itsIntegrationTime.limits);
        }
        if (itsChannelsPerSubbandCorrelator != null) {
            inputChannelsPerSubbandCorrelator.setText(itsChannelsPerSubbandCorrelator.limits);
        }
        if (itsBlocksPerIntegration != null) {
            inputBlocksPerIntegration.setText(itsBlocksPerIntegration.limits);
        }


        // CoherentStokes
        if (itsWhichCoherent != null) {
            inputWhichCoherent.setSelectedItem(itsWhichCoherent.limits);
        }
        if (itsChannelsPerSubbandCoherent != null) {
            inputChannelsPerSubbandCoherent.setText(itsChannelsPerSubbandCoherent.limits);
        }
        if (itsTimeIntegrationFactorCoherent != null) {
            inputTimeIntegrationFactorCoherent.setText(itsTimeIntegrationFactorCoherent.limits);
        }
        if (itsSubbandsPerFileCoherent != null) {
            inputSubbandsPerFileCoherent.setText(itsSubbandsPerFileCoherent.limits);
        }

        // CNProc_IncoherentStokes
        if (itsWhichIncoherent != null) {
            inputWhichIncoherent.setSelectedItem(itsWhichIncoherent.limits);
        }
        if (itsChannelsPerSubbandIncoherent != null) {
            inputChannelsPerSubbandIncoherent.setText(itsChannelsPerSubbandIncoherent.limits);
        }
        if (itsTimeIntegrationFactorIncoherent != null) {
            inputTimeIntegrationFactorIncoherent.setText(itsTimeIntegrationFactorIncoherent.limits);
        }
        if (itsSubbandsPerFileIncoherent != null) {
            inputSubbandsPerFileIncoherent.setText(itsSubbandsPerFileIncoherent.limits);
        }


        checkSettings();
    }

    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.setButtonIcon("Restore", new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_undo.png")));
        buttonPanel1.addButton("Apply");
        buttonPanel1.setButtonIcon("Apply", new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png")));
    }

    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        setAllEnabled(true);

        if (userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if (userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if (userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }


        if (itsNode != null) {
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(), false);
                itsTreeType = OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                String aS = "CobaltPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType = "";
            }
        } else {
            logger.error("no node given");
        }

        restore();
    }

    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            OtdbRmi.getRemoteMaintenance().saveNode(aNode);
        } catch (RemoteException ex) {
            String aS = "Error: saveNode failed : " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }

    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void enableButtons(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }

    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    @Override
    public void setButtonsVisible(boolean visible) {
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void setAllEnabled(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }

    private boolean saveInput() {
        boolean hasChanged = false;

        // Generic Cobalt       
        if ((!inputDelayCompensation.isSelected()
                & (itsDelayCompensation.limits.equals("TRUE") || itsDelayCompensation.limits.equals("true")))
                || (inputDelayCompensation.isSelected()
                & (itsDelayCompensation.limits.equals("FALSE") || itsDelayCompensation.limits.equals("false")))) {
            String delay = "true";
            if (!inputDelayCompensation.isSelected()) {
                delay = "false";
            }
            itsDelayCompensation.limits = delay;
            saveNode(itsDelayCompensation);
        }
        if ((!inputCorrectBandPass.isSelected()
                & (itsCorrectBandPass.limits.equals("TRUE") || itsCorrectBandPass.limits.equals("true")))
                || (inputCorrectBandPass.isSelected()
                & (itsCorrectBandPass.limits.equals("FALSE") || itsCorrectBandPass.limits.equals("false")))) {
            String bp = "true";
            if (!inputCorrectBandPass.isSelected()) {
                bp = "false";
            }
            itsCorrectBandPass.limits = bp;
            saveNode(itsCorrectBandPass);
        }
        if ((!inputCorrectClocks.isSelected()
                & (itsCorrectClocks.limits.equals("TRUE") || itsCorrectClocks.limits.equals("true")))
                || (inputCorrectClocks.isSelected()
                & (itsCorrectClocks.limits.equals("FALSE") || itsCorrectClocks.limits.equals("false")))) {
            String cc = "true";
            if (!inputCorrectClocks.isSelected()) {
                cc = "false";
            }
            itsCorrectClocks.limits = cc;
            saveNode(itsCorrectClocks);
        }
        if ((!inputRealTime.isSelected()
                & (itsRealTime.limits.equals("TRUE") || itsRealTime.limits.equals("true")))
                || (inputRealTime.isSelected()
                & (itsRealTime.limits.equals("FALSE") || itsRealTime.limits.equals("false")))) {
            String rt = "true";
            if (!inputRealTime.isSelected()) {
                rt = "false";
            }
            itsRealTime.limits = rt;
            saveNode(itsRealTime);
        }

        if (itsBlockSize != null && !inputBlockSize.getText().equals(itsBlockSize.limits)) {
            itsBlockSize.limits = inputBlockSize.getText();
            saveNode(itsBlockSize);
        }
        if ((!inputOutputCorrelatedData.isSelected()
                & (itsOutputCorrelatedData.limits.equals("TRUE") || itsOutputCorrelatedData.limits.equals("true")))
                || (inputOutputCorrelatedData.isSelected()
                & (itsOutputCorrelatedData.limits.equals("FALSE") || itsOutputCorrelatedData.limits.equals("false")))) {
            String bp = "true";
            if (!inputOutputCorrelatedData.isSelected()) {
                bp = "false";
            }
            itsOutputCorrelatedData.limits = bp;
            saveNode(itsOutputCorrelatedData);
        }
        if ((!inputOutputFilteredData.isSelected()
                & (itsOutputFilteredData.limits.equals("TRUE") || itsOutputFilteredData.limits.equals("true")))
                || (inputOutputFilteredData.isSelected()
                & (itsOutputFilteredData.limits.equals("FALSE") || itsOutputFilteredData.limits.equals("false")))) {
            String bp = "true";
            if (!inputOutputFilteredData.isSelected()) {
                bp = "false";
            }
            itsOutputFilteredData.limits = bp;
            saveNode(itsOutputFilteredData);
        }
        if ((!inputOutputBeamFormedData.isSelected()
                & (itsOutputBeamFormedData.limits.equals("TRUE") || itsOutputBeamFormedData.limits.equals("true")))
                || (inputOutputBeamFormedData.isSelected()
                & (itsOutputBeamFormedData.limits.equals("FALSE") || itsOutputBeamFormedData.limits.equals("false")))) {
            String bp = "true";
            if (!inputOutputBeamFormedData.isSelected()) {
                bp = "false";
            }
            itsOutputBeamFormedData.limits = bp;
            saveNode(itsOutputBeamFormedData);
        }
        if ((!inputOutputCoherentStokes.isSelected()
                & (itsOutputCoherentStokes.limits.equals("TRUE") || itsOutputCoherentStokes.limits.equals("true")))
                || (inputOutputCoherentStokes.isSelected()
                & (itsOutputCoherentStokes.limits.equals("FALSE") || itsOutputCoherentStokes.limits.equals("false")))) {
            String bp = "true";
            if (!inputOutputCoherentStokes.isSelected()) {
                bp = "false";
            }
            itsOutputCoherentStokes.limits = bp;
            saveNode(itsOutputCoherentStokes);
        }
        if ((!inputOutputIncoherentStokes.isSelected()
                & (itsOutputIncoherentStokes.limits.equals("TRUE") || itsOutputIncoherentStokes.limits.equals("true")))
                || (inputOutputIncoherentStokes.isSelected()
                & (itsOutputIncoherentStokes.limits.equals("FALSE") || itsOutputIncoherentStokes.limits.equals("false")))) {
            String bp = "true";
            if (!inputOutputIncoherentStokes.isSelected()) {
                bp = "false";
            }
            itsOutputIncoherentStokes.limits = bp;
            saveNode(itsOutputIncoherentStokes);
        }




        // Correlator
        if (itsIntegrationTime != null && !inputIntegrationTime.getText().equals(itsIntegrationTime.limits)) {
            itsIntegrationTime.limits = inputIntegrationTime.getText();
            saveNode(itsIntegrationTime);
        }
        if (itsChannelsPerSubbandCorrelator != null && !inputChannelsPerSubbandCorrelator.getText().equals(itsChannelsPerSubbandCorrelator.limits)) {
            itsChannelsPerSubbandCorrelator.limits = inputChannelsPerSubbandCorrelator.getText();
            saveNode(itsChannelsPerSubbandCorrelator);
        }
        if (itsBlocksPerIntegration != null && !inputBlocksPerIntegration.getText().equals(itsBlocksPerIntegration.limits)) {
            itsBlocksPerIntegration.limits = inputBlocksPerIntegration.getText();
            saveNode(itsBlocksPerIntegration);
        }


        // BeamFormer
        if ((!inputCoherentDedisperseChannels.isSelected()
                & (itsCoherentDedisperseChannels.limits.equals("TRUE") || itsCoherentDedisperseChannels.limits.equals("true")))
                || (inputOutputIncoherentStokes.isSelected()
                & (itsCoherentDedisperseChannels.limits.equals("FALSE") || itsCoherentDedisperseChannels.limits.equals("false")))) {
            String bp = "true";
            if (!inputCoherentDedisperseChannels.isSelected()) {
                bp = "false";
            }
            itsCoherentDedisperseChannels.limits = bp;
            saveNode(itsCoherentDedisperseChannels);
        }
        
        // CoherentStokes
        if (itsWhichCoherent != null && !inputWhichCoherent.getSelectedItem().toString().equals(itsWhichCoherent.limits)) {
            itsWhichCoherent.limits = inputWhichCoherent.getSelectedItem().toString();
            saveNode(itsWhichCoherent);
        }

        if (itsTimeIntegrationFactorCoherent != null && !inputTimeIntegrationFactorCoherent.getText().equals(itsTimeIntegrationFactorCoherent.limits)) {
            itsTimeIntegrationFactorCoherent.limits = inputTimeIntegrationFactorCoherent.getText();
            saveNode(itsTimeIntegrationFactorCoherent);
        }
        if (itsChannelsPerSubbandCoherent != null && inputChannelsPerSubbandCoherent.getText().equals(itsChannelsPerSubbandCoherent.limits)) {
            itsChannelsPerSubbandCoherent.limits = inputChannelsPerSubbandCoherent.getText();
            saveNode(itsChannelsPerSubbandCoherent);
        }
        if (itsSubbandsPerFileCoherent != null && inputSubbandsPerFileCoherent.getText().equals(itsSubbandsPerFileCoherent.limits)) {
            itsSubbandsPerFileCoherent.limits = inputSubbandsPerFileCoherent.getText();
            saveNode(itsSubbandsPerFileCoherent);
        }

        // IncoherentStokes
        if (itsWhichIncoherent != null && !inputWhichIncoherent.getSelectedItem().toString().equals(itsWhichIncoherent.limits)) {
            itsWhichIncoherent.limits = inputWhichIncoherent.getSelectedItem().toString();
            saveNode(itsWhichIncoherent);
        }

        if (itsTimeIntegrationFactorIncoherent != null && !inputTimeIntegrationFactorIncoherent.getText().equals(itsTimeIntegrationFactorIncoherent.limits)) {
            itsTimeIntegrationFactorIncoherent.limits = inputTimeIntegrationFactorIncoherent.getText();
            saveNode(itsTimeIntegrationFactorIncoherent);
        }
        if (itsChannelsPerSubbandIncoherent != null && inputChannelsPerSubbandIncoherent.getText().equals(itsChannelsPerSubbandIncoherent.limits)) {
            itsChannelsPerSubbandIncoherent.limits = inputChannelsPerSubbandIncoherent.getText();
            saveNode(itsChannelsPerSubbandIncoherent);
        }
        if (itsSubbandsPerFileIncoherent != null && inputSubbandsPerFileIncoherent.getText().equals(itsSubbandsPerFileIncoherent.limits)) {
            itsSubbandsPerFileIncoherent.limits = inputSubbandsPerFileIncoherent.getText();
            saveNode(itsSubbandsPerFileIncoherent);
        }

        return true;
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel9 = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        jPanel8 = new javax.swing.JPanel();
        inputDelayCompensation = new javax.swing.JCheckBox();
        inputCorrectBandPass = new javax.swing.JCheckBox();
        inputCorrectClocks = new javax.swing.JCheckBox();
        inputRealTime = new javax.swing.JCheckBox();
        labelBlockSize = new javax.swing.JLabel();
        inputBlockSize = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        inputOutputCorrelatedData = new javax.swing.JCheckBox();
        inputOutputFilteredData = new javax.swing.JCheckBox();
        inputOutputBeamFormedData = new javax.swing.JCheckBox();
        inputOutputCoherentStokes = new javax.swing.JCheckBox();
        inputOutputIncoherentStokes = new javax.swing.JCheckBox();
        jPanel7 = new javax.swing.JPanel();
        CoherentStokesPanel = new javax.swing.JPanel();
        labelWhichCoherent = new javax.swing.JLabel();
        inputWhichCoherent = new javax.swing.JComboBox();
        timeIntegrationFactorCoherent = new javax.swing.JLabel();
        inputTimeIntegrationFactorCoherent = new javax.swing.JTextField();
        labelChannelsPerSubbandCoherent = new javax.swing.JLabel();
        inputChannelsPerSubbandCoherent = new javax.swing.JTextField();
        labelSubbandsPerFileCoherent = new javax.swing.JLabel();
        inputSubbandsPerFileCoherent = new javax.swing.JTextField();
        IncoherentStokesPanel = new javax.swing.JPanel();
        labelWhichIncoherent = new javax.swing.JLabel();
        inputWhichIncoherent = new javax.swing.JComboBox();
        labelTimeIntegrationfactorCoherent = new javax.swing.JLabel();
        inputTimeIntegrationFactorIncoherent = new javax.swing.JTextField();
        labelChannelsPerSubbandIncoherent = new javax.swing.JLabel();
        inputChannelsPerSubbandIncoherent = new javax.swing.JTextField();
        labelSubbandsPerFileIncoherent = new javax.swing.JLabel();
        inputSubbandsPerFileIncoherent = new javax.swing.JTextField();
        inputCoherentDedisperseChannels = new javax.swing.JCheckBox();
        CorrelatorPanel = new javax.swing.JPanel();
        labelIntegrationTime = new javax.swing.JLabel();
        inputIntegrationTime = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        inputChannelsPerSubbandCorrelator = new javax.swing.JTextField();
        inputBlocksPerIntegration = new javax.swing.JTextField();
        labelChannelsPerSubbandCorrelator = new javax.swing.JLabel();
        labelBlocksPerIntegration = new javax.swing.JLabel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );

        setMinimumSize(new java.awt.Dimension(800, 600));
        setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel1.setPreferredSize(new java.awt.Dimension(100, 25));
        jPanel1.setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Cobalt Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel2.setPreferredSize(new java.awt.Dimension(5240, 3000));

        jPanel8.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Cobalt", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        inputDelayCompensation.setText("Delay Compensation");
        inputDelayCompensation.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputDelayCompensation.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputCorrectBandPass.setText("Correct BandPass");
        inputCorrectBandPass.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputCorrectBandPass.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputCorrectClocks.setText("Correct Clocks");
        inputCorrectClocks.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputCorrectClocks.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputRealTime.setText("realTime");
        inputRealTime.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputRealTime.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelBlockSize.setText("blockSize");

        inputBlockSize.setEnabled(false);

        jLabel3.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel3.setText("Output:");

        inputOutputCorrelatedData.setText("Correlated Data");
        inputOutputCorrelatedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCorrelatedDataActionPerformed(evt);
            }
        });

        inputOutputFilteredData.setText("Filtered Data");

        inputOutputBeamFormedData.setText("Beamformed Data");
        inputOutputBeamFormedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputBeamFormedDataActionPerformed(evt);
            }
        });

        inputOutputCoherentStokes.setText("Coherent Stokes");
        inputOutputCoherentStokes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCoherentStokesActionPerformed(evt);
            }
        });

        inputOutputIncoherentStokes.setText("Incoherent Stokes");
        inputOutputIncoherentStokes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputIncoherentStokesActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGap(21, 21, 21)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addComponent(labelBlockSize)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(inputBlockSize, javax.swing.GroupLayout.PREFERRED_SIZE, 86, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(145, 145, 145))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addComponent(inputRealTime, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGap(238, 238, 238))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(inputDelayCompensation, javax.swing.GroupLayout.DEFAULT_SIZE, 253, Short.MAX_VALUE)
                                .addComponent(inputCorrectBandPass))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(inputCorrectClocks)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 135, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGap(63, 63, 63)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputOutputIncoherentStokes)
                    .addComponent(inputOutputCoherentStokes)
                    .addComponent(inputOutputBeamFormedData)
                    .addComponent(jLabel3)
                    .addComponent(inputOutputCorrelatedData)
                    .addComponent(inputOutputFilteredData))
                .addGap(241, 241, 241))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(15, 15, 15)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelBlockSize)
                            .addComponent(inputBlockSize, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(inputRealTime, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(18, 18, 18)
                        .addComponent(inputDelayCompensation, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(18, 18, 18)
                        .addComponent(inputCorrectBandPass, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(18, 18, 18)
                        .addComponent(inputCorrectClocks, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(jLabel3)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputCorrelatedData)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputFilteredData)
                        .addGap(4, 4, 4)
                        .addComponent(inputOutputBeamFormedData)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(inputOutputCoherentStokes)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputIncoherentStokes)))
                .addContainerGap(21, Short.MAX_VALUE))
        );

        jPanel7.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "BeamFormer", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        jPanel7.setToolTipText("BGLProc");

        CoherentStokesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Coherent Stokes", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        CoherentStokesPanel.setToolTipText("Stokes");

        labelWhichCoherent.setText("which");

        inputWhichCoherent.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputWhichCoherent.setEnabled(false);

        timeIntegrationFactorCoherent.setText("time integration factor");

        inputTimeIntegrationFactorCoherent.setEnabled(false);

        labelChannelsPerSubbandCoherent.setText("# channels per subband");

        inputChannelsPerSubbandCoherent.setEnabled(false);

        labelSubbandsPerFileCoherent.setText("# subbands per file");

        inputSubbandsPerFileCoherent.setEnabled(false);

        javax.swing.GroupLayout CoherentStokesPanelLayout = new javax.swing.GroupLayout(CoherentStokesPanel);
        CoherentStokesPanel.setLayout(CoherentStokesPanelLayout);
        CoherentStokesPanelLayout.setHorizontalGroup(
            CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelSubbandsPerFileCoherent, javax.swing.GroupLayout.DEFAULT_SIZE, 183, Short.MAX_VALUE)
                    .addComponent(labelChannelsPerSubbandCoherent, javax.swing.GroupLayout.DEFAULT_SIZE, 183, Short.MAX_VALUE)
                    .addComponent(timeIntegrationFactorCoherent, javax.swing.GroupLayout.DEFAULT_SIZE, 183, Short.MAX_VALUE)
                    .addComponent(labelWhichCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, 103, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(inputSubbandsPerFileCoherent)
                    .addComponent(inputChannelsPerSubbandCoherent)
                    .addComponent(inputTimeIntegrationFactorCoherent, javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputWhichCoherent, 0, 151, Short.MAX_VALUE))
                .addContainerGap())
        );
        CoherentStokesPanelLayout.setVerticalGroup(
            CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputWhichCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelWhichCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(8, 8, 8)
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputTimeIntegrationFactorCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(timeIntegrationFactorCoherent))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputChannelsPerSubbandCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelChannelsPerSubbandCoherent))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputSubbandsPerFileCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelSubbandsPerFileCoherent))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        IncoherentStokesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Incoherent Stokes", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        IncoherentStokesPanel.setToolTipText("Stokes");

        labelWhichIncoherent.setText("which");

        inputWhichIncoherent.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputWhichIncoherent.setEnabled(false);

        labelTimeIntegrationfactorCoherent.setText("time integration factor");

        inputTimeIntegrationFactorIncoherent.setEnabled(false);

        labelChannelsPerSubbandIncoherent.setText("# channels per subband");

        inputChannelsPerSubbandIncoherent.setEnabled(false);

        labelSubbandsPerFileIncoherent.setText("# subbands per file");

        inputSubbandsPerFileIncoherent.setEnabled(false);

        javax.swing.GroupLayout IncoherentStokesPanelLayout = new javax.swing.GroupLayout(IncoherentStokesPanel);
        IncoherentStokesPanel.setLayout(IncoherentStokesPanelLayout);
        IncoherentStokesPanelLayout.setHorizontalGroup(
            IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(IncoherentStokesPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelSubbandsPerFileIncoherent, javax.swing.GroupLayout.DEFAULT_SIZE, 172, Short.MAX_VALUE)
                    .addComponent(labelWhichIncoherent, javax.swing.GroupLayout.DEFAULT_SIZE, 172, Short.MAX_VALUE)
                    .addComponent(labelTimeIntegrationfactorCoherent)
                    .addComponent(labelChannelsPerSubbandIncoherent))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(inputSubbandsPerFileIncoherent)
                    .addComponent(inputChannelsPerSubbandIncoherent)
                    .addComponent(inputTimeIntegrationFactorIncoherent, javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputWhichIncoherent, 0, 150, Short.MAX_VALUE))
                .addContainerGap())
        );
        IncoherentStokesPanelLayout.setVerticalGroup(
            IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(IncoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputWhichIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelWhichIncoherent))
                .addGap(8, 8, 8)
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputTimeIntegrationFactorIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelTimeIntegrationfactorCoherent))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputChannelsPerSubbandIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelChannelsPerSubbandIncoherent))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputSubbandsPerFileIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelSubbandsPerFileIncoherent))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        inputCoherentDedisperseChannels.setText("CoherentDedisperseChannels");

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(CoherentStokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(IncoherentStokesPanel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(inputCoherentDedisperseChannels))
                .addContainerGap())
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addComponent(CoherentStokesPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(IncoherentStokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputCoherentDedisperseChannels))
        );

        CorrelatorPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Correlator", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        CorrelatorPanel.setToolTipText("Correlator");

        labelIntegrationTime.setText("integration Time:");

        inputIntegrationTime.setEnabled(false);

        jLabel2.setText("sec");

        inputChannelsPerSubbandCorrelator.setEnabled(false);

        inputBlocksPerIntegration.setEnabled(false);

        labelChannelsPerSubbandCorrelator.setText("# channels per subband");

        labelBlocksPerIntegration.setText("# blocks per integration");

        javax.swing.GroupLayout CorrelatorPanelLayout = new javax.swing.GroupLayout(CorrelatorPanel);
        CorrelatorPanel.setLayout(CorrelatorPanelLayout);
        CorrelatorPanelLayout.setHorizontalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CorrelatorPanelLayout.createSequentialGroup()
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, 143, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelBlocksPerIntegration)
                    .addComponent(labelChannelsPerSubbandCorrelator, javax.swing.GroupLayout.PREFERRED_SIZE, 197, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(8, 8, 8)
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputBlocksPerIntegration, javax.swing.GroupLayout.DEFAULT_SIZE, 54, Short.MAX_VALUE)
                    .addComponent(inputChannelsPerSubbandCorrelator, javax.swing.GroupLayout.DEFAULT_SIZE, 54, Short.MAX_VALUE)
                    .addComponent(inputIntegrationTime, javax.swing.GroupLayout.DEFAULT_SIZE, 54, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel2)
                .addGap(30, 30, 30))
        );
        CorrelatorPanelLayout.setVerticalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CorrelatorPanelLayout.createSequentialGroup()
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationTime)
                    .addComponent(jLabel2)
                    .addComponent(inputIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(16, 16, 16)
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelChannelsPerSubbandCorrelator)
                    .addComponent(inputChannelsPerSubbandCorrelator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputBlocksPerIntegration, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelBlocksPerIntegration))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addComponent(jPanel7, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(18, 18, 18)
                        .addComponent(CorrelatorPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(CorrelatorPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jPanel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );

        jPanel7.getAccessibleContext().setAccessibleDescription("");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(4471, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(2442, Short.MAX_VALUE))
        );

        jScrollPane1.setViewportView(jPanel2);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        switch (evt.getActionCommand()) {
            case "Apply":
                saveInput();
                break;
            case "Restore":
                restore();
                break;
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void inputOutputCorrelatedDataActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputCorrelatedDataActionPerformed

        checkSettings();     }//GEN-LAST:event_inputOutputCorrelatedDataActionPerformed

    private void inputOutputBeamFormedDataActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputBeamFormedDataActionPerformed

        inputOutputCoherentStokes.setEnabled(!inputOutputBeamFormedData.isSelected());         inputOutputCoherentStokes.setSelected(false);         checkSettings();     }//GEN-LAST:event_inputOutputBeamFormedDataActionPerformed

    private void inputOutputCoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputCoherentStokesActionPerformed

        inputOutputBeamFormedData.setEnabled(!inputOutputCoherentStokes.isSelected());         inputOutputBeamFormedData.setSelected(false);         checkSettings();     }//GEN-LAST:event_inputOutputCoherentStokesActionPerformed

    private void inputOutputIncoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputIncoherentStokesActionPerformed

        checkSettings();     }//GEN-LAST:event_inputOutputIncoherentStokesActionPerformed
    private jOTDBnode itsNode = null;
    private MainFrame itsMainFrame = null;
    private String itsTreeType = "";
    private JFileChooser fc = null;
    //Cobalt specific parameters
    private jOTDBnode itsDelayCompensation = null;
    private jOTDBnode itsCorrectBandPass = null;
    private jOTDBnode itsRealTime = null;
    private jOTDBnode itsBlockSize = null;
    private jOTDBnode itsCorrectClocks = null;
    // _Output params
    private jOTDBnode itsOutputCorrelatedData = null;
    private jOTDBnode itsOutputFilteredData = null;
    private jOTDBnode itsOutputBeamFormedData = null;
    private jOTDBnode itsOutputCoherentStokes = null;
    private jOTDBnode itsOutputIncoherentStokes = null;
    //Correlator
    private jOTDBnode itsIntegrationTime = null;
    private jOTDBnode itsChannelsPerSubbandCorrelator = null;
    private jOTDBnode itsBlocksPerIntegration = null;
    // BeamFormer
    private jOTDBnode itsCoherentDedisperseChannels = null;
    // Coherentstokes
    private jOTDBnode itsWhichCoherent = null;
    private jOTDBnode itsChannelsPerSubbandCoherent = null;
    private jOTDBnode itsTimeIntegrationFactorCoherent = null;
    private jOTDBnode itsSubbandsPerFileCoherent = null;
    // Incoherentstokes
    private jOTDBnode itsWhichIncoherent = null;
    private jOTDBnode itsChannelsPerSubbandIncoherent = null;
    private jOTDBnode itsTimeIntegrationFactorIncoherent = null;
    private jOTDBnode itsSubbandsPerFileIncoherent = null;

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel CoherentStokesPanel;
    private javax.swing.JPanel CorrelatorPanel;
    private javax.swing.JPanel IncoherentStokesPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTextField inputBlockSize;
    private javax.swing.JTextField inputBlocksPerIntegration;
    private javax.swing.JTextField inputChannelsPerSubbandCoherent;
    private javax.swing.JTextField inputChannelsPerSubbandCorrelator;
    private javax.swing.JTextField inputChannelsPerSubbandIncoherent;
    private javax.swing.JCheckBox inputCoherentDedisperseChannels;
    private javax.swing.JCheckBox inputCorrectBandPass;
    private javax.swing.JCheckBox inputCorrectClocks;
    private javax.swing.JCheckBox inputDelayCompensation;
    private javax.swing.JTextField inputIntegrationTime;
    private javax.swing.JCheckBox inputOutputBeamFormedData;
    private javax.swing.JCheckBox inputOutputCoherentStokes;
    private javax.swing.JCheckBox inputOutputCorrelatedData;
    private javax.swing.JCheckBox inputOutputFilteredData;
    private javax.swing.JCheckBox inputOutputIncoherentStokes;
    private javax.swing.JCheckBox inputRealTime;
    private javax.swing.JTextField inputSubbandsPerFileCoherent;
    private javax.swing.JTextField inputSubbandsPerFileIncoherent;
    private javax.swing.JTextField inputTimeIntegrationFactorCoherent;
    private javax.swing.JTextField inputTimeIntegrationFactorIncoherent;
    private javax.swing.JComboBox inputWhichCoherent;
    private javax.swing.JComboBox inputWhichIncoherent;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelBlockSize;
    private javax.swing.JLabel labelBlocksPerIntegration;
    private javax.swing.JLabel labelChannelsPerSubbandCoherent;
    private javax.swing.JLabel labelChannelsPerSubbandCorrelator;
    private javax.swing.JLabel labelChannelsPerSubbandIncoherent;
    private javax.swing.JLabel labelIntegrationTime;
    private javax.swing.JLabel labelSubbandsPerFileCoherent;
    private javax.swing.JLabel labelSubbandsPerFileIncoherent;
    private javax.swing.JLabel labelTimeIntegrationfactorCoherent;
    private javax.swing.JLabel labelWhichCoherent;
    private javax.swing.JLabel labelWhichIncoherent;
    private javax.swing.JLabel timeIntegrationFactorCoherent;
    // End of variables declaration//GEN-END:variables
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList = null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    @Override
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add(java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    @Override
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove(java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) {
            return;
        }
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i] == java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener) listeners[i + 1]).actionPerformed(event);
            }
        }
    }
}
