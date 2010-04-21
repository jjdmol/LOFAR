/*
 * ImagerPanel.java
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
import java.util.Enumeration;
import java.util.Vector;
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
 * Panel for Imager specific configuration
 *
 * @author  Coolen
 *
 * Created on 22 januari 2008, 9:30
 *
 * @version $Id$
 */
public class ImagerPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(ImagerPanel.class);    
    static String name = "ImagerPanel";
    
   
     /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public ImagerPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form ImagerPanel */
    public ImagerPanel() {
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
        itsNode=(jOTDBnode)anObject;
        jOTDBparam aParam=null;
        try {
         
            //we need to get all the childs from this node.    
            Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("Images")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Gridder")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            return;
        }
        
        initPanel();
    }
    
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new ImagerPanel();
    }
    
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
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
        // For template trees
        } else if (itsTreeType.equals("VItemplate")) {
                
        }
        
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
         if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.debug("Create ParSet File");
            int aTreeID=itsMainFrame.getSharedVars().getTreeID();
            if (fc == null) {
                fc = new JFileChooser();
                fc.setApproveButtonText("Apply");
            }
            // try to get a new filename to write the parsetfile to
            if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                try {
                    File aFile = fc.getSelectedFile();
                    
                    // create filename that can be used at the remote site    
                    String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSet";
                    
                    // write the parset
                    OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName,2,false); 
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);

                    BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile));
                    output.write(dldata,0,dldata.length);
                    output.flush();
                    output.close();
                    logger.debug("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    logger.debug("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                }
            }
        }       
    }
    
     /** 
     * Helper method that retrieves the child nodes for a given jOTDBnode, 
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            logger.debug("Error during retrieveAndDisplayChildDataForNode: "+ ex);
            return;
        }
    }
    
    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {

        // Generic Observation
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        /* Set's the different fields in the GUI */
        
        // Generic Imager
        if (aParam==null) {
            return;
        }
        logger.debug("setField for: "+ aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());                
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getParam: "+ ex);
        }        

        // Imager Specific parameters    
        if(parentName.equals("Imager")){        
            if (aKeyName.equals("dataset")) {
                inputDataset.setToolTipText(aParam.description);
               itsDataSet=aNode;
                if (isRef && aParam != null) {
                    this.inputDatasetDeRef.setVisible(true);
                    inputDataset.setText(aNode.limits);
                    inputDatasetDeRef.setText(aParam.limits);
                } else {
                    this.inputDatasetDeRef.setVisible(false);
                    inputDatasetDeRef.setText("");
                    inputDataset.setText(aNode.limits);
                }
            } else if (aKeyName.equals("datacolumn")) {        
                inputDatacolumn.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputDatacolumn,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputDatacolumn.setSelectedItem(aNode.limits);            
                }
                itsDataColumn=aNode;
            } else if (aKeyName.equals("minUV")) {
                inputMinUV.setToolTipText(aParam.description);
               itsMinUV=aNode;
                if (isRef && aParam != null) {
                    inputMinUV.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputMinUV.setText(aNode.limits);
                }
            }
        // Gridder    
        } else if(parentName.equals("Gridder")){
            if (aKeyName.equals("type")) {     
                inputType.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputType,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputType.setSelectedItem(aNode.limits);            
                }
                itsType=aNode;
            } else if (aKeyName.equals("wmax")) {
                inputWmax.setToolTipText(aParam.description);
                itsWMax=aNode;
                if (isRef && aParam != null) {
                    inputWmax.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputWmax.setText(aNode.limits);
                }        
            } else if (aKeyName.equals("nwplanes")) {
                inputNWPlanes.setToolTipText(aParam.description);
                itsNWPlanes=aNode;
                if (isRef && aParam != null) {
                    inputNWPlanes.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNWPlanes.setText(aNode.limits);
                }        
            } else if (aKeyName.equals("oversample")) {
                inputOversample.setToolTipText(aParam.description);
                itsOverSample=aNode;
                if (isRef && aParam != null) {
                    inputOversample.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputOversample.setText(aNode.limits);
                }        
            } else if (aKeyName.equals("cutoff")) {
                inputCutOff.setToolTipText(aParam.description);
                itsCutOff=aNode;
                if (isRef && aParam != null) {
                    inputCutOff.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputCutOff.setText(aNode.limits);
                }        
            } else if (aKeyName.equals("nfacets")) {
                inputNFacets.setToolTipText(aParam.description);
                itsNFacets=aNode;
                if (isRef && aParam != null) {
                    inputNFacets.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNFacets.setText(aNode.limits);
                }        
            } 
        // Gridder    
        } else if(parentName.equals("Images")){
            if (aKeyName.equals("stokes")) {
                inputUseI.setToolTipText(aParam.description);
                inputUseQ.setToolTipText(aParam.description);
                inputUseU.setToolTipText(aParam.description);
                inputUseV.setToolTipText(aParam.description);
                itsStokes=aNode;
                setStokes();
            } else if (aKeyName.equals("shape")) {
                inputShape.setToolTipText(aParam.description);
                itsShape=aNode;
                if (isRef && aParam != null) {
                    inputShape.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputShape.setText(aNode.limits);
                }        
            } else if (aKeyName.equals("cellSize")) {
                inputCellSize.setToolTipText(aParam.description);
                itsCellSize=aNode;
                if (isRef && aParam != null) {
                    inputCellSize.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputCellSize.setText(aNode.limits);
                }        
            }
        }
    }
    
    
    /** set checkboxes according to stokes array
     */
    private void setStokes() {
        boolean usedI = false;
        boolean usedQ = false;
        boolean usedU = false;
        boolean usedV = false;
        if (itsStokes.limits.contains("I") ) {
            usedI=true;
        }
        if (itsStokes.limits.contains("Q") ) {
            usedQ=true;
        }
        if (itsStokes.limits.contains("U") ) {
            usedU=true;
        }
        if (itsStokes.limits.contains("V") ) {
            usedV=true;
        }
        inputUseI.setSelected(usedI);
        inputUseQ.setSelected(usedQ);
        inputUseU.setSelected(usedU);
        inputUseV.setSelected(usedV);
    }
    
    /** set checkboxes according to stokes array
     */
    private String getStokes() {
        String stokes="[";
        boolean first=true;
        if (inputUseI.isSelected() ) {
            stokes+="I";
            first=false;
        }
        if (inputUseQ.isSelected() ) {
            if (!first) {
                stokes +=",";
            } else {
                first=false;
            }
            stokes+="Q";
        }
        if (inputUseU.isSelected() ) {
            if (!first) {
                stokes +=",";
            } else {
                first=false;
            }
            stokes+="U";
        }
        if (inputUseV.isSelected() ) {
            if (!first) {
                stokes +=",";
            } else {
                first=false;
            }
            stokes+="V";
        }
        stokes+="]";
        return stokes;
    }
    
    
    /** Restore original Values in  panel
     */
    private void restore() {
        //Imager specific parameters
        inputDataset.setText(itsDataSet.limits);
        inputDatacolumn.setSelectedItem(itsDataColumn.limits);
        inputMinUV.setText(itsMinUV.limits);

        //Gridder  specific parameters
        inputType.setSelectedItem(itsType.limits);
        inputWmax.setText(itsWMax.limits);
        inputNWPlanes.setText(itsNWPlanes.limits);
        inputOversample.setText(itsOverSample.limits);
        inputCutOff.setText(itsCutOff.limits);
        inputNFacets.setText(itsNFacets.limits);

        //Images specific parameters
        setStokes();
        inputShape.setText(itsShape.limits);
        inputCellSize.setText(itsCellSize.limits);
    }
    
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.addButton("Apply");
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
        

         if (itsNode != null) {
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false);
                itsTreeType=OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                logger.debug("ImagerPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }         } else {
            logger.debug("ERROR:  no node given");
        }
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
            logger.debug("Error: saveNode failed : " + ex);
        } 
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
    }
    
    private void saveInput() {
        //Imager specific parameters       
        if (itsDataSet != null && !inputDataset.getText().equals(itsDataSet.limits)) {
            itsDataSet.limits = inputDataset.getText();
            saveNode(itsDataSet);
        }        
        if (itsDataColumn != null && !inputDatacolumn.getSelectedItem().toString().equals(itsDataColumn.limits)) {  
            itsDataColumn.limits = inputDatacolumn.getSelectedItem().toString();
            saveNode(itsDataColumn);
        }        
        if (itsMinUV != null && !inputMinUV.getText().equals(itsMinUV.limits)) {
            itsMinUV.limits = inputMinUV.getText();
            saveNode(itsMinUV);
        }        
    
        //Gridder  specific parameters
        if (itsType != null && !inputType.getSelectedItem().toString().equals(itsType.limits)) {  
            itsType.limits = inputType.getSelectedItem().toString();
            saveNode(itsType);
        }        
        if (itsWMax != null && !inputWmax.getText().equals(itsWMax.limits)) {
            itsWMax.limits = inputWmax.getText();
            saveNode(itsWMax);
        }        
        if (itsNWPlanes != null && !inputNWPlanes.getText().equals(itsNWPlanes.limits)) {
            itsNWPlanes.limits = inputNWPlanes.getText();
            saveNode(itsNWPlanes);
        }        
        if (itsOverSample != null && !inputOversample.getText().equals(itsOverSample.limits)) {
            itsOverSample.limits = inputOversample.getText();
            saveNode(itsOverSample);
        }        
        if (itsCutOff != null && !inputCutOff.getText().equals(itsCutOff.limits)) {
            itsCutOff.limits = inputCutOff.getText();
            saveNode(itsCutOff);
        }        
        if (itsNFacets != null && !inputNFacets.getText().equals(itsNFacets.limits)) {
            itsNFacets.limits = inputNFacets.getText();
            saveNode(itsNFacets);
        }        
        
        //Images specific parameters
        if (itsStokes != null && !getStokes().equals(itsStokes.limits)) {  
            itsStokes.limits = getStokes();
            saveNode(itsStokes);
        }        
        if (itsShape != null && !inputShape.getText().equals(itsNFacets.limits)) {
            itsShape.limits = inputShape.getText();
            saveNode(itsShape);
        }        
        if (itsCellSize != null && !inputCellSize.getText().equals(itsCellSize.limits)) {
            itsCellSize.limits = inputCellSize.getText();
            saveNode(itsCellSize);
        }        
    }
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        jPanel3 = new javax.swing.JPanel();
        labelDataset = new javax.swing.JLabel();
        inputDataset = new javax.swing.JTextField();
        labelDatacolumn = new javax.swing.JLabel();
        inputDatacolumn = new javax.swing.JComboBox();
        labelMinUV = new javax.swing.JLabel();
        inputMinUV = new javax.swing.JTextField();
        inputDatasetDeRef = new javax.swing.JTextField();
        jPanel4 = new javax.swing.JPanel();
        labelType = new javax.swing.JLabel();
        inputType = new javax.swing.JComboBox();
        labelWmax = new javax.swing.JLabel();
        inputWmax = new javax.swing.JTextField();
        labelNWPlanes = new javax.swing.JLabel();
        inputNWPlanes = new javax.swing.JTextField();
        labelOversample = new javax.swing.JLabel();
        inputOversample = new javax.swing.JTextField();
        labelCutOff = new javax.swing.JLabel();
        inputCutOff = new javax.swing.JTextField();
        labelNFacets = new javax.swing.JLabel();
        inputNFacets = new javax.swing.JTextField();
        jPanel5 = new javax.swing.JPanel();
        inputUseI = new javax.swing.JCheckBox();
        inputUseQ = new javax.swing.JCheckBox();
        inputUseU = new javax.swing.JCheckBox();
        inputUseV = new javax.swing.JCheckBox();
        labelShape = new javax.swing.JLabel();
        inputShape = new javax.swing.JTextField();
        labelCellSize = new javax.swing.JLabel();
        inputCellSize = new javax.swing.JTextField();
        labelStokes = new javax.swing.JLabel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jPanel1.setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));
        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Imager Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));
        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Imager", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0)));
        labelDataset.setText("DataSet:");

        labelDatacolumn.setText("DataColumn:");

        inputDatacolumn.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        labelMinUV.setText("MinUV:");

        inputDatasetDeRef.setEditable(false);
        inputDatasetDeRef.setToolTipText("dereffed value");

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelDatacolumn, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelDataset, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelMinUV, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(17, 17, 17)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputMinUV, javax.swing.GroupLayout.PREFERRED_SIZE, 220, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(inputDatacolumn, javax.swing.GroupLayout.PREFERRED_SIZE, 217, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(inputDataset, javax.swing.GroupLayout.PREFERRED_SIZE, 248, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputDatasetDeRef, javax.swing.GroupLayout.DEFAULT_SIZE, 382, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelDataset)
                    .addComponent(inputDataset, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(inputDatasetDeRef, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelDatacolumn)
                    .addComponent(inputDatacolumn, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputMinUV, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelMinUV))
                .addGap(17, 17, 17))
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Gridder", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0)));
        labelType.setText("Type:");

        inputType.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        labelWmax.setText("Wmax:");

        labelNWPlanes.setText("NWPlanes:");

        labelOversample.setText("Oversample:");

        labelCutOff.setText("CutOff:");

        labelNFacets.setText("NFacets");

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(labelType, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE)
                    .addComponent(labelWmax, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE)
                    .addComponent(labelNWPlanes, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE))
                .addGap(20, 20, 20)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputType, javax.swing.GroupLayout.Alignment.TRAILING, 0, 213, Short.MAX_VALUE)
                    .addComponent(inputWmax, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 213, Short.MAX_VALUE)
                    .addComponent(inputNWPlanes, javax.swing.GroupLayout.DEFAULT_SIZE, 213, Short.MAX_VALUE))
                .addGap(115, 115, 115)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelNFacets, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE)
                    .addComponent(labelCutOff, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE)
                    .addComponent(labelOversample, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE))
                .addGap(21, 21, 21)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(inputNFacets)
                    .addComponent(inputCutOff)
                    .addComponent(inputOversample, javax.swing.GroupLayout.DEFAULT_SIZE, 218, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelType)
                    .addComponent(inputType, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelOversample)
                    .addComponent(inputOversample, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelWmax)
                    .addComponent(inputWmax, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelCutOff)
                    .addComponent(inputCutOff, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputNWPlanes, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelNWPlanes)
                    .addComponent(labelNFacets)
                    .addComponent(inputNFacets, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(29, Short.MAX_VALUE))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Images", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0)));
        inputUseI.setText("use I");
        inputUseI.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputUseI.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputUseQ.setText("use Q");
        inputUseQ.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputUseQ.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputUseU.setText("use U");
        inputUseU.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputUseU.setMargin(new java.awt.Insets(0, 0, 0, 0));

        inputUseV.setText("use V");
        inputUseV.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputUseV.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelShape.setText("Shape:");

        labelCellSize.setText("CellSize:");

        labelStokes.setText("Stokes:");

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelStokes, javax.swing.GroupLayout.DEFAULT_SIZE, 57, Short.MAX_VALUE)
                    .addComponent(labelShape, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelCellSize, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(23, 23, 23)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel5Layout.createSequentialGroup()
                        .addComponent(inputUseI)
                        .addGap(19, 19, 19)
                        .addComponent(inputUseQ)
                        .addGap(17, 17, 17)
                        .addComponent(inputUseU)
                        .addGap(14, 14, 14)
                        .addComponent(inputUseV))
                    .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(inputCellSize, javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(inputShape, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 215, Short.MAX_VALUE)))
                .addContainerGap(421, Short.MAX_VALUE))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputUseI)
                    .addComponent(inputUseQ)
                    .addComponent(inputUseU)
                    .addComponent(inputUseV)
                    .addComponent(labelStokes))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputShape, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelShape))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputCellSize, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelCellSize))
                .addContainerGap())
        );

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanel5, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(jPanel4, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(jPanel3, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(500, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(27, 27, 27)
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(24, 24, 24)
                .addComponent(jPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(178, 178, 178))
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
        if(evt.getActionCommand().equals("Apply")) {
            saveInput();
        } else if(evt.getActionCommand().equals("Restore")) {
            restore();
        }

    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private jOTDBnode    itsNode = null;
    private MainFrame    itsMainFrame;
    private String       itsTreeType="";
    private JFileChooser fc          = null; 
    
    //Imager specific parameters
    private jOTDBnode itsDataSet=null;
    private jOTDBnode itsDataColumn=null;
    private jOTDBnode itsMinUV=null;

    //Gridder  specific parameters
    private jOTDBnode itsType=null;
    private jOTDBnode itsWMax=null;
    private jOTDBnode itsNWPlanes=null;
    private jOTDBnode itsOverSample=null;
    private jOTDBnode itsCutOff=null;
    private jOTDBnode itsNFacets=null;

    //Images specific parameters
    private jOTDBnode itsStokes=null;
    private jOTDBnode itsShape=null;
    private jOTDBnode itsCellSize=null;

    
    
    // Observation Specific parameters    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTextField inputCellSize;
    private javax.swing.JTextField inputCutOff;
    private javax.swing.JComboBox inputDatacolumn;
    private javax.swing.JTextField inputDataset;
    private javax.swing.JTextField inputDatasetDeRef;
    private javax.swing.JTextField inputMinUV;
    private javax.swing.JTextField inputNFacets;
    private javax.swing.JTextField inputNWPlanes;
    private javax.swing.JTextField inputOversample;
    private javax.swing.JTextField inputShape;
    private javax.swing.JComboBox inputType;
    private javax.swing.JCheckBox inputUseI;
    private javax.swing.JCheckBox inputUseQ;
    private javax.swing.JCheckBox inputUseU;
    private javax.swing.JCheckBox inputUseV;
    private javax.swing.JTextField inputWmax;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelCellSize;
    private javax.swing.JLabel labelCutOff;
    private javax.swing.JLabel labelDatacolumn;
    private javax.swing.JLabel labelDataset;
    private javax.swing.JLabel labelMinUV;
    private javax.swing.JLabel labelNFacets;
    private javax.swing.JLabel labelNWPlanes;
    private javax.swing.JLabel labelOversample;
    private javax.swing.JLabel labelShape;
    private javax.swing.JLabel labelStokes;
    private javax.swing.JLabel labelType;
    private javax.swing.JLabel labelWmax;
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
}
