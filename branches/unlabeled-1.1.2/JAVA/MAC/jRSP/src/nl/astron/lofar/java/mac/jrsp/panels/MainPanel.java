package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import nl.astron.lofar.mac.apl.gui.jrsp.Board;
import nl.astron.lofar.mac.apl.gui.jrsp.RCUMask;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.panels.IPluginPanel;

/**
 *
 * @author  balken
 */
public class MainPanel extends JPanel 
        implements IPluginPanel, ListSelectionListener, ChangeListener, Runnable
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
        
        // tabbedpane
        jTabbedPane.addChangeListener(this);
        
        // controlpanel
        controlPanel.setMainPanel(this);
        
        
        // waveformsettingspanel
        waveformSettingsPanel.setMainPanel(this);
        
        
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
     * Returns the index of the current selected board in the ListPanel.
     * @return  index
     */
    public int getSelectedBoardIndex()
    {
        return listPanel.getSelectedIndex();
    }
    
    /**
     * Returns the board.
     * @return  board
     */
    public Board getBoard()
    {
        return board;
    }
    
    /**
     * Sets the refresh rate of the currently selected panel.
     * @param   refreshRat      The new refreshRate for the current panel.
     */
    public void setCurrentRefreshRate(int refreshRate)
    {
        refreshRates[jTabbedPane.getSelectedIndex()] = refreshRate;
    }
    
    /**
     * Gets called when a value changed on the listPanel.
     * Note: Has *nothing* to do with the above functions.
     */
    public void valueChanged(ListSelectionEvent event)
    {
        updateCurrentPanel();
    }
    
    /**
     * Invoked when another tab is selected.
     */
    public void stateChanged(ChangeEvent event)
    {
        // Refresh can be only possible if the board is set. If not, the 0 in the
        // refreshrate textfield looks silly.
        if(board == null)
        {
            return;
        }
        
        // First check if there the refreshThread is running. If so: KILL IT!
        // Forget the checking part...
        refreshThread = null;
                
        // If the refresh rate of the selected panel is higher than 0, start the
        // refreshThread else call updateCurrentPanel once.
        if(refreshRates[jTabbedPane.getSelectedIndex()] > 0)
        {
            startRefreshThread();
        }
        else
        {
            updateCurrentPanel();
        }
        controlPanel.setRefreshRate(refreshRates[jTabbedPane.getSelectedIndex()]);
    }
    
    /**
     * This method is called to update the current panel.
     * Note: Board has to be set, to be using this function! Quick check is
     * performed at the beginning of the method.
     */
    public void updateCurrentPanel()
    {
        if(board == null)
            return;
        
        Class selectedClass = jTabbedPane.getSelectedComponent().getClass();
        
        int index = listPanel.getSelectedIndex();
        if(index == -1)
        {
            index = 0;
            listPanel.setSelectedIndex(0);
        }
        
        // StatusPanel
        // @TODO - change which status is returned based on the selected board!
        if(selectedClass.equals(StatusPanel.class))
        {
            RCUMask mask = new RCUMask();
            mask.setBit(index);
            ((StatusPanel)jTabbedPane.getSelectedComponent()).initFields(board.getStatus(mask.getMask())[0]);
        }
        else if(selectedClass.equals(TijdPanel.class))
        {
            ((TijdPanel)jTabbedPane.getSelectedComponent()).updatePanel();
        }
        else if(selectedClass.equals(WaveformSettingsPanel.class))
        {
            //waveformSettingsPanel.setBoard(board);
        }
    }
    
    /**
     * Updates the list of listItems of the listPanel.
     * Note: Board has to be set, before you can use this function!
     */
    private void updateListPanel()
    {
        String[] listItems = new String[board.getNofBoards()];
        for(int i=0; i<listItems.length; i++)
        {
            listItems[i] = Integer.toString(i);
        }
        listPanel.newList(listItems);
        
        // Sets the selected index to 0 on default.
        listPanel.setSelectedIndex(0);
    }
    
    /**
     * This method is invoked when the hostname is entered in the controlpanel
     * and makes a new Board instance based on the hostname.
     * @param   hostname    The hostname which will be used in the connection.
     * @param   refreshRate The number of seconds between the updates of current panel.
     *
     * @TODO Implement a error message when the connection fails.
     */
    public void initBoard(String hostname, int refreshRate)
    {
        // Do nothing if the hostname and refreshrate haven't changed.
        if(board != null && hostname.equals(board.getHostname()) && refreshRate == refreshRates[jTabbedPane.getSelectedIndex()])
        {
            return;
        }
        
        // The refreshrate should be 0 or higher.
        // @TODO add error message
        if(refreshRate < 0)
        {
            JOptionPane.showMessageDialog(null, "Only positive values and 0 are valid input for the refresh rate", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }        
        
        // If the board is null; there hasn't been a board yet, construct a new
        // board. Else change the current board.
        if(!board.isConnected())
        {
            board.connect(hostname);
            // Change listPanel according to the new Board.
            updateListPanel();
        }
        else if(!hostname.equals(board.getHostname()))
        {
            board.disconnect();
            board.connect(hostname);
            // Change listPanel according to the altered Board.
            updateListPanel();
        }
                
        refreshRates[jTabbedPane.getSelectedIndex()] = refreshRate;
        if(refreshRate == 0)
        {
            updateCurrentPanel();
        }
        else
        {
            // start thread that updates the board.
            startRefreshThread();
        }
    }
    
    /**
     * Starts the refresh thread.
     */
    public void startRefreshThread()
    {
        if(refreshThread == null)
        {
            refreshThread = new Thread(this, "RefreshThread");
            refreshThread.start();
        }
    }
    
    /**
     * Stops the refresh thread.
     */
    public void stopRefreshThread()
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
        statusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.StatusPanel();
        tijdPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.TijdPanel();
        waveformSettingsPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.WaveformSettingsPanel();
        controlPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel();
        listPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ListPanel();

        jTabbedPane.addTab("Status", statusPanel);

        jTabbedPane.addTab("[TEST] Tijd", tijdPanel);

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
                        .add(jTabbedPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 577, Short.MAX_VALUE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, listPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 589, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(controlPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel controlPanel;
    private javax.swing.JTabbedPane jTabbedPane;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ListPanel listPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.StatusPanel statusPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.TijdPanel tijdPanel;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.WaveformSettingsPanel waveformSettingsPanel;
    // End of variables declaration//GEN-END:variables
    
}
