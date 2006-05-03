/*
 * WaveformSettingsSelectPanel.java
 *
 * Created on April 18, 2006, 1:55 PM
 */

package nl.astron.lofar.mac.apl.gui.jrsp.panels.waveformsettings;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import javax.swing.event.EventListenerList;

/**
 *
 * @author  balken
 */
public class WaveformSettingsSelectPanel extends JPanel implements ActionListener
{   
    /** The parent class. */
    private WaveformSettingsPanel parent;
    
    /** Double array that stores the status of the antennas (shown or not). **/
    private boolean[][] antennas;
    
    /**
     * 
     * Creates new form WaveformSettingsSelectPanel
     */
    public WaveformSettingsSelectPanel() 
    {        
        /*
         * First number is the board it belongs to.
         * Second number the number of the antenna.
         * Because the names of the antennas range from 1 to 8, it's clearer
         * to use 9 values (of which the first (0) is never used) the always
         * subtracting 1 one the "name".
         */
        antennas = new boolean[2][9];
        
        initComponents();
        
        /*
         * Make comments in english.
         *
         * tientallen is het bord 0 (links) of 1 (rechts)
         * eentallen is welke antenne (op 0 na, is combobox)
         */
        
        cmbBoard1.addActionListener(this);
        cmbBoard1.setActionCommand("0");
        chkBoard1Antenna1.addActionListener(this);
        chkBoard1Antenna1.setActionCommand("1");
        chkBoard1Antenna2.addActionListener(this);
        chkBoard1Antenna2.setActionCommand("2");
        chkBoard1Antenna3.addActionListener(this);
        chkBoard1Antenna3.setActionCommand("3");
        chkBoard1Antenna4.addActionListener(this);
        chkBoard1Antenna4.setActionCommand("4");
        chkBoard1Antenna5.addActionListener(this);
        chkBoard1Antenna5.setActionCommand("5");
        chkBoard1Antenna6.addActionListener(this);
        chkBoard1Antenna6.setActionCommand("6");
        chkBoard1Antenna7.addActionListener(this);
        chkBoard1Antenna7.setActionCommand("7");
        chkBoard1Antenna8.addActionListener(this);
        chkBoard1Antenna8.setActionCommand("8");
        
        cmbBoard2.addActionListener(this);
        cmbBoard2.setActionCommand("10");
        chkBoard2Antenna1.addActionListener(this);
        chkBoard2Antenna1.setActionCommand("11");
        chkBoard2Antenna2.addActionListener(this);
        chkBoard2Antenna2.setActionCommand("12");
        chkBoard2Antenna3.addActionListener(this);
        chkBoard2Antenna3.setActionCommand("13");
        chkBoard2Antenna4.addActionListener(this);
        chkBoard2Antenna4.setActionCommand("14");
        chkBoard2Antenna5.addActionListener(this);
        chkBoard2Antenna5.setActionCommand("15");
        chkBoard2Antenna6.addActionListener(this);
        chkBoard2Antenna6.setActionCommand("16");
        chkBoard2Antenna7.addActionListener(this);
        chkBoard2Antenna7.setActionCommand("17");
        chkBoard2Antenna8.addActionListener(this);
        chkBoard2Antenna8.setActionCommand("18");
    }
    

    public void init(WaveformSettingsPanel parent)
    {
        this.parent = parent;
    }
    
    public void update()
    {          
        if(parent.getMainPanel().getBoard().isConnected())
        {
            /*
             * The comboboxes
             */
            int nofBoards = parent.getMainPanel().getBoard().getNrRSPBoards();
            
            cmbBoard1.removeAllItems();
            cmbBoard2.removeAllItems();
            
            for (int i = 0; i < nofBoards; i++)
            {
                cmbBoard1.addItem(Integer.toString(i));
                cmbBoard2.addItem(Integer.toString(i));
            }
            
            /*
             * The plotted lines.
             */
            for (int i = 0; i < antennas.length; i++)
            {
                for (int j = 0; j < antennas[i].length; j++)
                {
                    if (antennas[i][j])
                    {
                        // @TODO: edit!
                        parent.getPlotPanel().removeLine(Integer.toString((i * 10) + j));
                        parent.getPlotPanel().addLine(parent.getMainPanel().getBoard().getSubbandStats(1), Integer.toString((i * 10) + j));
                    }                    
                }
            } 
        }
    }
       
    /**
     * Invoked when an action occurs.
     * @param   e                       ActionEvent
     */
    public void actionPerformed(ActionEvent e)
    {
        if (!parent.getMainPanel().getBoard().isConnected())
        {
            return;
        }
        
        int actionSource = Integer.parseInt(e.getActionCommand());

        int board = actionSource / 10;

        if (actionSource % 10 == 0)
        {
            // actionSource is a comboboxg

        }
        else
        {
            // actionSource is a antenna
            int antenna = actionSource % 10;
            
            if (!antennas[board][antenna])
            {
                // not displayed in plot
                parent.getPlotPanel().addLine(parent.getMainPanel().getBoard().getSubbandStats(1), Integer.toString(actionSource));    
                antennas[board][antenna] = true;
            }
            else
            {
                // already displayed -> remove
                parent.getPlotPanel().removeLine(Integer.toString(actionSource));
                antennas[board][antenna] = false;
            }
            
            
        }           
    }
        
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        cmbBoard1 = new javax.swing.JComboBox();
        chkBoard1Antenna1 = new javax.swing.JCheckBox();
        chkBoard1Antenna2 = new javax.swing.JCheckBox();
        chkBoard1Antenna3 = new javax.swing.JCheckBox();
        chkBoard1Antenna4 = new javax.swing.JCheckBox();
        chkBoard1Antenna5 = new javax.swing.JCheckBox();
        chkBoard1Antenna6 = new javax.swing.JCheckBox();
        chkBoard1Antenna7 = new javax.swing.JCheckBox();
        chkBoard1Antenna8 = new javax.swing.JCheckBox();
        cmbBoard2 = new javax.swing.JComboBox();
        chkBoard2Antenna1 = new javax.swing.JCheckBox();
        chkBoard2Antenna2 = new javax.swing.JCheckBox();
        chkBoard2Antenna3 = new javax.swing.JCheckBox();
        chkBoard2Antenna4 = new javax.swing.JCheckBox();
        chkBoard2Antenna5 = new javax.swing.JCheckBox();
        chkBoard2Antenna6 = new javax.swing.JCheckBox();
        chkBoard2Antenna7 = new javax.swing.JCheckBox();
        chkBoard2Antenna8 = new javax.swing.JCheckBox();

        setBorder(javax.swing.BorderFactory.createTitledBorder("Selection"));

        chkBoard1Antenna1.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna1.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna2.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna2.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna3.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna3.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna4.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna4.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna5.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna5.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna6.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna6.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna7.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna7.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard1Antenna8.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard1Antenna8.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna1.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna1.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna2.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna2.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna3.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna3.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna4.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna4.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna5.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna5.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna6.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna6.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna7.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna7.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkBoard2Antenna8.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkBoard2Antenna8.setMargin(new java.awt.Insets(0, 0, 0, 0));

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(layout.createSequentialGroup()
                        .add(chkBoard1Antenna1)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna2)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna3)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna4)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna5)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna6)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna7)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard1Antenna8))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, cmbBoard1, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 71, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                    .add(layout.createSequentialGroup()
                        .add(chkBoard2Antenna1)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna2)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna3)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna4)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna5)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna6)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna7)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkBoard2Antenna8))
                    .add(cmbBoard2, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(cmbBoard1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(cmbBoard2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(chkBoard1Antenna1)
                    .add(chkBoard1Antenna2)
                    .add(chkBoard1Antenna3)
                    .add(chkBoard1Antenna4)
                    .add(chkBoard1Antenna5)
                    .add(chkBoard1Antenna6)
                    .add(chkBoard1Antenna7)
                    .add(chkBoard1Antenna8)
                    .add(chkBoard2Antenna8)
                    .add(chkBoard2Antenna7)
                    .add(chkBoard2Antenna6)
                    .add(chkBoard2Antenna5)
                    .add(chkBoard2Antenna4)
                    .add(chkBoard2Antenna3)
                    .add(chkBoard2Antenna2)
                    .add(chkBoard2Antenna1))
                .addContainerGap(69, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox chkBoard1Antenna1;
    private javax.swing.JCheckBox chkBoard1Antenna2;
    private javax.swing.JCheckBox chkBoard1Antenna3;
    private javax.swing.JCheckBox chkBoard1Antenna4;
    private javax.swing.JCheckBox chkBoard1Antenna5;
    private javax.swing.JCheckBox chkBoard1Antenna6;
    private javax.swing.JCheckBox chkBoard1Antenna7;
    private javax.swing.JCheckBox chkBoard1Antenna8;
    private javax.swing.JCheckBox chkBoard2Antenna1;
    private javax.swing.JCheckBox chkBoard2Antenna2;
    private javax.swing.JCheckBox chkBoard2Antenna3;
    private javax.swing.JCheckBox chkBoard2Antenna4;
    private javax.swing.JCheckBox chkBoard2Antenna5;
    private javax.swing.JCheckBox chkBoard2Antenna6;
    private javax.swing.JCheckBox chkBoard2Antenna7;
    private javax.swing.JCheckBox chkBoard2Antenna8;
    private javax.swing.JComboBox cmbBoard1;
    private javax.swing.JComboBox cmbBoard2;
    private javax.swing.JCheckBox jCheckBox18;
    private javax.swing.JCheckBox jCheckBox3;
    private javax.swing.JCheckBox jCheckBox4;
    private javax.swing.JComboBox jComboBox2;
    // End of variables declaration//GEN-END:variables
    
}
