/* BeamDialog.java
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

package nl.astron.lofar.sas.otbcomponents;
import java.util.ArrayList;
import java.util.BitSet;
import javax.swing.JOptionPane;
import javax.swing.ListSelectionModel;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.objects.Beam;
import nl.astron.lofar.sas.otb.objects.TiedArrayBeam;
import nl.astron.lofar.sas.otb.util.tablemodels.TiedArrayBeamConfigurationTableModel;
import org.apache.log4j.Logger;


/**
 *
 * @created 11-02-2008, 13:35
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class BeamDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(BeamDialog.class);
    static String name = "BeamDialog";
    
    /** Creates new form BeanForm
     *
     * @param   parent                  Frame where this dialog belongs
     * @param   modal                   Should the Dialog be modal or not
     * @param   usedBeamlets            Bitset of all used beamlets(in case edit, the old ones have been xorred allready)
     * @param   selection               Vector of all Beam parameters
     * @param   directionTypeChoices    String with all possible choices + default for combobox
     * @param   edit                    indicates edit or add mode
     * @param   show                    indicates show only mode
     */
    public BeamDialog(java.awt.Frame parent, String treeType, boolean modal,BitSet usedBeamlets, Beam aBeam, boolean edit, boolean show ) {

        super(parent, modal);
        initComponents();
        LofarUtils.setPopupComboChoices(inputDirectionTypes,aBeam.getDirectionTypeChoices());
        itsBeam = aBeam;
        itsSavedBeamlets=(BitSet)usedBeamlets.clone();
        itsUsedBeamlets=(BitSet)usedBeamlets.clone();
        editting=edit;
        showing=show;
        itsTreeType = treeType;
        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }

        initialize();
    }
    
    // check if subbands are between 1 and 511
    private boolean checkSubbands(){
        String s = LofarUtils.expandedArrayString(inputSubbandList.getText());
        if (s.length() <=2) {
            return true;
        }

        if (s.startsWith("[")) {
            s = s.substring(1, s.length());
        }
        if (s.endsWith("]")) {
            s = s.substring(0, s.length() - 1);
        }
        String[] vS = s.split(",");


        for (int i=0; i< vS.length;i++) {
            if (Integer.valueOf(vS[i]) < 1 || Integer.valueOf(vS[i]) > 511 ) {
                return false;
            }
        }

        return true;
    }

    // check if Beamlets are spelled correctly and if Beamlets are not used by other Beams'
    private boolean checkBeamlets(){
        if (inputBeamletList.getText().length() <=2) {
            return true;
        }
        
        BitSet aBitSet = LofarUtils.beamletToBitSet(LofarUtils.expandedArrayString(inputBeamletList.getText()));
        if(itsUsedBeamlets.intersects(aBitSet)) {
            return false;
        } else {
            itsUsedBeamlets.or(aBitSet);
        }
        return true;
    }

    // check if nr of beamlets used and nr of subbands used are equal
    private boolean checkNrOfBeamletsAndSubbands() {
        if (LofarUtils.nrArrayStringElements(inputBeamletList.getText())!= LofarUtils.nrArrayStringElements(inputSubbandList.getText()) ) {
            return false;
        }

        return true;
    }
    
    private void initialize() {
        
        itsTABConfigurationTableModel = new TiedArrayBeamConfigurationTableModel();
        TABConfigurationPanel.setTableModel(itsTABConfigurationTableModel);
        TABConfigurationPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        TABConfigurationPanel.setColumnSize("dirtype",40);
        TABConfigurationPanel.setColumnSize("angle 1",40);
        TABConfigurationPanel.setColumnSize("angle 2",40);
        TABConfigurationPanel.setColumnSize("coordtype",30);
        TABConfigurationPanel.setColumnSize("disp.Measure",50);
        TABConfigurationPanel.setColumnSize("coherent",30);
        TABConfigurationPanel.repaint();


        inputDirectionTypes.setSelectedItem(itsBeam.getDirectionType());
        labelAngle1.setText(LofarUtils.directionTypeToAngle(itsBeam.getDirectionType(),1)+" :");
        labelAngle2.setText(LofarUtils.directionTypeToAngle(itsBeam.getDirectionType(),2)+" :");
        inputAngle1.setText(itsBeam.getAngle1());
        inputAngle2.setText(itsBeam.getAngle2());
        coordTypeChange.setSelectedItem(itsBeam.getCoordType());
        inputSubbandList.setText(itsBeam.getSubbandList());
        inputBeamletList.setText(itsBeam.getBeamletList());
        inputDuration.setText(itsBeam.getDuration());
        inputTarget.setText(itsBeam.getTarget());
        inputStartTime.setText(itsBeam.getStartTime());
        inputNrTabRings.setText(itsBeam.getNrTabRings());
        inputTabRingSize.setText(itsBeam.getTabRingSize());
        itsTiedArrayBeams=itsBeam.getTiedArrayBeams();
        itsTiedArrayBeams.trimToSize();
        // fill table with all entries
        boolean fillTable = itsTABConfigurationTableModel.fillTable(itsTreeType,itsBeam.getTiedArrayBeams(), true);
        // if showmode, only view, so disable all buttons
        if (showing) {
            enableAll(false);
            saveButton.setVisible(false);
            addTiedArrayBeamButton.setVisible(false);
            editTiedArrayBeamButton.setVisible(false);
            deleteTiedArrayBeamButton.setVisible(false);
        }
    }
    
    private void enableAll(boolean flag) {
        TABConfigurationPanel.setEnabled(flag);
        inputDirectionTypes.setEnabled(flag);
        inputAngle1.setEnabled(flag);
        inputAngle2.setEnabled(flag);
        coordTypeChange.setEnabled(flag);
        inputSubbandList.setEnabled(flag);
        inputBeamletList.setEnabled(flag);
        inputDuration.setEnabled(flag);
        inputTarget.setEnabled(flag);
        inputStartTime.setEnabled(flag);
        inputNrTabRings.setEnabled(flag);
        inputTabRingSize.setEnabled(flag);
    }
    
    public boolean hasChanged() {
        return isChanged;
    }
    
    public BitSet getBeamletList() {
        return itsUsedBeamlets;
    }
    
    public void setBorderTitle(String text) {
        TitledBorder aBorder=(TitledBorder)jPanel1.getBorder();
        aBorder.setTitle(text);
    }
    
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel1 = new javax.swing.JPanel();
        inputDirectionTypes = new javax.swing.JComboBox();
        labelAngle1 = new javax.swing.JLabel();
        inputAngle1 = new javax.swing.JTextField();
        labelAngle2 = new javax.swing.JLabel();
        inputAngle2 = new javax.swing.JTextField();
        labelSubbandList = new javax.swing.JLabel();
        inputSubbandList = new javax.swing.JTextField();
        labelBeamletList = new javax.swing.JLabel();
        inputBeamletList = new javax.swing.JTextField();
        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        labelDirectionTypes = new javax.swing.JLabel();
        coordTypeChange = new javax.swing.JComboBox();
        jPanel3 = new javax.swing.JPanel();
        TABConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        addTiedArrayBeamButton = new javax.swing.JButton();
        editTiedArrayBeamButton = new javax.swing.JButton();
        deleteTiedArrayBeamButton = new javax.swing.JButton();
        labelTabRingSize = new javax.swing.JLabel();
        inputTabRingSize = new javax.swing.JTextField();
        labelTarget = new javax.swing.JLabel();
        inputTarget = new javax.swing.JTextField();
        labelNrTabRings = new javax.swing.JLabel();
        inputNrTabRings = new javax.swing.JTextField();
        labelStartTime = new javax.swing.JLabel();
        inputStartTime = new javax.swing.JTextField();
        labelDuration = new javax.swing.JLabel();
        inputDuration = new javax.swing.JTextField();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Edit/Add Beam", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        inputDirectionTypes.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1" }));
        inputDirectionTypes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputDirectionTypesActionPerformed(evt);
            }
        });

        labelAngle1.setText("Angle 1:");

        labelAngle2.setText("Angle 2 :");

        labelSubbandList.setText("Subbands :");

        labelBeamletList.setText("Beamlets :");

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        saveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        saveButton.setText("Save");
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });

        labelDirectionTypes.setText("directionTypes :");

        coordTypeChange.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "rad", "deg", "hmsdms", "dmsdms" }));
        coordTypeChange.setToolTipText("set to alternative coordinates");
        coordTypeChange.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                coordTypeChangeActionPerformed(evt);
            }
        });

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "TiedArrayBeam Configuration", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        jPanel3.setPreferredSize(new java.awt.Dimension(200, 125));
        jPanel3.setRequestFocusEnabled(false);
        jPanel3.setVerifyInputWhenFocusTarget(false);

        TABConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                TABConfigurationPanelMouseClicked(evt);
            }
        });

        addTiedArrayBeamButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_add.gif"))); // NOI18N
        addTiedArrayBeamButton.setText("add tiedArrayBeam");
        addTiedArrayBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addTiedArrayBeamButtonActionPerformed(evt);
            }
        });

        editTiedArrayBeamButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif"))); // NOI18N
        editTiedArrayBeamButton.setText("edit tiedArrayBeam");
        editTiedArrayBeamButton.setEnabled(false);
        editTiedArrayBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editTiedArrayBeamButtonActionPerformed(evt);
            }
        });

        deleteTiedArrayBeamButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png"))); // NOI18N
        deleteTiedArrayBeamButton.setText("delete tiedArrayBeam");
        deleteTiedArrayBeamButton.setEnabled(false);
        deleteTiedArrayBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteTiedArrayBeamButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(addTiedArrayBeamButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(editTiedArrayBeamButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(deleteTiedArrayBeamButton))
                    .add(TABConfigurationPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1179, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel3Layout.createSequentialGroup()
                .add(TABConfigurationPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 201, Short.MAX_VALUE)
                .add(18, 18, 18)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(addTiedArrayBeamButton)
                    .add(editTiedArrayBeamButton)
                    .add(deleteTiedArrayBeamButton)))
        );

        labelTabRingSize.setText("TAB ring size:");

        labelTarget.setText("Target: ");

        labelNrTabRings.setText("# TAB rings:");

        labelStartTime.setText("startTime:");

        labelDuration.setText("Duration:");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel1Layout.createSequentialGroup()
                                .add(cancelButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 110, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(saveButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 90, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel1Layout.createSequentialGroup()
                                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                        .add(labelTabRingSize, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .add(labelDirectionTypes, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .add(labelAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .add(labelAngle1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 77, Short.MAX_VALUE))
                                    .add(labelDuration, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 77, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                .add(18, 18, 18)
                                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                    .add(inputDuration)
                                    .add(inputTabRingSize)
                                    .add(inputAngle2)
                                    .add(inputAngle1)
                                    .add(inputDirectionTypes, 0, 126, Short.MAX_VALUE)
                                    .add(inputNrTabRings))
                                .add(38, 38, 38)
                                .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 81, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(121, 121, 121)
                                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                    .add(org.jdesktop.layout.GroupLayout.LEADING, labelStartTime, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .add(org.jdesktop.layout.GroupLayout.LEADING, labelTarget, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 96, Short.MAX_VALUE))
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                    .add(inputTarget)
                                    .add(inputStartTime, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 196, Short.MAX_VALUE)))
                            .add(org.jdesktop.layout.GroupLayout.LEADING, labelNrTabRings, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 86, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addContainerGap())
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jPanel1Layout.createSequentialGroup()
                                .add(labelSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 87, Short.MAX_VALUE)
                                .add(18, 18, 18))
                            .add(jPanel1Layout.createSequentialGroup()
                                .add(labelBeamletList)
                                .add(45, 45, 45)))
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputBeamletList)
                            .add(inputSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1077, Short.MAX_VALUE))
                        .add(665, 665, 665))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 1201, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap(646, Short.MAX_VALUE))))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelDirectionTypes)
                    .add(inputDirectionTypes, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelAngle1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 14, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(inputAngle1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelAngle2)
                    .add(inputAngle2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelTarget, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 17, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(inputTarget, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelTabRingSize)
                    .add(inputTabRingSize, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelStartTime)
                    .add(inputStartTime, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelNrTabRings)
                    .add(inputNrTabRings, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelDuration, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 23, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(inputDuration, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelSubbandList)
                    .add(inputSubbandList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelBeamletList)
                    .add(inputBeamletList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(18, 18, 18)
                .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 267, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(94, 94, 94)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(cancelButton)
                    .add(saveButton))
                .addContainerGap())
        );

        jScrollPane1.setViewportView(jPanel1);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1253, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 671, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void saveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveButtonActionPerformed
        //check if input is correct
        checkChanged();
        if (hasChanged() && !checkBeamlets()) {
            if (JOptionPane.showConfirmDialog(this,"There is an error in the beamletList, some of them are allready in use by other Beams. continueing discards all changes. Continue?","Beamlet Error",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
                isChanged=false;
                setVisible(false);
                dispose();
            } else {
                itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
                return;
            }
        } else if (hasChanged() && !checkSubbands()) {
            if (JOptionPane.showConfirmDialog(this,"There is an error in the SubbandsList , Only subbands 1-511 can be used. continueing discards all changes. Continue?","Beamlet Error",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
                isChanged=false;
                setVisible(false);
                dispose();
            } else {
                itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
                return;
            }

        } else if (hasChanged() && !checkNrOfBeamletsAndSubbands() ) {
            if (JOptionPane.showConfirmDialog(this,"The number of beamlets and subbands differ.","Beamlet-subband amount differ  Error",JOptionPane.OK_CANCEL_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.OK_OPTION ) {
                setVisible(false);
                dispose();
            } else {
                itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
                return;
            }

        }else {
            setVisible(false);
            this.itsTiedArrayBeams= this.itsTABConfigurationTableModel.getTable();
            itsTiedArrayBeams.trimToSize();
            this.TABConfigurationPanel=null;
            this.itsTABConfigurationTableModel = null;
        }
}//GEN-LAST:event_saveButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        isChanged=false;
        itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
        setVisible(false);
        TABConfigurationPanel=null;
        itsTABConfigurationTableModel = null;

}//GEN-LAST:event_cancelButtonActionPerformed

    private void coordTypeChangeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_coordTypeChangeActionPerformed

        if (evt.getActionCommand().equals("comboBoxChanged")) {
            String ch = coordTypeChange.getSelectedItem().toString();
            String tmpch = ch;
            String tmpcoord = itsBeam.getCoordType();
            if ((inputAngle1.getText().isEmpty() && inputAngle1.getText().isEmpty()) || ch.equals(itsBeam.getCoordType())) {
                return;
            }
            try {
            if (!inputAngle1.getText().isEmpty()) {
                    switch (ch) {
                        case "dmsdms":
                            tmpch = "dms";
                            break;
                        case "hmsdms":
                            tmpch = "hms";
                            break;
                    }
                    switch (itsBeam.getCoordType()) {
                        case "hmsdms":
                            tmpcoord="hms";
                            break;
                        case "dmsdms":
                            tmpcoord="dms";
                            break;
                    }
                inputAngle1.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle1.getText()));
            }
            if (!inputAngle2.getText().isEmpty()) {
                if (ch.equals("dmsdms") || ch.equals("hmsdms")) {
                    tmpch = "dms";
                }
                if (itsBeam.getCoordType().equals("hmsdms") || itsBeam.getCoordType().equals("dmsdms")) {
                    tmpcoord="dms";
                }
                inputAngle2.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle2.getText()));
            }
            itsBeam.setCoordType(ch);
            } catch (NumberFormatException ex) {
                String aS="Error in angle input format :" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        }
    }//GEN-LAST:event_coordTypeChangeActionPerformed

    private void inputDirectionTypesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputDirectionTypesActionPerformed
        if (inputDirectionTypes.getSelectedItem() == null) return;
        String type = inputDirectionTypes.getSelectedItem().toString();
        labelAngle1.setText(LofarUtils.directionTypeToAngle(type,1)+" :");
        labelAngle2.setText(LofarUtils.directionTypeToAngle(type,2)+" :");
    }//GEN-LAST:event_inputDirectionTypesActionPerformed

    private void TABConfigurationPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_TABConfigurationPanelMouseClicked
        editTiedArrayBeamButton.setEnabled(true);
        deleteTiedArrayBeamButton.setEnabled(true);
    }//GEN-LAST:event_TABConfigurationPanelMouseClicked

    private void addTiedArrayBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addTiedArrayBeamButtonActionPerformed
        editTiedArrayBeam = false;
        addTiedArrayBeam();
    }//GEN-LAST:event_addTiedArrayBeamButtonActionPerformed

    private void editTiedArrayBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_editTiedArrayBeamButtonActionPerformed
        editTiedArrayBeam = true;
        addTiedArrayBeam();   
    }//GEN-LAST:event_editTiedArrayBeamButtonActionPerformed

    private void deleteTiedArrayBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteTiedArrayBeamButtonActionPerformed
        deleteTiedArrayBeam(); 
    }//GEN-LAST:event_deleteTiedArrayBeamButtonActionPerformed
        
    private void checkChanged() {
        if (!itsBeam.getDirectionType().equals(inputDirectionTypes.getSelectedItem().toString())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getAngle1().equals(inputAngle1.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getAngle2().equals(inputAngle2.getText())) {
            isChanged=true;
            return;
        }

        if (!itsBeam.getCoordType().equals(coordTypeChange.getSelectedItem().toString())) {
            isChanged=true;
            return;
        }

        if (!itsBeam.getSubbandList().equals(inputSubbandList.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getBeamletList().equals(inputBeamletList.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getDuration().equals(inputDuration.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getTarget().equals(inputTarget.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getStartTime().equals(inputStartTime.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getNrTabRings().equals(inputNrTabRings.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeam.getTabRingSize().equals(inputTabRingSize.getText())) {
            isChanged=true;
            return;
        }
        if (itsTABConfigurationTableModel.changed() ) {
            isChanged=true;
            itsBeam.setNrTiedArrayBeams(Integer.toString(TABConfigurationPanel.getTableModel().getRowCount()));
            return;
        }

        return;

    }
    
    public Beam getBeam() {
        itsBeam.setTarget(inputTarget.getText());
        itsBeam.setDirectionType(inputDirectionTypes.getSelectedItem().toString());
        itsBeam.setAngle1(inputAngle1.getText());
        itsBeam.setAngle2(inputAngle2.getText());
        itsBeam.setCoordType(coordTypeChange.getSelectedItem().toString());
        itsBeam.setDuration(inputDuration.getText());
        itsBeam.setStartTime(inputStartTime.getText());
        itsBeam.setSubbandList(LofarUtils.compactedArrayString(inputSubbandList.getText()));
        itsBeam.setBeamletList(LofarUtils.compactedArrayString(inputBeamletList.getText()));
        itsBeam.setNrTabRings(inputNrTabRings.getText());
        itsBeam.setTabRingSize(inputTabRingSize.getText());
        itsBeam.setNrTiedArrayBeams(Integer.toString(itsTiedArrayBeams.size()-offset));
        itsBeam.setTiedArrayBeams(itsTiedArrayBeams);
        return itsBeam;
    }
    
    private void deleteTiedArrayBeam() {
        itsSelectedRow = TABConfigurationPanel.getSelectedRow();
        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this TAB ?","Delete TAB",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            // if removed then the old Beamlets's should be removed form the checklist also
            if (itsSelectedRow > -1) {
                itsTABConfigurationTableModel.removeRow(itsSelectedRow);
                // No selection anymore after delete, so buttons disabled again
                this.editTiedArrayBeamButton.setEnabled(false);
                this.deleteTiedArrayBeamButton.setEnabled(false);
            }
        } 
    }

      private void addTiedArrayBeam() {
     
        itsSelectedRow=-1;
        itsSelectedRow = TABConfigurationPanel.getSelectedRow();
        // set selection to defaults.
        TiedArrayBeam selection = itsTiedArrayBeams.get(0);
        if (editTiedArrayBeam) {
            selection = itsTABConfigurationTableModel.getSelection(itsSelectedRow);
        }
        itsTABDialog = new TiedArrayBeamDialog(itsMainFrame,true,selection.clone(),editTiedArrayBeam);
        itsTABDialog.setLocationRelativeTo(this);
        if (editTiedArrayBeam) {
            itsTABDialog.setBorderTitle("edit TiedArrayBeam");
        } else {
            itsTABDialog.setBorderTitle("add new TiedArrayBeam");            
        }
        itsTABDialog.setVisible(true);
        
        // check if something has changed 
        if (itsTABDialog.hasChanged()) {
            TiedArrayBeam newTAB = itsTABDialog.getTiedArrayBeam();
            // check if we are editting an entry or adding a new entry
            if (editTiedArrayBeam) {
                itsTABConfigurationTableModel.updateRow(newTAB,itsSelectedRow);
                // set editting = false
                editTiedArrayBeam=false;
            } else {            
                boolean succes = itsTABConfigurationTableModel.addRow(newTAB);
            }
        }
        
        this.editTiedArrayBeamButton.setEnabled(false);
        this.deleteTiedArrayBeamButton.setEnabled(false);
    }
     
    
    private MainFrame itsMainFrame = null;
    private boolean   isChanged=false;
    private Beam      itsBeam;
    private ArrayList<TiedArrayBeam> itsTiedArrayBeams;
    private int       itsSelectedRow = -1;
    private int       offset = 1;

    
    private boolean   editTiedArrayBeam = false;
    private boolean   editting          = false;
    private boolean   showing           = false;
    private BitSet    itsUsedBeamlets   = null;
    private BitSet    itsSavedBeamlets  = null;
    private String    itsTreeType       = null;
    
    private TiedArrayBeamConfigurationTableModel itsTABConfigurationTableModel = null;
    private TiedArrayBeamDialog                  itsTABDialog = null;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.TablePanel TABConfigurationPanel;
    private javax.swing.JButton addTiedArrayBeamButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox coordTypeChange;
    private javax.swing.JButton deleteTiedArrayBeamButton;
    private javax.swing.JButton editTiedArrayBeamButton;
    private javax.swing.JTextField inputAngle1;
    private javax.swing.JTextField inputAngle2;
    private javax.swing.JTextField inputBeamletList;
    private javax.swing.JComboBox inputDirectionTypes;
    private javax.swing.JTextField inputDuration;
    private javax.swing.JTextField inputNrTabRings;
    private javax.swing.JTextField inputStartTime;
    private javax.swing.JTextField inputSubbandList;
    private javax.swing.JTextField inputTabRingSize;
    private javax.swing.JTextField inputTarget;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelAngle1;
    private javax.swing.JLabel labelAngle2;
    private javax.swing.JLabel labelBeamletList;
    private javax.swing.JLabel labelDirectionTypes;
    private javax.swing.JLabel labelDuration;
    private javax.swing.JLabel labelNrTabRings;
    private javax.swing.JLabel labelStartTime;
    private javax.swing.JLabel labelSubbandList;
    private javax.swing.JLabel labelTabRingSize;
    private javax.swing.JLabel labelTarget;
    private javax.swing.JButton saveButton;
    // End of variables declaration//GEN-END:variables
    
}
