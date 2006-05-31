/*
 *  PlotSlotsPanel.java
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
 *
 */

package nl.astron.lofar.sas.otb.util.plotter;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;
import org.apache.log4j.Logger;

/**
 * @version $Id$
 * @created May 29, 2006, 14:12
 * @author pompert
 */
public class PlotSlotsPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(PlotSlotsPanel.class);
    static String name = "PlotSlotsPanel";
    private PlotSlotManager itsSlotManager;
    private PlotSlot selectedSlot;
      
    /** Creates new form BeanForm */
    public PlotSlotsPanel() {
        initComponents();
        int numberOfSlots = 4;
        itsSlotManager = new PlotSlotManager(numberOfSlots);
        itsSlotManager.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent evt){
                if(evt.getActionCommand().equals("REFRESH")){
                    rearrangeSlotGrid();
                }
            }            
        });
        rearrangeSlotGrid();
    }

    /** adds a button to the BeanForm */
    public void addPlotToSlot(int index,Object constraints) throws IllegalArgumentException{
        if(itsSlotManager.isSlotAvailable(index)){
            itsSlotManager.createPlotInSlot(index,constraints);
        }else{
            throw new IllegalArgumentException("A plot already exists in slot "+index);
        }
    }
    
    
    public int getAmountOfSlots(){
        return itsSlotManager.getAmountOfSlots();
    }
    
    public void setAmountOfSlots(int amount, boolean force) throws IllegalArgumentException{
        itsSlotManager.setAmountOfSlots(amount,force);
    }
       
    public boolean isSlotAvailable(int index){
        return itsSlotManager.isSlotAvailable(index);
    }
    
    public int[] getAvailableSlotIndexes(){
        return itsSlotManager.getAvailableSlotIndexes();
    }
    
    private void rearrangeSlotGrid(){
        slotsPanel.removeAll();
        double squareRoot = Math.sqrt(Double.parseDouble(""+itsSlotManager.getAmountOfSlots()));
        int columnsAndRows = Integer.parseInt(""+(int)squareRoot);
        GridBagConstraints gridBagConstraints = new java.awt.GridBagConstraints();
            gridBagConstraints.gridwidth = columnsAndRows;
            gridBagConstraints.gridheight = columnsAndRows;
            gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
            gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 0);
        GridBagLayout layout = new GridBagLayout();
        layout.setConstraints(slotsPanel,gridBagConstraints);
        //320,240 both!
        slotsPanel.setSize(this.getSize());
        slotsPanel.setMinimumSize(this.getSize());
        //setSize(slotsPanel.getSize());
        logger.trace("getting "+itsSlotManager.getAmountOfSlots()+" slots from PlotSlotManager");
        int x = 0;
        int y = 0;
        for(int i = 1; i <= itsSlotManager.getAmountOfSlots(); i++){
            logger.trace("Setting Slot to grid coordinate ("+x+","+y+")");
            gridBagConstraints = new java.awt.GridBagConstraints();
            gridBagConstraints.gridx = x;
            gridBagConstraints.gridy = y;
            gridBagConstraints.ipadx = 0;
            gridBagConstraints.ipady = 0;
            gridBagConstraints.weightx = 1.0;
            gridBagConstraints.weighty = 1.0;
            gridBagConstraints.anchor = java.awt.GridBagConstraints.CENTER;
            PlotSlot newSlot = itsSlotManager.getSlot(i);
            newSlot.addMouseListener(new SlotMouseAdapter());
            newSlot.setSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
            //
            newSlot.setMinimumSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
            newSlot.setPreferredSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
            slotsPanel.add(newSlot,gridBagConstraints);            
            x++;
            if (x == columnsAndRows){
                y++;
                x = 0;
            }
        }
        slotsPanel.validate();
        //slotsPanel.repaint();
     }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        slotsPanel = new javax.swing.JPanel();

        setLayout(new java.awt.BorderLayout());

        slotsPanel.setLayout(new java.awt.GridBagLayout());

        slotsPanel.setBackground(new java.awt.Color(255, 255, 255));
        slotsPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        add(slotsPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel slotsPanel;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;

    /**
     * Registers ActionListener to receive events.
     *
     * @param listener The listener to register.
     */
    public void addActionListener(java.awt.event.ActionListener listener) {

        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     *
     * @param listener The listener to remove.
     */
    public void removeActionListener(java.awt.event.ActionListener listener) {

        listenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }
   
    /**
     * This inner class provides the plotter with the functionality of moving slots etc
     *
     */
    class SlotMouseAdapter extends MouseAdapter{
        public void mouseReleased(MouseEvent e){
            Object source = e.getSource();
            if(source instanceof PlotSlot){
                selectedSlot = (PlotSlot)source;
                if(SwingUtilities.isRightMouseButton(e) && selectedSlot.containsPlot()){
                    
                    JPopupMenu aPopupMenu = new JPopupMenu();
                    int[] availableSlots = itsSlotManager.getAvailableSlotIndexes();
                    if(availableSlots.length > 0){
                        JMenuItem aMenuItem=new JMenuItem("Move to ...");
                      
                        aMenuItem.addMouseListener(new MouseAdapter(){
                            public void mouseEntered(MouseEvent e){}
                            public void mouseExited(MouseEvent e){}
                            public void mouseReleased(MouseEvent e){
                                System.out.println("hiero entered");
                                JPopupMenu moveSlotPopupMenu = new JPopupMenu();
                                int[] availableSlots = itsSlotManager.getAvailableSlotIndexes();
                                for(int i = 0; i < availableSlots.length; i++){
                                    JMenuItem subItem=new JMenuItem("Slot "+availableSlots[i]);
                                    subItem.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            itsSlotManager.movePlot(Integer.parseInt(selectedSlot.getLabel()),Integer.parseInt(evt.getActionCommand().substring(5)));
                                        }
                                    });
                                    moveSlotPopupMenu.add(subItem);
                                }
                                moveSlotPopupMenu.setOpaque(true);
                                moveSlotPopupMenu.show(selectedSlot, e.getX(), e.getY());
                            }
                        });
                        
                        aPopupMenu.add(aMenuItem);
                        
                        aPopupMenu.setOpaque(true);
                        aPopupMenu.show(selectedSlot, e.getX(), e.getY());
                    }
                }
            }
        }
    }
    
}
