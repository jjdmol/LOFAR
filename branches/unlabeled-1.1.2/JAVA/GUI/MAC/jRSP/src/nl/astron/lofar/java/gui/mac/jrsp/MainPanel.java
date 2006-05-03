package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import nl.astron.lofar.mac.apl.gui.jrsp.Board;
import nl.astron.lofar.mac.apl.gui.jrsp.panels.status.StatusPanel;
import nl.astron.lofar.mac.apl.gui.jrsp.panels.waveformsettings.WaveformSettingsPanel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.panels.IPluginPanel;

/**
 * The MainPanel is the GUI entrypoint for the jRSP project. It has a ListPanel
 * that lists all the available boards, a ControlPanel for editing the hostname
 * and refreshrate, and a tabbedpane containing the detailed panels for
 * controlling the boards.
 * 
 * To support refresh a thread is needed and therefore MainPanel implements
 * Runnable. The functions startRefresh and stopRefresh are used
 * to start and stop the thread. When the thread is started it will, depending
 * on the refreshrate, refresh the current panel using the method
 * updateCurrentPabel.
 * 
 * The MainPanel implements a ListSelectionListener to react to changes in the
 * ListPanel and a ChangeListener to react to changes in the tabbedpane.
 * 
 * 
 * @author balken
 */
public class MainPanel extends JPanel implements IPluginPanel, 
        ListSelectionListener, ActionListener, ChangeListener, Runnable
{
    /** MainFrame */
    private MainFrame mainFrame;
    
    /** Board */
    private Board board;
    
    /** Refresh Thread - Thread used to refresh the displayed panel. */
    private Thread refreshThread;
    
    /** RefreshRates - used to store the refreshRates of the different panels. */
    private int[] refreshRates;
    
    /** Name */
    private static final String name = "jRSP";
    
    /** Changed */
    private boolean changed;
    
    /** 
     * Creates new form MainPanel.
     */
    public MainPanel() 
    {
        mainFrame = null;
        board = new Board();
        refreshThread = null;
        changed = false;
        
        initComponents();        
        
        // listpanel
        listPanel.setTitle("Boards");
        listPanel.addListSelectionListener(this);
        
        // controlpanel
        controlPanel.addActionListener(this);
        
        // tabbedpane
        jTabbedPane.addChangeListener(this);
        
        // tabpanels        
        statusPanel.init(this);
        subbandStatsPanel.init(this);
        waveformSettingsPanel.init(this);
        
        refreshRates = new int[jTabbedPane.getTabCount()];
    }
    
    /**
     * Initializes the plugin and receives a refrence to the mainFrame.
     * @param   mainFrame   Refrence to the MainFrame.
     * @return  true        Letting the MainFrame know there are no problems.
     */
    public boolean initializePlugin(MainFrame mainFrame)
    {
        this.mainFrame = mainFrame;
        
        return true;
    }
    
    /**
     * Returns the friendly name of this panel.
     * @return  name    The friendly name.
     */
    public String getFriendlyName()
    {
        return name;
    }
    
    /**
     * Same as the above function except this function is static.
     * @return  name    The name in a static friendly way.
     */
    public static String getFriendlyNameStatic()
    {
        return name;
    }
    
    /**
     * Returns if this panel has changed,
     * @return  changed
     */
    public boolean hasChanged()
    {
        return changed;
    }
    
    /**
     * Set the changed value.
     * @param   changed     The new value for changed.
     */
    public void setChanged(boolean changed)
    {
        this.changed = changed;
    }
    
    /**
     * It is has to be implemented...
     */
    public void checkChanged() { }
      
    /**
     * Returns the board.
     * @return  board
     */
    public Board getBoard()
    {
        return board;
    }
    
    /**
     * Returns the index of the current selected board in the ListPanel.
     * @return  index
     */
    public int getSelectedBoardIndex()
    {
        return listPanel.getSelectedIndex();
    }
    
    /**
     * Sets the refresh rate of the currently selected panel.
     * @param   refreshRat      The new refreshRate for the current panel.
     */
    public void setCurrentRefreshRate(int refreshRate)
    {
        refreshRates[jTabbedPane.getSelectedIndex()] = refreshRate;
    }
    
    
    /** LISTENER IMPLEMENTATIONS **/
    
    /**
     * Invoked when another board is selected on the listPanel.
     */
    public void valueChanged(ListSelectionEvent e)
    {
        updateCurrentPanel();
    }
    
    /**
     * Invoked when an action is performed by the controlPanel.
     */
    public void actionPerformed(ActionEvent e)
    {
        switch(controlPanel.getSourceAction(e.getSource()))
        {
            case ControlPanel.UPDATE:
                /*
                 * Connect/update the board and the current panel.
                 */
                updateBoard();
                controlPanel.setRefreshRate(0);
                /*
                 * By calling startRefresh we make sure the buttons don't get 
                 * "locked" because we are waiting for the updateCurrentPanel()
                 * call that takes time.
                 */
                startRefresh(); //updateCurrentPanel();                
                break;
            case ControlPanel.REFRESH:
                /*
                 * Connect to board and update according to the refreshrate.
                 * Check if refresh rate is valid. If so set the refreshrate in 
                 * refreshRates[] and call startRefresh().
                 */
                if(controlPanel.getRefreshRate() < 0)
                {
                    /*
                     * The RefreshRate isn't a valid number (either error (-1) 
                     * or negative number). Display error and quit.
                     */
                    JOptionPane.showMessageDialog(this, "Refreshrate may only contain positive numbers and zero.", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }                
                updateBoard();
                refreshRates[jTabbedPane.getSelectedIndex()] = controlPanel.getRefreshRate();
                startRefresh();
                break;
            case ControlPanel.STOP:
                /*
                 * Stop the refresh.
                 */
                stopRefresh();
                break;
            default:
                /*
                 * Do nothing.
                 */
                break;                
        }
    }
        
    /**
     * Invoked when another tab is selected.
     */
    public void stateChanged(ChangeEvent e)
    {
        /*
         * Refresh can be only possible if the board is connected. If not, the 0 
         * in the refreshrate textfield looks silly.
         */
        if (!board.isConnected())
        {
            return;
        }
        
        /*
         * First check if there the refreshThread is running. If so: KILL IT!
         */
        refreshThread = null;
                
        /*
         * If the refresh rate of the selected panel is higher than 0, start the
         * refreshThread. If refreshrate is 0, then the current panel isn't
         * updated!
         */
        if (refreshRates[jTabbedPane.getSelectedIndex()] > 0)
        {
            startRefresh();
        }
        
        
        /*
         * Update controlPanel.
         */         
        controlPanel.setRefreshRate(refreshRates[jTabbedPane.getSelectedIndex()]);
    }
        
    /** END OF LISTENER IMPLEMENTATIONS **/
    
    /**
     * Updates or initializes the board by changing the hostname.
     */
    public void updateBoard()
    {
        /*
         * If there is no hostname entered display a error.
         */
        if ("".equals(controlPanel.getHostname().trim()))
        {            
            JOptionPane.showMessageDialog(this, "The hostname can't be empty.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        /*
         * If the board is already connected with the same hostname exit method.
         */
        if (board.isConnected() && controlPanel.getHostname().equals(board.getHostname()))
        {            
            return;
        }
        
        board.connect(controlPanel.getHostname());
        
        /*
         * Construct a String array for the listpanel.
         */
        int nofBoards = board.getNrRSPBoards();
        String[] listItems = new String[nofBoards];
        for (int i = 0; i < nofBoards; i++)
        {
            listItems[i] = Integer.toString(i);
        }
        
        listPanel.newList(listItems);
    }
    
    
    /**
     * This method is called to update the current panel.
     * Note: Board has to be set, to be using this function! Quick check is
     * performed at the beginning of the method.
     */
    public void updateCurrentPanel()
    {
        /*
         * The board has to be connected to update the panels.
         */
        if (!board.isConnected())
        {
            return;
        }
        
        /** The index of the selected board in the list panel. */
        int index = listPanel.getSelectedIndex();
        
        if(index == -1)
        {
            /* 
             * When the index is -1. The selected index will be changed. That will
             * generate a valueChangedEvent. Through the method valueChanged, that
             * responses to this event, this (updateCurrentPanel() function is 
             * called again. 
             */
            index = 0;
            listPanel.setSelectedIndex(0);
            return;
        }
        
        /*
         * Update the selected panel.
         */
        ((ITabPanel) jTabbedPane.getSelectedComponent()).update();
    }
    
    /**
     * Updates the list of listItems of the listPanel.
     * Note: Board has to be set, before you can use this function!
     */
    private void updateListPanel()
    {
        String[] listItems = new String[board.getNrRSPBoards()];
        for(int i=0; i<listItems.length; i++)
        {
            listItems[i] = Integer.toString(i);
        }
        listPanel.newList(listItems);
        
        // Sets the selected index to 0 on default.
        listPanel.setSelectedIndex(0);
    }
    
        
    /**
     * Starts the refresh thread.
     */
    private void startRefresh()
    {
        if (refreshThread == null)
        {
            refreshThread = new Thread(this, "RefreshThread");
            refreshThread.start();    
        }
    }
    
    /**
     * Stops the refresh thread.
     */
    private void stopRefresh()
    {
        refreshThread = null;
    }
    
    /**
     * Run method.
     */
    public void run()
    {
        Thread thread = Thread.currentThread();
        while(thread == refreshThread)
        {            
            try
            {
                Thread.sleep(refreshRates[jTabbedPane.getSelectedIndex()] * 1000);
            }
            catch(InterruptedException e)
            {
                // Just ignore it!
            }
            updateCurrentPanel();
            
            /*
             * If the refreshRate is smaller than 1, call stopRefresh
             */
            if (refreshRates[jTabbedPane.getSelectedIndex()] < 1)
            {
                stopRefresh();
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
        jTabbedPane = new javax.swing.JTabbedPane();
        statusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.status.StatusPanel();
        subbandStatsPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.subbandstats.SubbandStatsPanel();
        waveformSettingsPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.waveformsettings.WaveformSettingsPanel();
        controlPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel();
        listPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ListPanel();

        jTabbedPane.addTab("Status", statusPanel);

        jTabbedPane.addTab("Subband Statistics", subbandStatsPanel);

        jTabbedPane.addTab("Waveform Settings", waveformSettingsPanel);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(listPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 139, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jTabbedPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1125, Short.MAX_VALUE))
            .add(controlPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1270, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(12, 12, 12)
                        .add(jTabbedPane, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 577, Short.MAX_VALUE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, listPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 589, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(controlPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel controlPanel;
    private javax.swing.JTabbedPane jTabbedPane;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ListPanel listPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.status.StatusPanel statusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.subbandstats.SubbandStatsPanel subbandStatsPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.waveformsettings.WaveformSettingsPanel waveformSettingsPanel;
    // End of variables declaration//GEN-END:variables
    
}
