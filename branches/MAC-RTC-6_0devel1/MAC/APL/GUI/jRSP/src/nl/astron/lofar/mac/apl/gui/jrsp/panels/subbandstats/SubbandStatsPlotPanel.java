package nl.astron.lofar.mac.apl.gui.jrsp.panels.subbandstats;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import nl.astron.lofar.mac.apl.gui.jrsp.RCUMask;
import org.apache.log4j.Logger;

public class SubbandStatsPlotPanel extends JPanel implements ActionListener
{
    /** log4j logger. */
    private Logger log;
    
    /** SubbandStatsPanel. */
    private SubbandStatsPanel ssPanel;
  
    /** RCUMask representing the active antenna's. */
    private RCUMask mask;
        
    /**
     * Default Constructor.
     */
    public SubbandStatsPlotPanel()
    {
        log = Logger.getLogger(SubbandStatsPlotPanel.class);
        
        initComponents();
                       
        /*
         * ActionListening
         */
        cmbBoard.setActionCommand("0");
        cmbBoard.addActionListener(this);
        chkAntenna1.setActionCommand("1");
        chkAntenna1.addActionListener(this);
        chkAntenna2.setActionCommand("2");
        chkAntenna2.addActionListener(this);
        chkAntenna3.setActionCommand("3");
        chkAntenna3.addActionListener(this);
        chkAntenna4.setActionCommand("4");
        chkAntenna4.addActionListener(this);
        chkAntenna5.setActionCommand("5");
        chkAntenna5.addActionListener(this);
        chkAntenna6.setActionCommand("6");
        chkAntenna6.addActionListener(this);
        chkAntenna7.setActionCommand("7");
        chkAntenna7.addActionListener(this);
        chkAntenna8.setActionCommand("8");
        chkAntenna8.addActionListener(this);
        
        mask = new RCUMask();
        
        /*
         * Test
         * @TODO: Remove
         */
        init(null);
    }
    
    /**
     * Initializes this panel and giving it access to it's ssPanel.
     */
    public void init(SubbandStatsPanel parent)
    {
        this.ssPanel = parent; 
    }
    
    /**
     * Called when the panel should be updated.
     */
    public void update()
    {
        // board should be connected
        
        /*
         * Items to be displayed in cmbBoard
         */
        for (int i = 0; i < ssPanel.getMainPanel().getBoard().getNrRSPBoards(); i++)
        {
            cmbBoard.addItem(Integer.toString(i));
        }
        
        updatePlot();
    }
    
    /**
     * Updates the plot.
     */
    public void updatePlot()
    {
        /*
         * Board has to be connected!
         */
        if (!ssPanel.getMainPanel().getBoard().isConnected())
        {
            return;
        }
        
        /*
         * Remove the lines from the plot.
         */        
        plotPanel.removeAllLines();

        /*
         * Retrieve new data for the lines.
         */
        double[] ssData = ssPanel.getMainPanel().getBoard().getSubbandStats(mask.getMask());
        
        /*
         * Add a line to the plot for every 512 doubles in the array.
         */
        for (int i = 0; i < (ssData.length / 512); i++)
        {
            /*
             * Copy data for current line to 
             */
            double[] temp = new double[512];
            System.arraycopy(ssData, i*512, temp, 0, 512);
            plotPanel.addLine(temp, Integer.toString(i));
        }
    }
    
    /**
     * Resets all checkboxes, turning them off.
     */
    public void reset()
    {
        /*
         * Deselect all checkboxes.
         */
        chkAntenna1.setSelected(false);
        chkAntenna2.setSelected(false);
        chkAntenna3.setSelected(false);
        chkAntenna4.setSelected(false);
        chkAntenna5.setSelected(false);
        chkAntenna6.setSelected(false);
        chkAntenna7.setSelected(false);
        chkAntenna8.setSelected(false);
        
        /*
         * Clear RCUMask
         */
        mask.setMask(0);        
    }
    
    /**
     * Selected Index is the same as the Board number.
     * @return  Number of the selected board.
     */
    public int getSelectedBoard()
    {
        return cmbBoard.getSelectedIndex();
    }
    
    /**
     * Returns the RCUMask. The selected board and antennas can be determined
     * based on the set bits.
     * @return  RCUMask
     */
    public RCUMask getRCUMask()
    {
        return mask;
    }    
    
    public void actionPerformed(ActionEvent e)
    {        
        int action = Integer.parseInt(e.getActionCommand());
        
        /*
         * If another board was selected: reset everything. If a antenna was
         * (un)checked change the mask according to that antenna.
         */
        if (action == 0)
        {
            // combobox
            reset();            
        }
        else
        {
            // antenna
            mask.flipBit((getSelectedBoard() * 8) + action - 1); // action is the antennanr. -1 to get index.
        }
        
        updatePlot();
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        plotPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.PlotPanel();
        cmbBoard = new javax.swing.JComboBox();
        chkAntenna1 = new javax.swing.JCheckBox();
        chkAntenna2 = new javax.swing.JCheckBox();
        chkAntenna3 = new javax.swing.JCheckBox();
        chkAntenna4 = new javax.swing.JCheckBox();
        chkAntenna5 = new javax.swing.JCheckBox();
        chkAntenna6 = new javax.swing.JCheckBox();
        chkAntenna7 = new javax.swing.JCheckBox();
        chkAntenna8 = new javax.swing.JCheckBox();

        setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createTitledBorder("Subband Statistics Plot")));

        chkAntenna1.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna1.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna2.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna2.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna3.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna3.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna4.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna4.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna5.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna5.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna6.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna6.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna7.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna7.setMargin(new java.awt.Insets(0, 0, 0, 0));

        chkAntenna8.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        chkAntenna8.setMargin(new java.awt.Insets(0, 0, 0, 0));

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(12, 12, 12)
                        .add(plotPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(115, 115, 115)
                        .add(cmbBoard, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 48, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna1)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna2)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna3)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna4)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna5)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna6)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna7)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(chkAntenna8)))
                .addContainerGap(12, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(plotPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(chkAntenna1)
                    .add(chkAntenna2)
                    .add(chkAntenna3)
                    .add(chkAntenna4)
                    .add(chkAntenna5)
                    .add(chkAntenna6)
                    .add(chkAntenna7)
                    .add(chkAntenna8)
                    .add(cmbBoard, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(14, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox chkAntenna1;
    private javax.swing.JCheckBox chkAntenna2;
    private javax.swing.JCheckBox chkAntenna3;
    private javax.swing.JCheckBox chkAntenna4;
    private javax.swing.JCheckBox chkAntenna5;
    private javax.swing.JCheckBox chkAntenna6;
    private javax.swing.JCheckBox chkAntenna7;
    private javax.swing.JCheckBox chkAntenna8;
    private javax.swing.JComboBox cmbBoard;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.PlotPanel plotPanel;
    // End of variables declaration//GEN-END:variables
}