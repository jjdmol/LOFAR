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
import java.util.BitSet;
import javax.swing.JOptionPane;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
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
     */
    public BeamDialog(java.awt.Frame parent, boolean modal,BitSet usedBeamlets, String[] selection, String directionTypeChoices, boolean edit ) {

        super(parent, modal);
        initComponents();
        LofarUtils.setPopupComboChoices(inputDirectionTypes,directionTypeChoices);
        itsDirectionType=selection[0];
        itsTarget=selection[1];
        itsAngle1=selection[2];
        itsAngle2=selection[3];
        itsCoordType=selection[4];
        itsDuration=selection[5];
        itsMaxDur=selection[6];
        itsStartTime=selection[7];
        itsSubbandList=selection[8];
        itsBeamletList=selection[9];
        itsMomID=selection[10];
        editting=edit;
        itsSavedBeamlets=(BitSet)usedBeamlets.clone();
        itsUsedBeamlets=(BitSet)usedBeamlets.clone();
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

        inputDirectionTypes.setSelectedItem(itsDirectionType);
        labelAngle1.setText(LofarUtils.directionTypeToAngle(itsDirectionType,1)+" :");
        labelAngle2.setText(LofarUtils.directionTypeToAngle(itsDirectionType,2)+" :");
        inputAngle1.setText(itsAngle1);
        inputAngle2.setText(itsAngle2);
        coordTypeChange.setSelectedItem(itsCoordType);
        if (!itsMaxDur.equals("Missing")) {
            inputMaxDur.setVisible(true);
            inputMaxDur.setSelected(LofarUtils.StringToBoolean(itsMaxDur));
        } else {
            inputMaxDur.setVisible(false);
        }
        inputSubbandList.setText(itsSubbandList);
        inputBeamletList.setText(itsBeamletList);

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
        inputMaxDur = new javax.swing.JCheckBox();

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

        inputMaxDur.setText("Maximize Duration");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(labelAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(labelAngle1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(labelDirectionTypes, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 133, Short.MAX_VALUE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputAngle1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 185, Short.MAX_VALUE)
                            .add(inputDirectionTypes, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 117, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(inputAngle2)
                            .add(inputMaxDur))
                        .add(28, 28, 28)
                        .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(cancelButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 110, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(saveButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 90, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(labelSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(labelBeamletList))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 676, Short.MAX_VALUE)
                            .add(inputBeamletList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 676, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                .addContainerGap(111, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
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
                            .add(inputAngle2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(37, 37, 37)
                        .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(inputMaxDur)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelSubbandList)
                    .add(inputSubbandList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelBeamletList)
                    .add(inputBeamletList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(53, 53, 53)
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
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 875, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 294, Short.MAX_VALUE)
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
            dispose();
        }
}//GEN-LAST:event_saveButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        isChanged=false;
        itsUsedBeamlets=(BitSet)itsSavedBeamlets.clone();
        setVisible(false);
        dispose();
}//GEN-LAST:event_cancelButtonActionPerformed

    private void coordTypeChangeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_coordTypeChangeActionPerformed

        if (evt.getActionCommand().equals("comboBoxChanged")) {
            String ch = coordTypeChange.getSelectedItem().toString();
            String tmpch = ch;
            String tmpcoord = itsCoordType;
            if ((inputAngle1.getText().isEmpty() && inputAngle1.getText().isEmpty()) || ch.equals(itsCoordType)) {
                return;
            }
            try {
            if (!inputAngle1.getText().isEmpty()) {
                if (ch.equals("dmsdms")) {
                    tmpch = "dms";
                } else if (ch.equals("hmsdms")) {
                    tmpch = "hms";
                }
                if (itsCoordType.equals("hmsdms")) {
                    tmpcoord="hms";
                } else if (itsCoordType.equals("dmsdms")) {
                    tmpcoord="dms";
                }
                inputAngle1.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle1.getText()));
            }
            if (!inputAngle2.getText().isEmpty()) {
                if (ch.equals("dmsdms") || ch.equals("hmsdms")) {
                    tmpch = "dms";
                }
                if (itsCoordType.equals("hmsdms") || itsCoordType.equals("dmsdms")) {
                    tmpcoord="dms";
                }
                inputAngle2.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle2.getText()));
            }
            itsCoordType = ch;
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
        
    private void checkChanged() {
        if (!itsDirectionType.equals(inputDirectionTypes.getSelectedItem().toString())) {
            isChanged=true;
            return;
        }
        if (!itsAngle1.equals(inputAngle1.getText())) {
            isChanged=true;
            return;
        }
        if (!itsAngle2.equals(inputAngle2.getText())) {
            isChanged=true;
            return;
        }

        if (!itsCoordType.equals(coordTypeChange.getSelectedItem().toString())) {
            isChanged=true;
            return;
        }

        if (inputMaxDur.isVisible()) {
            if (!itsMaxDur.equals(LofarUtils.BooleanToString(inputMaxDur.isSelected()))) {
                isChanged=true;
            }
        }

        if (!itsSubbandList.equals(inputSubbandList.getText())) {
            isChanged=true;
            return;
        }
        if (!itsBeamletList.equals(inputBeamletList.getText())) {
            isChanged=true;
            return;
        }
    }
    
    public String[] getBeam() {
        String aS= "Missing";
        if(!itsMaxDur.equals("Missing")) {
            aS=LofarUtils.BooleanToString(inputMaxDur.isSelected());
        }
        String[] newRow = {inputDirectionTypes.getSelectedItem().toString(),
        itsTarget,
        inputAngle1.getText(),
        inputAngle2.getText(),
        coordTypeChange.getSelectedItem().toString(),
        itsDuration,
        aS,
        itsStartTime,
        LofarUtils.compactedArrayString(inputSubbandList.getText()),
        LofarUtils.compactedArrayString(inputBeamletList.getText()),
        itsMomID
        };
        
        return newRow;
    }
        
    
    private MainFrame itsMainFrame = null;
    private boolean isChanged=false;
    
    private String    itsDirectionType = "";
    private String    itsTarget         = "";
    private String    itsAngle1         = "";
    private String    itsAngle2         = "";
    private String    itsCoordType      = "";
    private String    itsDuration       = "";
    private String    itsMaxDur         = "";
    private String    itsStartTime      = "";
    private String    itsSubbandList    = "";
    private String    itsBeamletList    = "";
    private String    itsMomID          = "";
    private boolean   editting          = false;
    private BitSet    itsUsedBeamlets   = null;
    private BitSet    itsSavedBeamlets  = null;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox coordTypeChange;
    private javax.swing.JTextField inputAngle1;
    private javax.swing.JTextField inputAngle2;
    private javax.swing.JTextField inputBeamletList;
    private javax.swing.JComboBox inputDirectionTypes;
    private javax.swing.JCheckBox inputMaxDur;
    private javax.swing.JTextField inputSubbandList;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelAngle1;
    private javax.swing.JLabel labelAngle2;
    private javax.swing.JLabel labelBeamletList;
    private javax.swing.JLabel labelDirectionTypes;
    private javax.swing.JLabel labelSubbandList;
    private javax.swing.JButton saveButton;
    // End of variables declaration//GEN-END:variables
    
}
