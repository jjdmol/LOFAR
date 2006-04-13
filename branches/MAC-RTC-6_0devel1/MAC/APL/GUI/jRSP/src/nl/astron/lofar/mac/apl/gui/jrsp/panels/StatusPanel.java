package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import javax.swing.JPanel;
import nl.astron.lofar.mac.apl.gui.jrsp.BoardStatus;

/**
 * The StatusPanel displays the Status of the RSPboard (BoardStatus) that is 
 * passed to this class through the initFields() method. It delegates all the 
 * "displaying" to other panels.
 * @see ADOStatusPanel
 * @see DIAGStatusPanel
 * @see ETHStatusPanel
 * @see MEPStatusPanel
 * @see RCUStatusPanel
 * @see RSPStatusPanel
 * @see RSUStatusPanel
 * @see SyncStatusPanel
 *
 * @author  balken
 */
public class StatusPanel extends JPanel
{
    /** Creates new form StatusPanel */
    public StatusPanel() 
    {
        initComponents();
        blp0SyncStatusPanel.setTitle("BLP0 Sync");
        blp1SyncStatusPanel.setTitle("BLP1 Sync");
        blp2SyncStatusPanel.setTitle("BLP2 Sync");
        blp3SyncStatusPanel.setTitle("BLP3 Sync");
        blp0RcuStatusPanel.setTitle("BLP0 RCU");
        blp1RcuStatusPanel.setTitle("BLP1 RCU");
        blp2RcuStatusPanel.setTitle("BLP2 RCU");
        blp3RcuStatusPanel.setTitle("BLP3 RCU");
        blp0AdoStatusPanel.setTitle("BLP0 ADO");
        blp1AdoStatusPanel.setTitle("BLP1 ADO");
        blp2AdoStatusPanel.setTitle("BLP2 ADO");
        blp3AdoStatusPanel.setTitle("BLP3 ADO");
    }
    
    public void initFields(BoardStatus boardStatus)
    {        
        rspStatusPanel.initFields(boardStatus);
        ethStatusPanel.initFields(boardStatus);
        mepStatusPanel.initFields(boardStatus);
        diagStatusPanel.initFields(boardStatus);
        blp0SyncStatusPanel.initFields(boardStatus.blp0Sync);
        blp1SyncStatusPanel.initFields(boardStatus.blp1Sync);
        blp2SyncStatusPanel.initFields(boardStatus.blp2Sync);
        blp3SyncStatusPanel.initFields(boardStatus.blp3Sync);
        blp0RcuStatusPanel.initFields(boardStatus.blp0Rcu);
        blp1RcuStatusPanel.initFields(boardStatus.blp1Rcu);
        blp2RcuStatusPanel.initFields(boardStatus.blp2Rcu);
        blp3RcuStatusPanel.initFields(boardStatus.blp3Rcu);
        rsuStatusPanel.initFields(boardStatus);
        blp0AdoStatusPanel.initFields(boardStatus.blp0AdcOffset);
        blp1AdoStatusPanel.initFields(boardStatus.blp1AdcOffset);
        blp2AdoStatusPanel.initFields(boardStatus.blp2AdcOffset);
        blp3AdoStatusPanel.initFields(boardStatus.blp3AdcOffset);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        rspStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RSPStatusPanel();
        ethStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ETHStatusPanel();
        mepStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.MEPStatusPanel();
        diagStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.DIAGStatusPanel();
        blp0SyncStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel();
        blp1SyncStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel();
        blp2SyncStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel();
        blp3SyncStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel();
        blp0RcuStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel();
        blp1RcuStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel();
        blp2RcuStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel();
        blp3RcuStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel();
        rsuStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.RSUStatusPanel();
        blp0AdoStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel();
        blp1AdoStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel();
        blp2AdoStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel();
        blp3AdoStatusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel();

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                        .add(ethStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(mepStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(rspStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(diagStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(blp0SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(6, 6, 6)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createSequentialGroup()
                                .add(blp1RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp1AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(layout.createSequentialGroup()
                                .add(blp0RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp0AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(blp1SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp2SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp3SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createSequentialGroup()
                                .add(blp2RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp2AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(layout.createSequentialGroup()
                                .add(blp3RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp3AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(rsuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(rspStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(ethStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(mepStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(diagStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(layout.createSequentialGroup()
                        .add(blp0SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp1SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp2SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp3SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(blp0RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp0AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(layout.createSequentialGroup()
                                .add(blp1AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .add(blp2AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(layout.createSequentialGroup()
                                .add(blp1RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp2RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(blp3RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp3AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(rsuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(97, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel blp0AdoStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel blp0RcuStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel blp0SyncStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel blp1AdoStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel blp1RcuStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel blp1SyncStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel blp2AdoStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel blp2RcuStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel blp2SyncStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ADOStatusPanel blp3AdoStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RCUStatusPanel blp3RcuStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.SyncStatusPanel blp3SyncStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.DIAGStatusPanel diagStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ETHStatusPanel ethStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.MEPStatusPanel mepStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RSPStatusPanel rspStatusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.RSUStatusPanel rsuStatusPanel;
    // End of variables declaration//GEN-END:variables
    
}
